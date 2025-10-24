
#include "oledwidget.h"
#include <QPainter>
#include <QMouseEvent> // <--- 把這一行加進來！
#include <cstring>
#include <QScrollBar>
#include <algorithm>
#include <cmath> // For circle drawing

void OLEDWidget::setScale(int s) {
    const int minScale = 1;
    const int maxScale = 20; // 依需求調整最大放大倍數
    scale = std::clamp(s, minScale, maxScale);
    //scale = s > 0 ? s : 1;

    setFixedSize(img.width() * scale, img.height() * scale);
#ifdef V25_10_20
    setMinimumSize(img.width()*scale, img.height()*scale);
#endif
    update();
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
OLEDWidget::OLEDWidget(QWidget *parent)
    : QWidget(parent),
    m_currentTool(Tool_Pen), // 預設為畫筆
    m_isDrawing(false)
{
    // 初始為空白 128x64
    img = QImage(128, 64, QImage::Format_RGB888);
    img.fill(Qt::black);
    setMinimumSize(img.width()*scale, img.height()*scale);
    memset(m_buffer, 0, sizeof(m_buffer)); // 初始化 buffer
}

// ================== 新增的 SLOT ==================
void OLEDWidget::setCurrentTool(ToolType tool) {
    m_currentTool = tool;
}


void OLEDWidget::loadBitmap(const uint8_t *data, int w, int h) {
    if (!data || w <= 0 || h <= 0) return;

    img = QImage(w, h, QImage::Format_Mono);
    img.fill(0);

    int byteWidth = (w + 7) / 8;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int byteIndex = y * byteWidth + (x / 8);
            uint8_t byte = data[byteIndex];
            uint8_t mask = 0x80 >> (x % 8); // MSB first
            bool pixelOn = (byte & mask);
            img.setPixel(x, y, pixelOn ? 1 : 0);
        }
    }

    setMinimumSize(img.width()*scale, img.height()*scale);
    update();
}

void OLEDWidget::paintEvent(QPaintEvent *) {

    QPainter p(this);
    p.fillRect(rect(), Qt::darkGray); // 背景色（方便看）
    if (img.isNull()) return;

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
}

// Bresenham's line algorithm
void OLEDWidget::drawLine(int x0, int y0, int x1, int y1, bool on,uint8_t* buffer) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        setPixel(x0, y0, on,buffer);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void OLEDWidget::drawRectangle(int x, int y, int w, int h, bool on, bool fill,uint8_t* buffer) {
    int x0 = std::min(x, x + w);
    int y0 = std::min(y, y + h);
    int x1 = std::max(x, x + w);
    int y1 = std::max(y, y + h);

    if (fill) {
        for (int i = y0; i <= y1; ++i) {
            drawLine(x0, i, x1, i, on,buffer);
        }
    } else {
        drawLine(x0, y0, x1, y0, on,buffer);
        drawLine(x0, y1, x1, y1, on,buffer);
        drawLine(x0, y0, x0, y1, on,buffer);
        drawLine(x1, y0, x1, y1, on,buffer);
    }
}


// ↓↓↓↓ 在 oledwidget.cpp 中，用這個【更穩定】的版本替換掉你舊的 drawCircle 函式 ↓↓↓↓

void OLEDWidget::drawCircle(const QPoint &p1, const QPoint &p2, uint8_t* buffer)
{
    int x0 = std::min(p1.x(), p2.x());
    int y0 = std::min(p1.y(), p2.y());
    int x1 = std::max(p1.x(), p2.x());
    int y1 = std::max(p1.y(), p2.y());

    // 如果寬度或高度為0，則不繪製，避免演算法出錯
    if (x0 == x1 || y0 == y1) {
        return;
    }

    long xc = (x0 + x1) / 2;
    long yc = (y0 + y1) / 2;
    long a = (x1 - x0) / 2;
    long b = (y1 - y0) / 2;

    long a2 = a * a;
    long b2 = b * b;
    long two_a2 = 2 * a2;
    long two_b2 = 2 * b2;

    // Region 1
    long x = 0;
    long y = b;
    long p = b2 - a2 * b + (a2 / 4);

    while (two_b2 * x < two_a2 * y) {
        setPixel(xc + x, yc + y, true, buffer);
        setPixel(xc - x, yc + y, true, buffer);
        setPixel(xc + x, yc - y, true, buffer);
        setPixel(xc - x, yc - y, true, buffer);

        x++;
        if (p < 0) {
            p += two_b2 * x + b2;
        } else {
            y--;
            p += two_b2 * x + b2 - two_a2 * y;
        }
    }

    // Region 2
    p = b2 * (x * x + x) + a2 * (y * y - y) - a2 * b2;
    while (y >= 0) {
        setPixel(xc + x, yc + y, true, buffer);
        setPixel(xc - x, yc + y, true, buffer);
        setPixel(xc + x, yc - y, true, buffer);
        setPixel(xc - x, yc - y, true, buffer);

        y--;
        if (p > 0) {
            p -= two_a2 * y + a2;
        } else {
            x++;
            p += two_b2 * x - two_a2 * y + a2;
        }
    }
}



void OLEDWidget::setBuffer(const uint8_t *buffer){
    // 同步内部状态
    if (buffer) {
        memcpy(m_buffer, buffer, sizeof(m_buffer));
    } else {
        memset(m_buffer, 0, sizeof(m_buffer));
    }
    // 调用统一的更新函数来显示
    updateImageFromBuffer();
}


// ↓↓↓↓ 檢查並補上 clearScreen 函式 ↓↓↓↓
void OLEDWidget::clearScreen() {
    // 將內部緩衝區全部填 0
    memset(m_buffer, 0, sizeof(m_buffer));
    updateImageFromBuffer(); // 更新顯示
}


void OLEDWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!m_isDrawing) return;

    int oled_x = (event->pos().x() - (width() - img.width() * scale) / 2) / scale;
    int oled_y = (event->pos().y() - (height() - img.height() * scale) / 2) / scale;

    // 檢查座標是否在畫布內，避免預覽圖形畫到外面去
    oled_x = std::clamp(oled_x, 0, img.width() - 1);
    oled_y = std::clamp(oled_y, 0, img.height() - 1);


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

    // 最後再呼叫一次 update() 來清除預覽圖形 (藍色線)
    // 因為此時 m_isDrawing 已經是 false，paintEvent 中的預覽繪圖邏輯不會再執行
    update();

}

// ↓↓↓↓ 把這個完整的函式實作，複製貼上到你的 oledwidget.cpp 檔案中 ↓↓↓↓
void OLEDWidget::updateImageFromBuffer()
{
    // 1. 確保 img 物件是正確的大小和格式
    img = QImage(128, 64, QImage::Format_RGB888);

    // 2. 定義像素的亮/暗顏色
    const QColor pixelOnColor = QColor(135, 206, 250); // 淺藍色
    const QColor pixelOffColor = Qt::black;

    // 3. 遍歷內部緩衝區 m_buffer，將數據轉換為 QImage 的像素
    for (int page = 0; page < 8; page++) {
        for (int x = 0; x < 128; x++) {
            // 注意：是從 m_buffer 讀取，這是我們自己的數據儲存區
            uint8_t data = m_buffer[page * 128 + x];
            for (int bit = 0; bit < 8; bit++) {
                bool on = data & (1 << bit);
                int y = page * 8 + bit;
                img.setPixelColor(x, y, on ? pixelOnColor : pixelOffColor);
            }
        }
    }

    // 4. 更新 widget 的最小尺寸並觸發重繪
    setMinimumSize(img.width() * scale, img.height() * scale);
    update(); // 觸發 paintEvent
}

void OLEDWidget::setPixel(int x, int y, bool on, uint8_t* buffer)
{
    int offset = m_brushSize / 2;

    for (int dx = 0; dx < m_brushSize; ++dx) {
        for (int dy = 0; dy < m_brushSize; ++dy) {
            int px = x + dx - offset;
            int py = y + dy - offset;

            if (px >= 0 && px < 128 && py >= 0 && py < 64) {
                int page = py / 8;
                int bit_index = py % 8;
                int byte_index = page * 128 + px;

                if (on)
                    buffer[byte_index] |= (1 << bit_index);
                else
                    buffer[byte_index] &= ~(1 << bit_index);
            }
        }
    }
}




/*
// 【实作】"高效版"，私有
// 它只修改 buffer，不做其他任何事。
void OLEDWidget::setPixel(int x, int y, bool on,uint8_t* buffer)
{
    // 1. 邊界檢查，防止寫入超出範圍的數據
    if (x < 0 || x >= 128 || y < 0 || y >= 64) {
        return; // 超出範圍，直接返回，不做任何事
    }

    // 2. 計算像素在 m_buffer 緩衝區中的準確位置
    //    SH1106 是垂直位元組模式
    int page = y / 8;                  // 計算在哪一頁 (第幾行，每行8個像素高)
    int bit_index = y % 8;             // 計算在該位元組的第幾個位元 (0-7)
    int byte_index = page * 128 + x;   // 計算在1024位元組陣列中的索引

    // 3. 使用位元運算來修改該位元，而不影響同一個位元組裡的其他7個像素
    if (on) {
        // 如果是要「點亮」像素 (on = true)
        // 使用「位元或 OR」操作，將該位元設為 1
        m_buffer[byte_index] |= (1 << bit_index);
    } else {
        // 如果是要「熄滅」像素 (on = false)
        // 使用「位元與 AND」和「位元反向 NOT」操作，將該位元設為 0
        m_buffer[byte_index] &= ~(1 << bit_index);
    }
}*/

// 【修改】"方便版"，公开
// 它的实现是呼叫"高效版"，然后更新画面
void OLEDWidget::setPixel(int x, int y, bool on)
{
    // 步骤1：呼叫高效版来修改主缓冲区 m_buffer
    //setPixel(x, y, on, m_buffer);

    for (int dx = 0; dx < m_brushSize; ++dx) {
        for (int dy = 0; dy < m_brushSize; ++dy) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < 128 && py >= 0 && py < 64) {
                setPixel(px, py, on, m_buffer);
            }
        }
    }


    // 步骤2：更新整个画面
    updateImageFromBuffer();
}

// ↓↓↓↓ 請將這個完整的函式實作，加入到 oledwidget.cpp 檔案中 ↓↓↓↓
const uint8_t* OLEDWidget::getBuffer() const
{
    // 這個函式的唯一任務，就是返回內部 m_buffer 陣列的地址
    return m_buffer;
}

void OLEDWidget::setBrushSize(int size)
{
    // 安全限制：筆刷大小只能是 1～6
    if (size < 1) size = 1;
    if (size > 6) size = 6;

    m_brushSize = size;
}
