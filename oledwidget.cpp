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


std::vector<uint8_t> OLEDWidget::getHardwareBuffer() const
{
    // 直接返回从数据模型翻译过来的硬体 buffer
    return m_model.getHardwareBuffer();
}

// ================== 新增的 SLOT ==================
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


void OLEDWidget::handleCopy(){

    // 步驟 1: 檢查是否有有效的選取區域
    if (!m_selectedRegion.isValid())
    {
        qDebug() << "[handleCopy] 沒有選取區域，直接 return";
        return; // 沒有選取框就不做
    }

    // 步驟 2 : 從 m_model 讀取像素
    // 我们将选区内的像素数据读取到一个临时的、直观的逻辑格式中。
    // QImage 是一个非常好的选择，因为它就是一种逻辑格式。

    QImage copiedLogicalData = m_model.copyRegionToLogicalFormat(m_selectedRegion);

    // 步骤 3: "將讀取資料備份，把備份的資料轉換成硬體格式"
    // 这个转换现在由一个专门的函数来完成。
    // 我们把上一步得到的逻辑数据传入，让它翻译成硬件格式。

    QVector<uint8_t> hardwareData = OledDataModel::convertLogicalToHardwareFormat(copiedLogicalData);

    // 步骤 4: (您的第五點) 启动贴上预览模式
    startPastePreview(hardwareData, copiedLogicalData.width(), copiedLogicalData.height());

}


void OLEDWidget::showBufferDataAsHeader()
{
    // 步骤 1: 确定要导出的区域 (有选区就用选区，否则用整个屏幕)
    QRect region = m_selectedRegion.isValid() ? m_selectedRegion :
                       QRect(0, 0, OledConfig::DISPLAY_WIDTH, OledConfig::DISPLAY_HEIGHT);

    // 步骤 2: [复用!] 调用 model 将该区域转换为逻辑图像 QImage
    QImage logicalData = m_model.copyRegionToLogicalFormat(region);

    if (logicalData.isNull()) {
        qDebug() << "showBufferDataAsHeader: Failed to copy region to logical format.";
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






void OLEDWidget::paintEvent(QPaintEvent *) {

    QPainter p(this);
    p.fillRect(rect(), Qt::darkGray); // 背景色（方便看）
    if (img.isNull()) return;

    if (m_pastePreviewActive) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(128,128,128,128)); // 半透明灰色

        //QPainter painter(this);
        //painter.setPen(Qt::NoPen);
        //painter.setBrush(Qt::gray); // 半透明預覽色


        int pages = (m_pasteHeight + 7) / 8;
        int index = 0;
        for (int x = 0; x < m_pasteWidth; ++x) {
            for (int page = 0; page < pages; ++page) {
                if (index >= m_pasteData.size()) break;
                uint8_t byte = m_pasteData[index++];
                for (int bit = 0; bit < 8; ++bit) {
                    int y = m_pastePosition.y() + page*8 + bit;
                    if (y >= OledConfig::DISPLAY_HEIGHT) break;
                    if (byte & (1 << bit)) {
                        p.drawRect((m_pastePosition.x()+x)*scale,y*scale,scale, scale);
                    }
                }
            }
        }
    }

    if (m_isSelecting || m_isDraggingSelection) {
        QPen pen(Qt::green);
        pen.setStyle(Qt::DashLine);
        p.setPen(pen);
        p.drawRect(m_selectedRegion);
    }

    // --- 计算 OLED 图像的显示区域 (基于 DISPLAY_WIDTH 和 DISPLAY_HEIGHT) ---
    // OLED 屏幕的实际像素尺寸
    int oled_pixel_width = OledConfig::DISPLAY_WIDTH;
    int oled_pixel_height = OledConfig::DISPLAY_HEIGHT;

    // --- 计算 OLED 图像的显示区域 ---
    int scaled_width = img.width() * scale;
    int scaled_height = img.height() * scale;

    int x_offset = (width() - scaled_width) / 2;
    int y_offset = (height() - scaled_height) / 2;

    // 绘制实际的 OLED 像素内容
    // 這裡直接使用 p.drawImage 比逐像素繪製更有效率，如果 img 是 QImage 類型
    // 如果 img 是 QImage，這裡可以直接寫：
    p.drawImage(QRect(x_offset, y_offset, scaled_width, scaled_height), img);

    // 1. 绘制一个清晰的白色外边框
    p.setPen(QPen(Qt::white, 1));
    p.drawRect(x_offset, y_offset, scaled_width - 1, scaled_height - 1);

    // 2. 绘制格线 (可选，但推荐)
    if (scale >= 4) {
        QPen grid_pen(QColor(128, 128, 128, 100), 1);
        p.setPen(grid_pen);

        for (int i = 1; i < img.width(); ++i) {
            p.drawLine(x_offset + i * scale, y_offset, x_offset + i * scale, y_offset + scaled_height);
        }
        for (int j = 1; j < img.height(); ++j) {
            p.drawLine(x_offset, y_offset + j * scale, x_offset + scaled_width, y_offset + j * scale);
        }
    }

    // 【核心改動在這裡】直接用 QPainter 繪製預覽圖形
    if (m_isDrawing && m_currentTool != Tool_Pen) {
        // 設定預覽線的樣式
        QPen previewPen(Qt::blue, 1); // 藍色，1像素寬
        // previewPen.setStyle(Qt::DotLine); // 可以設定虛線效果，讓預覽更明顯
        p.setPen(previewPen);
        p.setBrush(Qt::NoBrush); // 預覽通常不填滿

        // 將 m_startPoint 和 m_endPoint 轉換為螢幕上的像素座標
        int screen_x0 = x_offset + m_startPoint.x() * scale;
        int screen_y0 = y_offset + m_startPoint.y() * scale;
        int screen_x1 = x_offset + m_endPoint.x() * scale;
        int screen_y1 = y_offset + m_endPoint.y() * scale;

        // 計算矩形或圓形繪圖所需的正確座標和尺寸
        // 確保寬度和高度為正值，從左上角開始繪製
        int preview_x = std::min(screen_x0, screen_x1);
        int preview_y = std::min(screen_y0, screen_y1);
        int preview_w = std::abs(screen_x1 - screen_x0);
        int preview_h = std::abs(screen_y1 - screen_y0);

        switch (m_currentTool) {
        case Tool_Line:
            p.drawLine(screen_x0, screen_y0, screen_x1, screen_y1);
            break;
        case Tool_Rectangle:
            p.drawRect(preview_x, preview_y, preview_w, preview_h);
            break;
        case Tool_FilledRectangle:
            // 實心矩形的預覽可以選擇只畫邊框，或者畫半透明填充
            p.setBrush(QColor(0, 0, 255, 50)); // 半透明藍色填充
            p.drawRect(preview_x, preview_y, preview_w, preview_h);
            p.setBrush(Qt::NoBrush); // 畫完後恢復不填充
            break;
        case Tool_Circle:
            // 繪製橢圓，以拖曳的矩形作為外接矩形
            p.drawEllipse(preview_x, preview_y, preview_w, preview_h);
            break;
        default:
            break;
        }
    }

#ifdef SelectCopy

    if (m_isSelecting || m_selectedRegion.isValid()) {
            qDebug() << "選取框:" << m_selectedRegion;
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing, false);
        QPen pen(Qt::yellow, 1, Qt::DashLine); // 虛線框
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        QRect rect;

        if (m_isSelecting) {
            int x1 = std::min(m_startPoint.x(), m_endPoint.x());
            int y1 = std::min(m_startPoint.y(), m_endPoint.y());
            int x2 = std::max(m_startPoint.x(), m_endPoint.x());
            int y2 = std::max(m_startPoint.y(), m_endPoint.y());
            rect = QRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
        } else {
            rect = m_selectedRegion;
        }

        // 放大顯示（轉成 widget 座標）
        QRect scaledRect(
            rect.x() * scale,
            rect.y() * scale,
            rect.width() * scale,
            rect.height() * scale
            );

        painter.drawRect(scaledRect);
    }

        //qDebug() << "paintEvent: drawing rect" << rect << "-> GUI:" << guiRect;
#endif

}



void OLEDWidget::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();
        if (delta > 0)
            setScale(scale + 1);
        else if (delta < 0 && scale > 1)
            setScale(scale - 1);
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

//mouse 三兄弟
void OLEDWidget::mouseMoveEvent(QMouseEvent *event) {

    //選取貼上程式段開始
#ifdef SelectCopy
    if (m_currentTool == Tool_Select && m_isSelecting) {
        handleSelectMove(event);
    }

    if (m_pastePreviewActive && event->buttons() & Qt::LeftButton) {
        m_pastePosition = event->pos();
        update();
    }


#endif

    //選取貼上程式段結束

    if (!m_isDrawing) return;

    int oled_x = (event->pos().x() - (width() - img.width() * scale) / 2) / scale;
    int oled_y = (event->pos().y() - (height() - img.height() * scale) / 2) / scale;

    // 檢查座標是否在畫布內，避免預覽圖形畫到外面去
    oled_x = std::clamp(oled_x, 0, img.width() - 1);
    oled_y = std::clamp(oled_y, 0, img.height() - 1);

    // 【新增】發送信號，將當前座標廣播出去
    emit coordinatesChanged(QPoint(oled_x, oled_y));

    // 🔹 新增這行，讓拖曳時即時更新 m_endPoint
    m_endPoint = QPoint(oled_x, oled_y);

    if (event->buttons() & Qt::LeftButton) {
        if (m_currentTool == Tool_Pen) {
            // 左鍵筆工具：即時畫線
            // 從上一個點 (m_startPoint) 畫到當前點 (m_endPoint)
            drawLine(m_startPoint.x(), m_startPoint.y(), oled_x, oled_y, true,m_buffer);
            m_startPoint = QPoint(oled_x, oled_y);
            updateImageFromBuffer();
        }
        // 對於其他工具 (線、矩形、圓)，只需更新 m_endPoint
        // 實際繪製預覽線會在 paintEvent 中根據 m_startPoint 和 m_endPoint 進行

    } else if (event->buttons() & Qt::RightButton ){

        if (m_currentTool == Tool_Pen){
            // 右鍵筆工具：即時擦除線
            drawLine(m_startPoint.x(), m_startPoint.y(), oled_x, oled_y, false,m_buffer);
            m_startPoint = m_endPoint;
            updateImageFromBuffer();
        }

        // 對於其他工具，右鍵在 mousePressEvent 時已經被定義為取消操作，
        // 所以這裡不需要做額外處理。如果右鍵被按著移動，應該是在取消繪圖後，
        // 就不應該再進行繪圖邏輯。
    }
    update();
}

void OLEDWidget::mousePressEvent(QMouseEvent *event) {

    int oled_x = (event->pos().x() - (width() - img.width() * scale) / 2) / scale;
    int oled_y = (event->pos().y() - (height() - img.height() * scale) / 2) / scale;

    // 限制在畫布範圍內
    oled_x = std::clamp(oled_x, 0, img.width() - 1);
    oled_y = std::clamp(oled_y, 0, img.height() - 1);

#ifdef SelectCopy
    if (m_currentTool == Tool_Select) {
        handleSelectPress(event);
        return;
    }

    if (m_currentTool == Tool_Select) {
        QPoint pos = convertToOLED(event->pos());

        // 👉 右鍵拖曳選取框
        if (event->button() == Qt::RightButton) {
            if (m_selectedRegion.contains(pos)) {
                m_dragOffset = pos - m_selectedRegion.topLeft();
                m_dragStartRegion = m_selectedRegion;   // ✅ 記錄原始框位置
                m_isDraggingSelection = true;
            }
            return;
        }
    }

#endif

#ifdef SelectCopy_

    if (m_currentTool == Tool_Select) {
        QPoint pos = convertToOLED(event->pos());

        // 👉 右鍵拖曳選取框
        if (event->button() == Qt::RightButton) {
            if (m_selectedRegion.contains(pos)) {
                m_dragOffset = pos - m_selectedRegion.topLeft();
                m_dragStartRegion = m_selectedRegion;   // ✅ 記錄原始框位置
                m_isDraggingSelection = true;
            }
            return;
        }

        // 👉 左鍵開始選取
        if (event->button() == Qt::LeftButton) {
            m_startPoint = pos;
            m_endPoint = pos;
            m_isSelecting = true;
            update();
            return;
        }
    }

#endif

    // 不論左右鍵，m_startPoint 都是第一次點擊的位置
    m_startPoint = QPoint(oled_x, oled_y);
    m_endPoint   = m_startPoint; // 初始時終點與起點相同
    m_isDrawing  = true; // 點擊時就認為開始繪圖 (拖曳可能會發生)


    // 左鍵畫圖
    if (event->button() == Qt::LeftButton) {
        if (m_currentTool == Tool_Pen) {
            setPixel(oled_x, oled_y, true);
            updateImageFromBuffer();
        }
    }
    // 對於其他工具，只需設定 m_isDrawing = true 和 m_startPoint，
    // 實際的預覽會在 mouseMoveEvent -> update() -> paintEvent 裡處理
    // 實際繪製會在 mouseReleaseEvent 裡處理
    else if (event->button() == Qt::RightButton){
        if(m_currentTool == Tool_Pen){
            setPixel(oled_x, oled_y, false);
            updateImageFromBuffer(); // ✅ 右鍵清除也即時更新
        }else{
            m_isDrawing = false;
            m_startPoint = QPoint(-1, -1);
            m_endPoint = QPoint(-1, -1);
            update(); // 清掉預覽線
        }

    }
    update();

}

void OLEDWidget::mouseReleaseEvent(QMouseEvent *event) {

#ifdef SelectCopy
    if (m_currentTool == Tool_Select && event->button() == Qt::LeftButton) {
        handleSelectRelease(event);
        return;
    }
#endif



    // 如果不是在繪圖狀態，就直接返回
    if (!m_isDrawing) {
        return;
    }


    if (event->button() == Qt::LeftButton) {
        int x0 = m_startPoint.x();
        int y0 = m_startPoint.y();
        int x1 = m_endPoint.x();
        int y1 = m_endPoint.y();

        // 除了 Pen，其他圖形在滑鼠釋放時才真正繪製到 buffer
        switch (m_currentTool) {
        case Tool_Line:
            drawLine(x0, y0, x1, y1, true, m_buffer);
            break;
        case Tool_Rectangle:
            drawRectangle(x0, y0, x1 - x0, y1 - y0, true, false, m_buffer);
            break;
        case Tool_FilledRectangle:
            drawRectangle(x0, y0, x1 - x0, y1 - y0, true, true, m_buffer);
            break;
        case Tool_Circle:
        {
            drawCircle(m_startPoint, m_endPoint, m_buffer);
            break;
        }
        default: // 包括 Pen
            break;
        }

        // 【重要】在所有繪圖演算法執行完畢後，
        // 只有在非畫筆工具時，才需要在這裡做一次最終的畫面更新。
        if (m_currentTool != Tool_Pen) {
            updateImageFromBuffer();
        }

    }
    m_isDrawing = false;
    //選取貼上

    //選取貼上
    // 最後再呼叫一次 update() 來清除預覽圖形 (藍色線)
    // 因為此時 m_isDrawing 已經是 false，paintEvent 中的預覽繪圖邏輯不會再執行
    update();
}

//mouse 三兄弟


void OLEDWidget::leaveEvent(QEvent *event)
{
    // 當滑鼠離開 widget 時，發送一個無效座標 (-1, -1)
    emit coordinatesChanged(QPoint(-1, -1));
    QWidget::leaveEvent(event);
}

// ↓↓↓↓ 把這個完整的函式實作，複製貼上到你的 oledwidget.cpp 檔案中 ↓↓↓↓
void OLEDWidget::updateImageFromBuffer()
{
    // 1. 確保 img 物件是正確的大小和格式
    img = QImage(OledConfig::DISPLAY_WIDTH, OledConfig::DISPLAY_HEIGHT, QImage::Format_RGB888);

    // 2. 定義像素的亮/暗顏色
    const QColor pixelOnColor = QColor(135, 206, 250); // 淺藍色
    const QColor pixelOffColor = Qt::black;

    // 3. 遍歷內部緩衝區 m_buffer，將數據轉換為 QImage 的像素
    for (int page = 0; page < OledConfig::DISPLAY_HEIGHT/8; page++) {
        for (int x = 0; x < OledConfig::DISPLAY_WIDTH; x++) {
            // 注意：是從 m_buffer 讀取，這是我們自己的數據儲存區
            int byte_index = page * OledConfig::RAM_PAGE_WIDTH + (x + OledConfig::COLUMN_OFFSET);

            if (byte_index >= 0 && byte_index < OledConfig::RAM_PAGE_WIDTH * (OledConfig::DISPLAY_HEIGHT / 8)) {
                uint8_t data = m_buffer[byte_index];

                for (int bit = 0; bit < 8; bit++) {
                    bool on = data & (1 << bit);
                    int y = page * 8 + bit;
                    img.setPixelColor(x, y, on ? pixelOnColor : pixelOffColor);
                }
            }
        }
    }

    // 4. 更新 widget 的最小尺寸並觸發重繪
    setMinimumSize(img.width() * scale, img.height() * scale);
    update(); // 觸發 paintEvent
}

// ↓↓↓↓ 請將這個完整的函式實作，加入到 oledwidget.cpp 檔案中 ↓↓↓↓
const uint8_t* OLEDWidget::getBuffer() const
{
    // 這個函式的唯一任務，就是返回內部 m_buffer 陣列的地址
    return m_buffer;
}



/*選取複製*/

#ifdef SelectCopy
// === Tool_Select ===
void OLEDWidget::handleSelectPress(QMouseEvent *event)
{
    QPoint pos = convertToOLED(event->pos());

    // 只接受左鍵
    if (event->button() != Qt::LeftButton)
        return;

    // 開始新的選取
    m_isSelecting = true;

    m_startPoint = pos;
    m_endPoint = pos;

    m_selectedRegion = QRect(); // 清除舊框
    update();
}

void OLEDWidget::handleSelectMove(QMouseEvent *event)
{
    if (!m_isSelecting)
        return;

    QPoint pos = convertToOLED(event->pos());
    m_endPoint = pos;
    update(); // 繪製虛線框
}

void OLEDWidget::handleSelectRelease(QMouseEvent *event)
{
    if (!m_isSelecting || event->button() != Qt::LeftButton)
        return;

    QPoint pos = convertToOLED(event->pos());

    m_endPoint = pos;
    m_isSelecting = false;

    // 正規化框（確保左上角小於右下角）
    QRect region = QRect(m_startPoint, m_endPoint).normalized();

    // 避免太小的誤觸框
    if (region.width() < 2 && region.height() < 2){
         qDebug() << "[handleSelectRelease] 選取框太小，清除";
        region = QRect();
    }else {
        qDebug() << "[handleSelectRelease] 選取框設定為:" << region;
    }

    m_selectedRegion = region;

    update();
}


#endif

QPoint OLEDWidget::convertToOLED(const QPoint &pos)
{
    int x = pos.x() / scale;
    int y = pos.y() / scale;

    // 限制在 OLED 顯示範圍內
    x = std::clamp(x, 0, OledConfig::DISPLAY_WIDTH - 1);
    y = std::clamp(y, 0, OledConfig::DISPLAY_HEIGHT - 1);

    return QPoint(x, y);
}



#ifdef Past_Function

void OLEDWidget::startPastePreview(const QVector<uint8_t>& data, int width, int height)
/*oledwidget.cpp:759:60: Passing argument to parameter 'data' here
oledwidget.cpp:856:23: Passing argument to parameter 'data' here*/
{
    m_pastePreviewActive = true;
    m_pasteWidth = width;
    m_pasteHeight = height;
    m_pastePosition = QPoint(0, 0);

    // [关键修改]
    // 直接将传入的 QVector 赋值给成员变量 m_pasteData
    // QVector 会自动处理内存的复制和管理，非常安全。
    m_pasteData = data;


    //  m_pasteData = QVector<uint8_t>(data, data + width * ((height + 7) / 8));

    update();
}

void OLEDWidget::keyPressEvent(QKeyEvent *event)
{
    if (m_pastePreviewActive && event->key() == Qt::Key_Return) {
        confirmPasteDialog();
    }
}

void OLEDWidget::confirmPasteDialog()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("確認貼上");

    QLabel *label = new QLabel("是否確定要貼上這塊資料？", dialog);
    QPushButton *yesButton = new QPushButton("確定", dialog);
    QPushButton *noButton = new QPushButton("取消", dialog);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(label);
    layout->addWidget(yesButton);
    layout->addWidget(noButton);

    connect(yesButton, &QPushButton::clicked, [this, dialog]() {
        commitPaste();
        dialog->accept();
    });
    connect(noButton, &QPushButton::clicked, dialog, &QDialog::reject);

    dialog->exec();
}

void OLEDWidget::commitPaste()
{
    int x0 = m_pastePosition.x();
    int y0 = m_pastePosition.y();
    int pages = (m_pasteHeight + 7) / 8;
    int index = 0;

    for (int x = 0; x < m_pasteWidth; ++x) {
        for (int page = 0; page < pages; ++page) {
            uint8_t byte = m_pasteData[index++];
            for (int bit = 0; bit < 8; ++bit) {
                int y = y0 + page * 8 + bit;
                if (y >= OledConfig::DISPLAY_HEIGHT) continue;
                bool pixelOn = byte & (1 << bit);
                setPixel(x0 + x, y, pixelOn, m_buffer);
            }
        }
    }
    qDebug() << "[貼上狀態] active:" << m_pastePreviewActive
             << " size:" << m_pasteData.size()
             << " pos:" << m_pastePosition
             << " w:" << m_pasteWidth << " h:" << m_pasteHeight;

    m_pastePreviewActive = false;
    updateImageFromBuffer();
    update();
}

/*
void OLEDWidget::pasteBlock(const QRect &region)
{
    int x0 = region.left();
    int y0 = region.top();
    int w = region.width();
    int h = region.height();
    int pages = (h + 7) / 8;

    QVector<uint8_t> blockData;
    for (int x = x0; x < x0 + w; ++x) {
        for (int page = 0; page < pages; ++page) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                int y = y0 + page * 8 + bit;
                if (y >= y0 + h) break;
                if (getPixel(x, y)) byte |= (1 << bit);
            }
            blockData.append(byte);
        }
    }

    startPastePreview(blockData.data(), w, h);
    commitPaste();
}
*/

#endif


