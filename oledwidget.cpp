#include "oledwidget.h"
#include <QPainter>
#include <QMouseEvent> // <--- 把這一行加進來！
#include <cstring>

#include <algorithm>

OLEDWidget::OLEDWidget(QWidget *parent) : QWidget(parent) {
    // 初始為空白 128x64
    img = QImage(128, 64, QImage::Format_Mono);
    img.fill(0);
    setMinimumSize(img.width()*scale, img.height()*scale);
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


void OLEDWidget::setBuffer(const uint8_t *buffer)
{
    img = QImage(128, 64, QImage::Format_RGB888);
    //img.fill(Qt::black);

    // 2. 將像素顏色改為亮色（例如淺藍色或白色）和黑色
    const QColor pixelOnColor = QColor(135, 206, 250); // 淺藍色，模擬OLED

    const QColor pixelOffColor = Qt::black;

    // SH1106 格式是「每列 128 bytes，每 byte 是垂直 8 pixels」
    for (int page = 0; page < 8; page++) { // 8 pages * 8 pixels = 64 高度
        for (int x = 0; x < 128; x++) {
            uint8_t data = buffer[page * 128 + x];
            for (int bit = 0; bit < 8; bit++) {
                bool on = data & (1 << bit);
                int y = page * 8 + bit;
                img.setPixelColor(x, y, on ? pixelOnColor : pixelOffColor);
            }
        }
    }
    setMinimumSize(img.width() * scale, img.height() * scale);
    update(); // 重新繪圖
}


// ↓↓↓↓ 檢查並補上 clearScreen 函式 ↓↓↓↓
void OLEDWidget::clearScreen() {
    // 將內部緩衝區全部填 0
    memset(m_buffer, 0, sizeof(m_buffer));
    updateImageFromBuffer(); // 更新顯示
    //oledwidget.cpp:95: error: undefined reference to `OLEDWidget::updateImageFromBuffer()'
}

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
