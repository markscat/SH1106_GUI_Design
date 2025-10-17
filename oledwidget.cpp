




#include "oledwidget.h"
#include <QPainter>
#include <QMouseEvent> // <--- 把這一行加進來！
#include <cstring>

#include <algorithm>

#include <cmath> // For circle drawing


//OLEDWidget::OLEDWidget(QWidget *parent) : QWidget(parent)



void OLEDWidget::setScale(int s) {
    scale = s > 0 ? s : 1;
    setMinimumSize(img.width()*scale, img.height()*scale);
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
    QRect oled_rect(x_offset, y_offset, scaled_width, scaled_height);

    p.drawImage(oled_rect, img);

    // 1. 绘制一个清晰的白色外边框
    p.setPen(QPen(Qt::white, 1));
    p.drawRect(oled_rect.adjusted(0, 0, -1, -1)); // adjusted 修正 1 像素偏差

    // 2. 绘制格线 (可选，但推荐)
    // 只有当放大倍数足够大时才绘制格线，避免糊成一团
    if (scale >= 4) {
        // 设置一个半透明的灰色画笔
        QPen grid_pen(QColor(128, 128, 128, 100), 1);
        p.setPen(grid_pen);

        // 画垂直线
        for (int i = 1; i < img.width(); ++i) {
            p.drawLine(x_offset + i * scale, y_offset, x_offset + i * scale, y_offset + scaled_height);
        }
        // 画水平线
        for (int j = 1; j < img.height(); ++j) {
            p.drawLine(x_offset, y_offset + j * scale, x_offset + scaled_width, y_offset + j * scale);
        }
    }

    if (m_isDrawing && m_currentTool != Tool_Pen) {
        p.setPen(Qt::blue);

        // 將 128x64 座標轉換回 Widget 座標
        int sx = x_offset + m_startPoint.x() * scale;
        int sy = y_offset + m_startPoint.y() * scale;
        int ex = x_offset + m_endPoint.x() * scale;
        int ey = y_offset + m_endPoint.y() * scale;

        switch (m_currentTool) {
        case Tool_Line:
            p.drawLine(sx, sy, ex, ey);
            break;
        case Tool_Rectangle:
        case Tool_FilledRectangle:
            p.drawRect(QRect(QPoint(sx, sy), QPoint(ex, ey)));
            break;
        case Tool_Circle:
        {
            int dx = ex - sx;
            int dy = ey - sy;
            int radius = static_cast<int>(std::sqrt(dx*dx + dy*dy));
            p.drawEllipse(QPoint(sx, sy), radius, radius);
        }
        break;
        default:
            break;
        }
    }


}


// Bresenham's line algorithm
void OLEDWidget::drawLine(int x0, int y0, int x1, int y1, bool on) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        setPixel(x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void OLEDWidget::drawRectangle(int x, int y, int w, int h, bool on, bool fill) {
    int x0 = std::min(x, x + w);
    int y0 = std::min(y, y + h);
    int x1 = std::max(x, x + w);
    int y1 = std::max(y, y + h);

    if (fill) {
        for (int i = y0; i <= y1; ++i) {
            drawLine(x0, i, x1, i, on);
        }
    } else {
        drawLine(x0, y0, x1, y0, on);
        drawLine(x0, y1, x1, y1, on);
        drawLine(x0, y0, x0, y1, on);
        drawLine(x1, y0, x1, y1, on);
    }
}

// Midpoint circle algorithm
void OLEDWidget::drawCircle(int centerX, int centerY, int radius, bool on) {
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y) {
        setPixel(centerX + x, centerY + y, on);
        setPixel(centerX + y, centerY + x, on);
        setPixel(centerX - y, centerY + x, on);
        setPixel(centerX - x, centerY + y, on);
        setPixel(centerX - x, centerY - y, on);
        setPixel(centerX - y, centerY - x, on);
        setPixel(centerX + y, centerY - x, on);
        setPixel(centerX + x, centerY - y, on);

        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}




    /*
    // 先轉為黑白 pixmap（Qt Mono 1 = 黑）
    QPixmap px = QPixmap::fromImage(
        img.scaled(
            img.width()*scale,
            img.height()*scale,
            Qt::IgnoreAspectRatio,
            Qt::FastTransformation ));
*/

    // 中心顯示
    /*
    int x = (width() - px.width())/2;
    int y = (height() - px.height())/2;
    p.drawPixmap(x, y, px);
*/
    // 可選：畫格線（取消註解可看像素邊界）
    /*
    p.setPen(QPen(Qt::darkGray, 1));
    for (int i = 0; i <= img.width(); ++i)
        p.drawLine(x + i*scale, y, x + i*scale, y + img.height()*scale);
    for (int j = 0; j <= img.height(); ++j)
        p.drawLine(x, y + j*scale, x + img.width()*scale, y + j*scale);
    */



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
    //oledwidget.cpp:95: error: undefined reference to `OLEDWidget::updateImageFromBuffer()'
}


void OLEDWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int oled_x = (event->pos().x() - (width() - img.width() * scale) / 2) / scale;
        int oled_y = (event->pos().y() - (height() - img.height() * scale) / 2) / scale;

        m_startPoint.setX(oled_x);
        m_startPoint.setY(oled_y);
        m_endPoint = m_startPoint;
        m_isDrawing = true;

        if (m_currentTool == Tool_Pen) {
            setPixel(oled_x, oled_y, true);
        }
    }
}

void OLEDWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!m_isDrawing) return;

    int oled_x = (event->pos().x() - (width() - img.width() * scale) / 2) / scale;
    int oled_y = (event->pos().y() - (height() - img.height() * scale) / 2) / scale;
    m_endPoint.setX(oled_x);
    m_endPoint.setY(oled_y);

    if (m_currentTool == Tool_Pen) {
        // 使用 Bresenham 演算法畫線，確保連續性
        drawLine(m_startPoint.x(), m_startPoint.y(), m_endPoint.x(), m_endPoint.y(), true);
        m_startPoint = m_endPoint; // 更新起點
    }

    update(); // 觸發 paintEvent 繪製預覽
}

void OLEDWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_isDrawing) {
        m_isDrawing = false;

        // 除了 Pen，其他圖形在滑鼠釋放時才真正繪製到 buffer
        switch (m_currentTool) {
        case Tool_Line:
            drawLine(m_startPoint.x(), m_startPoint.y(), m_endPoint.x(), m_endPoint.y(), true);
            break;
        case Tool_Rectangle:
            drawRectangle(m_startPoint.x(), m_startPoint.y(), m_endPoint.x() - m_startPoint.x(), m_endPoint.y() - m_startPoint.y(), true, false);
            break;
        case Tool_FilledRectangle:
            drawRectangle(m_startPoint.x(), m_startPoint.y(), m_endPoint.x() - m_startPoint.x(), m_endPoint.y() - m_startPoint.y(), true, true);
            break;
        case Tool_Circle:
        {
            int dx = m_endPoint.x() - m_startPoint.x();
            int dy = m_endPoint.y() - m_startPoint.y();
            int radius = static_cast<int>(std::sqrt(dx*dx + dy*dy));
            drawCircle(m_startPoint.x(), m_startPoint.y(), radius, true);
        }
        break;
        default: // 包括 Pen
            break;
        }

        updateImageFromBuffer(); // 完成繪圖後，從 buffer 更新 QImage
    }
}



/*

// ↓↓↓↓ 檢查並補上 mousePressEvent 函式 ↓↓↓↓
void OLEDWidget::mousePressEvent(QMouseEvent *event) {
    // 計算滑鼠點擊位置對應到 OLED 的 (x, y) 座標
    int oled_x = (event->pos().x() - (width() - img.width() * scale) / 2) / scale;
    int oled_y = (event->pos().y() - (height() - img.height() * scale) / 2) / scale;

    // 左鍵點擊 -> 畫點 (點亮)
    if (event->button() == Qt::LeftButton) {
        setPixel(oled_x, oled_y, true);
        //..\oledwidget.cpp:107: error: undefined reference to `OLEDWidget::setPixel(int, int, bool)'
    }
    // 右鍵點擊 -> 橡皮擦 (熄滅)
    else if (event->button() == Qt::RightButton) {
        setPixel(oled_x, oled_y, false);
        //同上
    }
}

// ↓↓↓↓ 檢查並補上 mouseMoveEvent 函式 ↓↓↓↓
void OLEDWidget::mouseMoveEvent(QMouseEvent *event) {
    // 檢查滑鼠按鍵狀態 (buttons() 返回的是狀態，不是單一事件)
    if (event->buttons() & Qt::LeftButton || event->buttons() & Qt::RightButton) {
        // 如果按住左鍵或右鍵移動，就呼叫 press 事件的處理邏輯
        // 這樣可以實現拖曳滑鼠來連續畫點或擦除
        mousePressEvent(event);
    }
}
*/


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


void OLEDWidget::setPixel(int x, int y, bool on)
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

    // 4. 數據修改完畢後，呼叫更新函式來讓畫面產生變化
    updateImageFromBuffer();
}

// ↓↓↓↓ 請將這個完整的函式實作，加入到 oledwidget.cpp 檔案中 ↓↓↓↓
const uint8_t* OLEDWidget::getBuffer() const
{
    // 這個函式的唯一任務，就是返回內部 m_buffer 陣列的地址
    return m_buffer;
}
