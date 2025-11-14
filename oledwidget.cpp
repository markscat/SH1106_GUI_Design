/* ******************Copyright (C) 2025 Ethan Yang *****************************
 * @file    oledwidget.cpp
 * @author  你的名字
 * @date    2024/10/25
 * @brief   OLED 螢幕模擬 widget 的核心功能實作。
 *
 * @details 這個檔案包含了 OLEDWidget 類別的所有成員函式實作，
 *          負責處理底層像素操作、繪圖演算法的執行，
 *          以及對使用者滑鼠輸入的反應。
 *
 * @note    本專案使用 GPLv3 授權，詳情請見 LICENSE 檔案。
 *
 * *****************Copyright (C) 2024*****************************************
 */

#include "oledwidget.h"
#include "oleddatamodel.h"

OLEDWidget::OLEDWidget(QWidget *parent)
    : QWidget(parent),
    m_image(OledConfig::DISPLAY_WIDTH, OledConfig::DISPLAY_HEIGHT, QImage::Format_RGB888), // 直接在初始化列表創建 QImage
    m_currentTool(Tool_Pen), // 預設為畫筆
    m_isDrawing(false),
    m_brushSize(1)
{
    // 初始為空白 128x64

    m_image.fill(Qt::black); // 填充背景色

    setScale(7); // 呼叫 setScale 來設定尺寸和縮放

    setFocusPolicy(Qt::StrongFocus); // 允許接收鍵盤事件
}

void OLEDWidget::setScale(int s) {
    const int minScale = 1;
    const int maxScale = 20; // 依需求調整最大放大倍數
    scale = std::clamp(s, minScale, maxScale);
    //scale = s > 0 ? s : 1;
    //setFixedSize(img.width() * scale, img.height() * scale);
    setFixedSize(m_image.width() * scale, m_image.height() * scale);

    update();
}

// ↓↓↓↓ 檢查並補上 clearScreen 函式 ↓↓↓↓
void OLEDWidget::clearScreen() {
    // 將內部緩衝區全部填 0
    //memset(m_buffer, 0, sizeof(m_buffer));
    //updateImageFromBuffer(); // 更新顯示

    // 1. 调用数据模型来清除数据
    m_model.clear();

    // 2. 调用辅助函数，从更新后的模型同步到显示图像
    updateImageFromModel();
}

void OLEDWidget::setBrushSize(int size)
{
    // 安全限制：筆刷大小只能是 1～6
    //if (size < 1) size = 1;
    //if (size > 6) size = 6;

    //m_brushSize = size;

    m_brushSize = std::clamp(size, 1, 6); // 限制笔刷大小在 1-6 之间

}


void OLEDWidget::setBuffer(const uint8_t *buffer){
    // 同步内部状态

    // 1. 调用数据模型的新方法，从硬体 buffer 载入数据并完成翻译
    m_model.setFromHardwareBuffer(buffer);

    // 2. 数据模型更新后，同步到显示图像
    updateImageFromModel();
}

/**
 * @brief 取得符合硬體格式的顯示緩衝區。
 *
 * 這是一個方便的包裝函式（wrapper function），作為 `OLEDWidget`（View）對外的介面，
 * 用於取得底層資料模型（Model）所產生的硬體緩衝區。
 * 實際的格式轉換邏輯由 `m_model` 物件處理，此處僅做轉發呼叫。
 * 這樣設計符合 Model-View 的職責分離原則。
 *
 * @return std::vector<uint8_t> 一個包含可以直接寫入 OLED 硬體的原始資料緩衝區。
 * @see OledDataModel::getHardwareBuffer() const
 */
std::vector<uint8_t> OLEDWidget::getHardwareBuffer() const
{
    // 直接返回从数据模型翻译过来的硬体 buffer
    return m_model.getHardwareBuffer();
}

// ================== 新增的 SLOT ==================
/**
 * @brief [SLOT] 設定目前使用的繪圖或互動工具。
 *
 * 這是一個 Qt 的槽（Slot），設計用來接收來自工具列按鈕或其他 UI 元件的訊號。
 * 當使用者點選不同的工具（如畫筆、橡皮擦、選取工具）時，這個槽會被呼叫，
 * 並更新內部狀態 `m_currentTool`。
 *
 * 這個狀態變數會直接影響滑鼠事件（如 `mousePressEvent`, `mouseMoveEvent`）的行為，
 * 從而決定使用者在畫布上的操作是進行繪製、擦除還是選取。
 *
 * @param[in] tool 要被啟用的新工具類型（來自 `ToolType` enum）。
 */
void OLEDWidget::setCurrentTool(ToolType tool) {
    m_currentTool = tool;
/*
    if (tool != Tool_Select) {
        m_selectedRegion = QRect(); // 清除選取框
        m_isSelecting = false;
        m_isDraggingSelection = false;
        update(); // 觸發重繪
    }*/

}


/**
 * @brief [SLOT] 將像素資料轉換為 C/C++ 標頭檔陣列格式並顯示。
 *
 * 這是一個工具性槽函數，主要提供給開發者使用，以便將畫面上的圖形快速轉換為
 * 可用於嵌入式系統（如 Arduino, ESP32 等）的原始位元組陣列。
 *
 * 此函數會根據當前是否存在有效選取區 (`m_selectedRegion`) 來決定匯出的範圍：
 * - 如果有選取區，則只轉換選取區內的像素。
 * - 如果沒有選取區，則轉換整個畫面的像素。
 *
 * 轉換完成後，結果會被格式化成一個 C 語言的 `const unsigned char` 陣列，
 * 並在一個彈出視窗中顯示，方便使用者複製貼上。
 *
 * @see OledDataModel::copyRegionToLogicalFormat()
 * @see OledDataModel::convertLogicalToHardwareFormat()
 */
void OLEDWidget::showBufferDataAsHeader()
{
    // 步骤 1: 确定要导出的区域 (有选区就用选区，否则用整个屏幕)
    QRect region = m_selectedRegion.isValid() ? m_selectedRegion :
                       QRect(0, 0, OledConfig::DISPLAY_WIDTH, OledConfig::DISPLAY_HEIGHT);

    // 步骤 2: [复用!] 调用 model 将该区域转换为逻辑图像 QImage
    QImage logicalData = m_model.copyRegionToLogicalFormat(region);

    if (logicalData.isNull()) {
        return; // 如果区域无效或复制失败，则不继续
    }

    // 步骤 3: [复用!] 调用 model 将逻辑图像转换为硬件格式的 QVector
    QVector<uint8_t> hardwareData = OledDataModel::convertLogicalToHardwareFormat(logicalData);

    // ------------------------------------------------------------------
    // 到此为止，数据准备工作已完成，且没有任何重复的转换算法。
    // 下面的代码只负责将准备好的 hardwareData 格式化为字符串。
    // ------------------------------------------------------------------


    // 步骤 4: 将打包好的 hardwareData 格式化成 C 阵列字符串
    QString output;
    output += QString("// Image Data (%1x%2 region at (%3, %4))\n")
                  .arg(logicalData.width()).arg(logicalData.height())
                  .arg(region.left()).arg(region.top());
    output += QString("const uint8_t imageData[%1] = {\n    ").arg(hardwareData.size());

    for (int i = 0; i < hardwareData.size(); ++i) {
        output += QString("0x%1, ").arg(hardwareData[i], 2, 16, QChar('0')).toUpper();
        if ((i + 1) % 16 == 0 && i < hardwareData.size() - 1) {
            output += "\n    ";
        }
    }

    if (output.endsWith(", ")) {
        output.chop(2);
    }
    output += "\n};";


    // === 顯示在視窗中 ===
    QDialog *dialog = new QDialog(this);
    dialog->setStyleSheet("QDialog { background-color: white; border: 1px solid #ccc; }");
    dialog->setWindowTitle("SH1106 .h 格式輸出");
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setPlainText(output);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Courier", 10));
    textEdit->setStyleSheet("background-color: white; color: black;");

    QPushButton *copyButton = new QPushButton("複製到剪貼簿", dialog);
    connect(copyButton, &QPushButton::clicked, [output]() {
        QApplication::clipboard()->setText(output);
    });

    QPushButton *closeButton = new QPushButton("Close", dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(textEdit);

    layout->addWidget(copyButton);
    layout->addWidget(closeButton);

    dialog->resize(500, 400);
    dialog->exec();
}


void OLEDWidget::paintEvent(QPaintEvent *event) {

    QWidget::paintEvent(event);

    QPainter painter(this);
    //int ScaleMultiple=1;


    // 步骤 1: 绘制 widget 的灰色背景，方便区分显示区域
    painter.fillRect(rect(), Qt::darkGray);

    // 步骤 2: 计算 OLED 图像的显示位置和大小

    int scaled_width = OledConfig::DISPLAY_WIDTH * scale;
    int scaled_height = OledConfig::DISPLAY_HEIGHT * scale;

    // 计算偏移量，使其在 widget 中居中显示
    int x_offset = (width() - scaled_width) / 2;
    int y_offset = (height() - scaled_height) / 2;

    QRect targetRect(x_offset, y_offset, scaled_width, scaled_height);

    /*1107修改 開始*/
    painter.drawImage(targetRect, m_image);
    /*1107修改 結束*/


    // 步骤 3: 绘制核心的 OLED 屏幕图像 (m_image)
    // 这是最高效的绘制方式，一次性将缓存的 QImage "贴"上去
    painter.drawImage(targetRect, m_image);

    // 步骤 4: 绘制边框和网格线，增强视觉效果
    // 4.1 绘制白色外边框
    painter.setPen(QPen(Qt::white, 1));
    // adjusted(-1, -1) 是为了让边框完全落在绘制区域内，避免被裁切
    painter.drawRect(targetRect.adjusted(0, 0, -1, -1));// adjusted 确保边框在内侧

    // 4.2 如果缩放比例足够大，绘制像素网格
    if (scale >= 4) {
        QPen grid_pen(QColor(128, 128, 128, 100), 1); // 半透明灰色
        painter.setPen(grid_pen);

        // 绘制垂直线
        for (int i = 1; i < OledConfig::DISPLAY_WIDTH; ++i) {
            painter.drawLine(x_offset + i * scale, y_offset, x_offset + i * scale, y_offset + scaled_height);
        }
        // 绘制水平线
        for (int j = 1; j < OledConfig::DISPLAY_HEIGHT; ++j) {
            painter.drawLine(x_offset, y_offset + j * scale, x_offset + scaled_width, y_offset + j * scale);
        }
    }

    if (m_pastePreviewActive && !m_pastePreviewImage.isNull()) {
        // 使用 QPainter 的透明度功能，效果更好且代码更简单
        painter.setOpacity(0.65); // 设置 65% 的不透明度

        // 计算预览图像在 widget 上的目标绘制矩形
        const QRect previewTargetRect(
            x_offset + m_pastePosition.x() * scale,
            y_offset + m_pastePosition.y() * scale,
            m_pastePreviewImage.width() * scale,
            m_pastePreviewImage.height() * scale
            );

        // [核心简化] 直接让 QPainter 绘制整个预览 QImage，无需我们手动循环
        painter.drawImage(previewTargetRect, m_pastePreviewImage);

        painter.setOpacity(1.0); // 绘制完毕后，恢复不透明度，以免影响后续绘制
    }


    // 2.2 绘制非画笔工具的拖拽预览 (如画线、画矩形)
    if (m_isDrawing && m_currentTool != Tool_Pen && m_currentTool != Tool_Select) {
        QPen previewPen(Qt::cyan, 1, Qt::DotLine); // 亮蓝色虚线，更像预览
        painter.setPen(previewPen);
        painter.setBrush(Qt::NoBrush); // 预览通常不填充

        // 将逻辑坐标的起始点和结束点转换为屏幕坐标的矩形
        QRectF previewLogicalRect(m_startPoint, m_endPoint);

        QRectF previewScreenRect(
            x_offset + previewLogicalRect.left() * scale,
            y_offset + previewLogicalRect.top() * scale,
            previewLogicalRect.width() * scale,
            previewLogicalRect.height() * scale
            );

        previewScreenRect = previewScreenRect.normalized();

        switch (m_currentTool) {
        case Tool_Line:
            painter.drawLine(x_offset + m_startPoint.x() * scale, y_offset + m_startPoint.y() * scale,
                             x_offset + m_endPoint.x() * scale,   y_offset + m_endPoint.y() * scale);
            break;
        case Tool_Rectangle:
            painter.drawRect(previewScreenRect);
            break;
        case Tool_FilledRectangle:
            // 填充矩形的预览可以只画框，也可以画半透明填充
            painter.setBrush(QColor(0, 0, 255, 50));
            painter.drawRect(previewScreenRect);
            break;
        case Tool_Circle:
            painter.drawEllipse(previewScreenRect);
            break;
        default:
            break;
        }
    }

    // 2.3 绘制选区虚线框 (最高层)
    if (m_isSelecting || m_selectedRegion.isValid()) {

        QRect rectToDraw = m_isSelecting
                               ? QRect(m_startPoint, m_endPoint).normalized()
                               : m_selectedRegion;

        QPen selectionPen(Qt::yellow, 2, Qt::DashLine);
        painter.setPen(selectionPen);
        painter.setBrush(Qt::NoBrush);

        QRect scaledRect(
            x_offset + rectToDraw.x() * scale,
            y_offset + rectToDraw.y() * scale,
            rectToDraw.width() * scale,
            rectToDraw.height() * scale
            );
        painter.drawRect(scaledRect);

    }

}
//mouse 三兄弟

void OLEDWidget::mousePressEvent(QMouseEvent *event) {

    // 步骤 1: 将 Qt 的 widget 坐标转换为我们的 OLED 逻辑坐标
    const QPoint oled_pos = convertToOLED(event->pos());


    // 步骤 2: [高优先级] 检查是否处于“贴上预览”模式
    if (m_pastePreviewActive) {
        if (event->button() == Qt::LeftButton) {
            // 如果是左键点击，确认贴上
            //commitPaste();
            m_dragStartPos = event->pos();
            m_dragStartPastePos = m_pastePosition;

        } else {
            // 任何其他按键 (右键, 中键) 都取消贴上
            m_pastePreviewActive = false;
            m_pastePreviewImage = QImage(); // 清空预览图像
            update(); // 更新画面以移除预览
        }
        return; // 贴上模式下，不进行其他任何绘图操作
    }

    // 步骤 3: 根据当前选择的工具，分发事件
    switch (m_currentTool) {
    case Tool_Select:
        // 选区工具的 press 事件单独处理
        handleSelectPress(event);
        break;

    case Tool_Pen:
        // --- 画笔工具的逻辑 ---
        m_isDrawing = true;     // 开始绘制状态
        m_startPoint = oled_pos; // 记录起点
        m_endPoint = oled_pos;   // 终点与起点相同

        if (event->button() == Qt::LeftButton) {
            // 左键：调用 model 画点 (应用笔刷大小)
            // [注意] 我们需要为 OledDataModel 添加一个带笔刷的 setPixel
            m_model.setPixel(oled_pos.x(), oled_pos.y(), true, m_brushSize);
            updateImageFromModel(); // 数据已变，立即更新画面
        } else if (event->button() == Qt::RightButton) {
            // 右键：调用 model 擦除点 (应用笔刷大小)
            m_model.setPixel(oled_pos.x(), oled_pos.y(), false, m_brushSize);
            updateImageFromModel(); // 数据已变，立即更新画面
        }
        break;

    case Tool_Line:
    case Tool_Rectangle:
    case Tool_FilledRectangle:
    case Tool_Circle:
        // --- 其他形状工具的逻辑 (它们的 press 逻辑都一样) ---
        if (event->button() == Qt::LeftButton) {
            m_isDrawing = true;     // 开始绘制状态
            m_startPoint = oled_pos; // 记录起点
            m_endPoint = oled_pos;   // 终点与起点相同
            update(); // 更新一下，以便 paintEvent 可以开始画预览
        }
        // 对于形状工具，右键点击可以理解为“取消本次操作”，所以什么都不做
        break;

    default:
        // 其他未知工具，不做任何事
        break;
    }


    // 调用基类实现
    QWidget::mousePressEvent(event);

}

void OLEDWidget::mouseMoveEvent(QMouseEvent *event) {


    // 步骤 1: 坐标转换，并发射信号让 MainWindow 显示
    const QPoint oled_pos = convertToOLED(event->pos());
    emit coordinatesChanged(oled_pos);

    // 步骤 2: [高优先级] 检查是否处于“贴上预览”模式
    if (m_pastePreviewActive) {
        m_pastePosition = oled_pos; // 更新预览图的左上角位置
        update();                   // 触发重绘，让 paintEvent 画出移动后的预览
        return;                     // 贴上模式下，不进行其他任何操作
    }

    // Step 2.5: 選取工具獨立處理，不依賴 m_isDrawing
    if (m_currentTool == Tool_Select && m_isSelecting) {
        handleSelectMove(event);
        return;
    }

    // Step 3: 其他工具才用 m_isDrawing 判斷
    if (!m_isDrawing) {
        QWidget::mouseMoveEvent(event);
        return;
    }



    // 步骤 4: 更新当前鼠标位置作为“终点”
    m_endPoint = oled_pos;

    // 步骤 5: 根据当前工具，分发事件
    switch (m_currentTool) {
    case Tool_Select:
        // 选区工具的 move 事件单独处理
        handleSelectMove(event);
        break;

    case Tool_Pen:
    { // 使用花括号创建一个作用域
        // --- 画笔工具的逻辑：实时绘制 ---
        bool isLeftButton = event->buttons() & Qt::LeftButton;
        bool isRightButton = event->buttons() & Qt::RightButton;

        if (isLeftButton || isRightButton) {
            // 指挥“绘图引擎”在起点和当前点之间，用指定笔刷画一条线
            m_model.drawLine(m_startPoint.x(), m_startPoint.y(),
                             m_endPoint.x(), m_endPoint.y(),
                             isLeftButton,      // true 为画，false 为擦除
                             m_brushSize);      // 使用当前笔刷大小

            // 【重要】将当前点更新为下一次移动的“起点”，以形成连续轨迹
            m_startPoint = m_endPoint;

            // 数据模型已改变，立即同步视图
            updateImageFromModel();
        }
        break;
    }

    case Tool_Line:
    case Tool_Rectangle:
    case Tool_FilledRectangle:
    case Tool_Circle:
        // --- 其他形状工具的逻辑：只更新预览 ---
        // 我们只需要更新 m_endPoint (前面已完成)，然后触发一次重绘。
        // 真正的绘制逻辑在 paintEvent 中，它会根据 m_startPoint 和 m_endPoint 绘制预览线框。
        update();
        break;

    default:
        break;
    }

    QWidget::mouseMoveEvent(event);
}

void OLEDWidget::mouseReleaseEvent(QMouseEvent *event) {



    // ✅ 選取工具獨立處理
    if (m_currentTool == Tool_Select && m_isSelecting) {
        handleSelectRelease(event);
        return;
    }

    // 步骤 1: [高优先级] 检查是否正在进行绘图或选区操作
    // 如果 m_isDrawing 为 false，说明可能只是一个简单的点击然后释放，
    // 或者是一个被 press 事件取消的操作，直接返回即可。
    if (!m_isDrawing) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    // 步骤 2: 更新终点坐标 (确保即使用户只是点击一下没有移动，终点也是有效的)
    const QPoint oled_pos = convertToOLED(event->pos());
    m_endPoint = oled_pos;


    // 步骤 3: 根据当前工具，分发事件
    switch (m_currentTool) {
    /*case Tool_Select:
        // 选区工具的 release 事件单独处理
        handleSelectRelease(event);
        break;*/

    case Tool_Pen:
        // 对于画笔工具，所有的绘制工作都在 press 和 move 事件中完成了。
        // release 事件只需要做一件事：结束“正在绘制”的状态。
        // 无需调用任何绘图函数。
        break;

    case Tool_Line:
        // --- 直线工具：最终绘制 ---
        if (event->button() == Qt::LeftButton) {
            // 指挥“绘图引擎”在起点和终点之间，画一条 1 像素宽的线
            m_model.drawLine(m_startPoint.x(), m_startPoint.y(),
                             m_endPoint.x(), m_endPoint.y(),
                             true,m_brushSize); // on=true, brushSize=1
            updateImageFromModel(); // 数据已变，同步视图
        }
        break;

    case Tool_Rectangle:
        // --- 矩形工具：最终绘制 ---
        if (event->button() == Qt::LeftButton) {
            const QRect rect = QRect(m_startPoint, m_endPoint).normalized();
            // 指挥引擎画一个不填充的、1 像素宽的矩形
            m_model.drawRectangle(rect.x(), rect.y(), rect.width(), rect.height(),
                                  true, false, m_brushSize); // on=true, fill=false, brushSize=1
            updateImageFromModel();
        }
        break;

    case Tool_FilledRectangle:
        // --- 实心矩形工具：最终绘制 ---
        if (event->button() == Qt::LeftButton) {
            const QRect rect = QRect(m_startPoint, m_endPoint).normalized();
            // 指挥引擎画一个填充的、1 像素宽边框的矩形
            m_model.drawRectangle(rect.x(), rect.y(), rect.width(), rect.height(),
                                  true, true, m_brushSize); // on=true, fill=true, brushSize=1
            updateImageFromModel();
        }
        break;

    case Tool_Circle:
        // --- 圆形工具：最终绘制 ---
        if (event->button() == Qt::LeftButton) {
            // 指挥引擎在起点和终点构成的矩形内，画一个 1 像素宽的椭圆
            m_model.drawCircle(m_startPoint, m_endPoint, m_brushSize); // brushSize=1
            updateImageFromModel();
        }
        break;

    default:
        break;
    }

    // 步骤 4: [重要] 无论是什么工具，在鼠标释放后，都必须结束“正在绘制”的状态
    m_isDrawing = false;

    // 步骤 5: [重要] 调用 update() 清除所有预览图形
    // 因为 m_isDrawing 现在是 false，paintEvent 中的预览绘制逻辑将不会执行，
    // 从而达到清除蓝色预览线框和黄色选区框（如果是正在选区）的效果。
    update();

    QWidget::mouseReleaseEvent(event);
}

//mouse 三兄弟


void OLEDWidget::wheelEvent(QWheelEvent *event)
{
    // 步骤 1: 检查用户是否同时按下了 "Control" 键
    // event->modifiers() 返回一个标志位，表示事件发生时有哪些修饰键 (Ctrl, Shift, Alt) 被按下
    // 我们用位与 (&) 运算来检查是否包含了 Qt::ControlModifier 这个标志

    if (event->modifiers() & Qt::ControlModifier) {

        // 步骤 2: 获取滚轮滚动的方向和幅度
        // event->angleDelta().y() 返回垂直方向的滚动角度。
        // 通常，向前滚动 (放大) 是一个正值 (如 120)，向后滚动 (缩小) 是一个负值 (如 -120)。

        int delta = event->angleDelta().y();
        if (delta > 0)// 向前滚动，放大
            setScale(scale + 1);
        else if (delta < 0 && scale > 1)// 向后滚动，缩小
            setScale(scale - 1);


        // 步骤 3: “接受”这个事件
        // event->accept() 是在告诉 Qt：“这个滚轮事件我已经处理了，
        // 你不需要再把它传递给父 widget 或进行其他默认处理了。”
        event->accept();
    } else {

        // 步骤 4: 如果没有按下 Ctrl 键，我们不做任何特殊处理
        // 比如，用户可能只是想滚动外面的 QScrollArea (如果我们未来有的话)
        // 所以，我们应该把这个事件交给基类去进行它的默认处理。
        QWidget::wheelEvent(event);
    }
}


void OLEDWidget::leaveEvent(QEvent *event)
{
    // 當滑鼠離開 widget 時，發送一個無效座標 (-1, -1)
    emit coordinatesChanged(QPoint(-1, -1));
    QWidget::leaveEvent(event);
}
 // 确保包含了 QKeyEvent

void OLEDWidget::keyPressEvent(QKeyEvent *event)
{
    // 步骤 1: [高优先级] 检查当前是否处于“贴上预览”模式
    if (m_pastePreviewActive) {

        // 步骤 2: 检查用户按下的是否是 "Escape" 键
        if (event->key() == Qt::Key_Escape) {

            // 如果是，执行取消操作
            m_pastePreviewActive = false;   // 1. 关闭贴上预览模式
            m_pastePreviewImage = QImage(); // 2. 清空预览图像数据 (释放内存)
            update();                       // 3. 请求重绘，让预览图从屏幕上消失

            event->accept(); // 4. "消费"掉这个事件，表示我们已经处理了它
            return;          // 5. 直接返回，不再进行其他处理
        }
    }

    // 步骤 3: 如果不是我们关心的特殊情况，就把事件交给基类处理
    // 这很重要，因为基类可能会处理其他按键，比如 Tab 键的焦点切换等
    QWidget::keyPressEvent(event);
}


/**
 * @brief 根據資料模型（OledDataModel）的狀態，重新生成用於顯示的 QImage 緩衝區。
 *
 * 此函數扮演著將資料模型層的邏輯狀態（像素的開/關）轉換為視圖層的視覺呈現（像素的顏色）的關鍵角色。
 * 它會遍歷顯示器上的每一個像素點，逐一向資料模型 `m_model` 查詢該點的狀態。
 * 根據查詢結果（true 為亮，false 為暗），在內部成員 `m_image` 的對應位置上設置預先定義的亮點或暗點顏色。
 *
 * 完成整個圖像的更新後，它會呼叫 `update()` 來觸發一次重繪請求（schedule a repaint）。
 * 這會告知 Qt 在下一個事件循環中呼叫此 Widget 的 `paintEvent()`，
 * `paintEvent()` 接著會將這個更新後的 `m_image` 繪製到螢幕上。
 *
 * @note 這是一個像素級別的操作。在模型數據發生任何改變後都應該呼叫此函數，以確保使用者介面與資料模型保持同步。
 * @see paintEvent(QPaintEvent*)
 * @see OledDataModel::getPixel(int, int) const
 */
void OLEDWidget::updateImageFromModel(){

    const QColor pixelOnColor = QColor(135, 206, 250); // 淺藍色
    const QColor pixelOffColor = Qt::black;

    // 安全检查：确保 m_image 已经被正确初始化
    // (虽然我们的构造函数保证了这一点，但这是一个好的防御性编程习惯)
    if (m_image.isNull()) {
        m_image = QImage(OledConfig::DISPLAY_WIDTH, OledConfig::DISPLAY_HEIGHT, QImage::Format_RGB888);
    }


    // 使用两层 for 循环，遍历屏幕上的每一个像素坐标 (x, y)
    for (int y = 0; y < OledConfig::DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < OledConfig::DISPLAY_WIDTH; ++x) {

            // 步骤 1: 从数据模型查询该点的状态 (是亮是暗？)
            // m_model.getPixel(x, y) 返回一个 bool 值
            bool isPixelOn = m_model.getPixel(x, y);

            // 步骤 2: 根据查询结果，在 m_image 的相同坐标位置上设置颜色
            if (isPixelOn) {
                m_image.setPixelColor(x, y, pixelOnColor);
            } else {
                m_image.setPixelColor(x, y, pixelOffColor);
            }

            // 上面的 if/else 可以简化为一行三元表达式，效果完全相同:
            // m_image.setPixelColor(x, y, isPixelOn ? pixelOnColor : pixelOffColor);
        }
    }

    // 步骤 3: 请求重绘
    // 在更新完 m_image 的所有像素后，调用 update() 来通知 Qt：
    // “我的画面内容已经准备好了，请在下一个刷新周期调用我的 paintEvent()！”
    update();
}


QPoint OLEDWidget::convertToOLED(const QPoint &pos)
{

    // 步骤 1: 计算 OLED 图像在 widget 中居中显示的几何信息
    // [注意] 这部分计算逻辑必须与 paintEvent() 中的完全一致！
    const int scaled_width = OledConfig::DISPLAY_WIDTH * scale;
    const int scaled_height = OledConfig::DISPLAY_HEIGHT * scale;
    const int x_offset = (width() - scaled_width) / 2;
    const int y_offset = (height() - scaled_height) / 2;

    // 步骤 2: 将窗口坐标减去偏移量，得到在缩放后图像上的相对坐标
    int relative_x = pos.x() - x_offset;
    int relative_y = pos.y() - y_offset;


    // 步骤 3: 将相对坐标除以缩放比例，得到最终的 OLED 逻辑坐标
    int oled_x = relative_x / scale;
    int oled_y = relative_y / scale;


    // 步骤 4: [重要] 边界限制 (Clamping)
    // 即使鼠标点击在灰色背景区域（绘图区之外），
    // 我们也应该将坐标限制在有效的 OLED 范围内 (0-127, 0-63)。
    // 这可以防止后续的绘图操作访问到无效的坐标。
    oled_x = std::clamp(oled_x, 0, OledConfig::DISPLAY_WIDTH - 1);
    oled_y = std::clamp(oled_y, 0, OledConfig::DISPLAY_HEIGHT - 1);

    // 步骤 5: 返回计算出的逻辑坐标
    return QPoint(oled_x, oled_y);
}


void OLEDWidget::handleSelectPress(QMouseEvent *event)
{
    // 步骤 1: 检查是否是鼠标左键按下的事件
    // 通常，我们只用左键来开始一个新的选区。
    if (event->button() == Qt::LeftButton) {
        // 步骤 2: 开启“正在选择”模式
        // 这个布尔标志位 m_isSelecting 非常重要，
        // 它会告诉 mouseMoveEvent 和 paintEvent 当前正处于选区绘制状态。


        m_isSelecting = true;

        // 步骤 3: 转换坐标并记录选区的“起始点”
        const QPoint oled_pos = convertToOLED(event->pos());
        m_startPoint = oled_pos;

        // 步骤 4: 同时将“结束点”也设置为起始点
        // 因为在刚按下的瞬间，选区是一个 0x0 大小的点。
        m_endPoint = oled_pos;

        // 步骤 5: 清除上一次的旧选区
        // m_selectedRegion 存储的是已经“确定”的选区。
        // 一旦开始一个新的选区操作，旧的就应该作废。
        m_selectedRegion = QRect(); // QRect() 创建一个无效的矩形

        // 步骤 6: 请求重绘
        // 调用 update() 会让 paintEvent 被触发，
        // paintEvent 会根据 m_isSelecting = true 来绘制一个（目前还看不见的）黄色的虚线框。
        update();


        // 步骤 7: 接受事件
        event->accept();
    }
    // 如果是右键或其他按键，我们什么都不做，事件会继续传递
}

void OLEDWidget::handleSelectMove(QMouseEvent *event)
{
    // 步骤 1: 安全检查
    // 确保我们只在“正在选择”的状态下执行此逻辑。
    // 这个检查虽然在 mouseMoveEvent 的调用处已经做过，
    // 但在函数内部再做一次，是一种更安全的防御性编程。
    if (!m_isSelecting) {
        return;
    }

    // 步骤 2: 获取当前鼠标的 OLED 逻辑坐标
    const QPoint oled_pos = convertToOLED(event->pos());

    // 步骤 3: 更新选区的“结束点”
    // m_startPoint 在 handleSelectPress 时已经固定下来，我们不去动它。
    // 我们只需要不断地更新 m_endPoint 即可。
    m_endPoint = oled_pos;

    // 步骤 4: 请求重绘以更新预览
    // 这是整个函数最关键的一步。
    // 调用 update() 会触发 paintEvent。
    // paintEvent 会看到 m_isSelecting 是 true，
    // 然后它会使用最新的 m_startPoint 和 m_endPoint
    // 来绘制一个动态变化的黄色虚线矩形。
    // 这就为用户提供了实时的视觉反馈。
    update();

    // 步骤 5: 接受事件
    event->accept();
}

void OLEDWidget::handleSelectRelease(QMouseEvent *event)
{
    // 步骤 1: 安全检查
    // 确保我们是在“正在选择”的状态下松开【鼠标左键】。
    // 如果不是，或者松开的是右键，则忽略此事件。
    if (!m_isSelecting || event->button() != Qt::LeftButton) {
        return;
    }

    // 步骤 2: 关闭“正在选择”模式
    // 这是关键的状态清理步骤。将开关关闭，
    // mouseMoveEvent 就不会再响应选区逻辑了。
    m_isSelecting = false;

    // 步骤 3: 获取最终的鼠标位置并更新“结束点”
    const QPoint oled_pos = convertToOLED(event->pos());
    m_endPoint = oled_pos;

    // 步骤 4: 计算并“固化”最终的选区矩形
    // 使用 QRect 的构造函数和 .normalized() 来创建一个有效的矩形。
    // .normalized() 能确保矩形的左上角坐标值总是小于右下角坐标值，
    // 即使你是从右下往左上拖动鼠标来创建选区的。
    QRect finalRegion = QRect(m_startPoint, m_endPoint).normalized();

    // 步骤 5: [重要] 有效性检查
    // 如果用户只是点击了一下就松开，或者拖动范围太小，
    // 我们应该认为这是一个无效的选区，直接丢弃它。
    // 这样可以避免后续的复制等操作处理一个 1x1 或 0x0 的无效区域。

    if (finalRegion.width() < 2 || finalRegion.height() < 2) {

        // 如果区域太小，我们就创建一个无效的 QRect，相当于清除了选区。
        m_selectedRegion = QRect();


    } else {

#ifdef newcode_Buffer
        // ==========================================================
        // 【核心逻辑】在这里清空上一次复制/剪下的内容
        // ==========================================================
        if (!m_selectionBuffer.isNull()) {
            m_selectionBuffer = QImage(); // 清空暂存区
        }
        // ==========================================================
#endif

        // 如果区域有效，就将其保存在 m_selectedRegion 成员变量中。
        // m_selectedRegion 现在存储了这次操作的最终成果。
        m_selectedRegion = finalRegion;
    }

    // 步骤 6: 请求最后一次重绘
    // 这次 update() 调用有两个目的：
    // 1. 因为 m_isSelecting 已经是 false，paintEvent 将不再根据 start/endPoint
    //    绘制动态的虚线框，清除了“过程”中的预览。
    // 2. 如果我们得到了一个有效的 m_selectedRegion，paintEvent 会根据它
    //    绘制一个“固化”的黄色虚线框，向用户展示最终的选区结果。
    // 3. 如果选区无效，m_selectedRegion 为空，paintEvent 将不会画任何框。


    update();

    event->accept();
}

// 在 oledwidget.cpp 中

/**
 * @brief 啟動「貼上預覽」模式。
 *
 * 此函數負責進入一個特殊的互動模式，在該模式下，一個半透明的預覽圖像會顯示在畫布上，
 * 並且其位置會跟隨滑鼠移動。這是貼上操作的第一步，讓使用者可以確定貼上的最終位置。
 *
 * 函數的具體工作包括：
 * 1. 驗證傳入的圖像資料是否有效。
 * 2. 設定狀態旗標 `m_pastePreviewActive` 為 `true`，以通知其他事件處理器（如 `paintEvent`）。
 * 3. 將傳入的 `logicalImage` 儲存到 `m_pastePreviewImage` 成員變數中。
 * 4. 初始化預覽圖像的初始位置 `m_pastePosition`（通常是左上角）。
 * 5. 觸發一次重繪（`update()`），以便 `paintEvent` 首次將預覽圖像繪製出來。
 *
 * @param[in] logicalImage 要進行預覽的圖像資料。此圖像必須是有效的且格式為 `QImage::Format_Mono`。
 * @note 此函數僅啟動視覺預覽，並不會修改底層的 `OledDataModel`。實際的像素寫入操作
 *       由 `applyPaste()` 函數在使用者確定位置後（例如，點擊滑鼠）執行。
 * @see applyPaste()
 * @see paintEvent()
 * @see mouseMoveEvent()
 * @see handleCopy()
 */
void OLEDWidget::startPastePreview(const QImage &logicalImage)
{
    // 步骤 1: 安全检查
    // 检查传入的 QImage 是否有效，并且是我们期望的内部逻辑格式 (Format_Mono)。
    // 这是一个好的防御性编程习惯。
    if (logicalImage.isNull() || logicalImage.format() != QImage::Format_Mono) {

        // 如果数据无效，就确保我们不会进入贴上模式
        m_pastePreviewActive = false;
        m_pastePreviewImage = QImage(); // 清空可能存在的旧数据
        return;
    }

    // 步骤 2: 开启“贴上预览”模式
    // 这是“总开关”，告诉其他事件处理器（如 paintEvent, mouseMoveEvent）
    // 当前正处于特殊的贴上模式。
    m_pastePreviewActive = true;

    // 步骤 3: 保存预览图像数据
    // 我们直接将传入的 logicalImage 存储到成员变量 m_pastePreviewImage 中。
    // QImage 是隐式共享的 (implicitly shared)，所以这个赋值操作非常快，
    // 它只复制了图像的元数据和一个指向数据块的指针。
    m_pastePreviewImage = logicalImage;


    // 步骤 4: 初始化预览图像的显示位置
    // 通常，我们让预览图像一开始出现在屏幕的左上角 (0, 0)。
    // 在 mouseMoveEvent 中，这个位置会跟随鼠标实时更新。
    m_pastePosition = QPoint(0, 0);

    // 步骤 5: 请求重绘
    // 调用 update() 会触发 paintEvent。
    // paintEvent 会检查到 m_pastePreviewActive 为 true，
    // 然后调用我们之前写好的逻辑，在 (0, 0) 位置首次绘制出半透明的 m_pastePreviewImage。
    update();

}



// 在 oledwidget.cpp 中

void OLEDWidget::commitPaste()
{
    // 步骤 1: 安全检查
    // 确保我们确实处于贴上模式，并且有有效的预览图像数据。
    if (!m_pastePreviewActive || m_pastePreviewImage.isNull()) {
        return;
    }
    // 步骤 2: 遍历预览图像的每一个像素
    // 我们需要将 m_pastePreviewImage 中的像素，一个一个地“复制”到 m_model 中。
    for (int y = 0; y < m_pastePreviewImage.height(); ++y) {
        for (int x = 0; x < m_pastePreviewImage.width(); ++x) {

            // 步骤 3: 检查预览图像在该点是否有像素 (是否为亮色)
            // .pixelIndex() 对于单色图 (Format_Mono) 来说，
            // 返回 1 代表前景色 (亮)，返回 0 代表背景色 (暗)。
            if (m_pastePreviewImage.pixelIndex(x, y) == 1) {

                // 步骤 4: 如果有像素，计算它在主数据模型 m_model 中的目标坐标
                int targetX = m_pastePosition.x() + x;
                int targetY = m_pastePosition.y() + y;

                // 步骤 5: [核心] 调用 m_model 的 setPixel，将数据写入核心模型
                // 我们不需要关心 setPixel 内部是怎么实现的，只需要相信它能
                // 正确地在 (targetX, targetY) 位置上点亮一个像素。
                // 这里的笔刷大小应为 1，因为我们是逐像素复制。
                m_model.setPixel(targetX, targetY, true, 1);
            }
        }
    }

    // 步骤 6: [重要] 结束并清理贴上模式
    m_pastePreviewActive = false;
    m_pastePreviewImage = QImage(); // 清空预览图像数据，释放内存

    // 步骤 7: [重要] 从数据模型更新主显示图像
    // 因为我们刚刚通过多次 setPixel 修改了 m_model，
    // 所以必须调用 updateImageFromModel() 来将这些改动同步到 m_image 上。
    updateImageFromModel();
}

QImage OLEDWidget::getCurrentImage() const
{
    // m_image 是您用來在 paintEvent 中繪圖的那個 QImage
    return m_image;
}

/**
     * @brief 從一個邏輯格式的 QImage 更新整個 OLED 顯示內容。
     * @param image 來源圖片，必須是 QImage::Format_Mono 格式。
     */
void OLEDWidget::updateOledFromImage(const QImage& image){
    // 步驟 1: 呼叫外部工具函式來處理資料模型的更新
    // 我們把 m_model 的指標傳遞給它，讓它去操作
    OledDataConverter::updateModelFromImage(&m_model, image);

    // 步驟 2: 資料模型已經被外部工具更新了，
    //         現在我們只需要同步 View 的顯示即可。
    updateImageFromModel();
}


/**
 * @brief [SLOT] 處理「複製」操作的槽函數。
 *
 * 當使用者觸發複製動作時（例如透過選單或快捷鍵），此槽函數會被呼叫。
 * 它的主要工作是從資料模型 `m_model` 中，將目前選取區域 (`m_selectedRegion`) 的像素資料
 * 複製成一個獨立的、邏輯格式的 QImage。
 *
 * @note 此函數的行為是「複製並立即準備貼上」。它不會將資料存放到系統的剪貼簿，
 *       而是直接呼叫 `startPastePreview()`，讓使用者可以立刻看到複製內容的預覽並移動它。
 *       如果當前沒有有效的選取區域，此函數將不會執行任何操作。
 *
 * @see OledDataModel::copyRegionToLogicalFormat()
 * @see startPastePreview()
 * @see handlePaste()
 */
void OLEDWidget::handleCopy(){
    // 步驟 1: 檢查是否有有效的選取區域
    if (!m_selectedRegion.isValid())
    {
        return; // 沒有選取框就不做
    }
#ifdef newcode_Buffer
    m_selectionBuffer = m_model.copyRegionToLogicalFormat(m_selectedRegion);
#endif

#ifdef newcode_clipboard


    // 步驟 2: 從 m_model 讀取像素並儲存到剪貼簿
    // [修改] 將結果存到 m_clipboardImage，而不是直接啟動預覽
    m_clipboardImage = m_model.copyRegionToLogicalFormat(m_selectedRegion);

    // 注意：複製後不再自動進入貼上模式，這符合標準軟體的行為

#endif

#ifdef oldcode
    // 步驟 2 : 從 m_model 讀取像素
    // 让 model 把选区复制成一个逻辑图像。

    QImage copiedLogicalData = m_model.copyRegionToLogicalFormat(m_selectedRegion);

    // QVector<uint8_t> hardwareData = OledDataModel::convertLogicalToHardwareFormat(copiedLogicalData);

    //  步骤 2: 直接用这个逻辑图像启动贴上预览
    startPastePreview(copiedLogicalData);
#endif

}

/**
 * @brief [SLOT] 處理「剪下」操作的槽函數。
 *
 * 剪下操作是一個複合動作，依序執行以下三件事：
 * 1.  **複製 (Copy)**: 將目前選取區域 (`m_selectedRegion`) 的像素資料複製到一個暫存的 QImage 中。
 * 2.  **刪除 (Delete)**: 將原選取區域的像素從資料模型 (`m_model`) 中清除（設為熄滅）。
 * 3.  **貼上預覽 (Paste Preview)**: 立刻進入「貼上預覽」模式，讓使用者可以移動剛剪下的內容到新位置。
 *
 * 如果當前沒有有效的選取區域，此函數將不會執行任何操作。
 *
 * @see handleCopy()
 * @see OledDataModel::copyRegionToLogicalFormat()
 * @see OledDataModel::drawRectangle()
 * @see startPastePreview()
 */
void OLEDWidget::handleCut() {
    // 步驟 1: 檢查是否有有效的選取區域
    if (!m_selectedRegion.isValid()) {
        return; // 沒有選取框就不做任何事
    }

#ifdef newcode_clipboard
    // ================== 1. 複製到剪貼簿 (Copy to Clipboard) ==================
    // [修改] 將選區內容存到 m_clipboardImage
    m_clipboardImage = m_model.copyRegionToLogicalFormat(m_selectedRegion);
    if (m_clipboardImage.isNull()) {
        return;
    }

#endif

#ifdef newcode_Buffer
    m_selectionBuffer = m_model.copyRegionToLogicalFormat(m_selectedRegion);
    if (m_selectionBuffer.isNull()) return;

#endif

#ifdef oldcode
    // ================== 1. 複製 (Copy) ==================
    // [重用!] 直接呼叫 model 的現有功能，將選區內容複製成一個邏輯圖像。
    // 這一步與 handleCopy() 完全相同。
    QImage cutLogicalImage = m_model.copyRegionToLogicalFormat(m_selectedRegion);
    if (cutLogicalImage.isNull()) {
        // 如果因某些原因複製失敗，則中止操作
        return;
    }
#endif

    // ================== 2. 刪除 (Delete) ==================
    // [優化!] 指揮 model 在原選區位置畫一個「熄滅的、實心的」矩形，
    // 這比逐點清除像素的效率高得多。
    m_model.drawRectangle(
        m_selectedRegion.x(),
        m_selectedRegion.y(),
        m_selectedRegion.width(),
        m_selectedRegion.height(),
        false, // on = false 代表「擦除」
        true,  // fill = true 代表「實心」
        1      // 筆刷大小在此不重要
        );



#ifdef newcode_clipboard
    // 因為資料模型已經被修改 (原區域被清空)，必須同步顯示
    updateImageFromModel();

    // ================== 3. 清理選區 ==================
    m_selectedRegion = QRect();
    update(); // 更新畫面以移除選取框

#endif


#ifdef newcode_Buffer

    updateImageFromModel();
    m_selectedRegion = QRect();
    update();

#endif

#ifdef oldcode

    // ================== 3. 貼上預覽 (Paste Preview) ==================
    // [重用!] 呼叫現有的輔助函式來啟動貼上預覽模式
    startPastePreview(cutLogicalImage);

    // [體驗優化] 讓預覽圖的初始位置出現在原本被剪下的地方，而不是左上角 (0,0)
    m_pastePosition = m_selectedRegion.topLeft();

    // 再次觸發更新，確保 paintEvent 能在正確的位置畫出預覽圖
    update();
#endif

    // [清理] 剪下後，原有的選取框應該消失
    m_selectedRegion = QRect();
}

#ifdef newcode_Buffer

/**
 * @brief [SLOT] 處理「貼上」操作的槽函數。
 *
 * 當使用者觸發貼上動作時，此槽函數被呼叫。
 * 它會檢查內部剪貼簿 (`m_clipboardImage`) 是否有有效的圖像資料。
 * 如果有，它會呼叫 `startPastePreview()`，使用剪貼簿中的圖像
 * 來啟動一個新的貼上預覽流程，讓使用者可以決定貼上的位置。
 *
 * @note 此函數可以被多次呼叫，每次都會從同一個剪貼簿內容創建一個新的貼上預覽。
 * @see startPastePreview()
 * @see m_clipboardImage
 */
void OLEDWidget::handlePaste()
{
    // 步驟 1: 檢查剪貼簿是否為空
    if (m_selectionBuffer.isNull()) {
        return; // 剪貼簿沒東西，就不做任何事
    }

    // 步驟 2: 使用剪貼簿的內容來啟動貼上預覽
    // 每次呼叫 handlePaste，都會開始一個全新的貼上過程
    startPastePreview(m_selectionBuffer);
}

#endif
