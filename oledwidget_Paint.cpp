/* ******************Copyright (C) 2025 Ethan Yang *****************************
 * @file    oledwidget.cpp
 * @author  Ethan and ai
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

#include "oledwidget_Paint.h"
#include "oled_datamodel.h"

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

//留下paintEvent
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

        qDebug() << "PaintEvent: Drawing paste preview. Paste Preview Active:" << m_pastePreviewActive;

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
    } else {
        if (m_pastePreviewActive) qDebug() << "预览图像为空";
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

        }/*else {
            // 任何其他按键 (右键, 中键) 都取消贴上
            m_pastePreviewActive = false;
            m_pastePreviewImage = QImage(); // 清空预览图像
            update(); // 更新画面以移除预览
        }*/
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
    update();
    QWidget::mouseReleaseEvent(event);
}

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

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {

        // 如果是，执行确认贴上操作
        commitPaste();
        m_pastePreviewActive = false;   // 贴上完成后，关闭贴上预览模式
        m_pastePreviewImage = QImage(); // 清空预览图像数据
        update();                       // 请求重绘，让预览图从屏幕上消失

        event->accept(); // 消费掉这个事件
        return;          // 直接返回
    }


    // 步骤 3: 如果不是我们关心的特殊情况，就把事件交给基类处理
    // 这很重要，因为基类可能会处理其他按键，比如 Tab 键的焦点切换等
    QWidget::keyPressEvent(event);
}


//留下updateImageFromModel
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




