
#include "oleddatamodel.h"
#include <algorithm>
#include <QPoint>
#include <cmath>
#include <cstring>   // for memset, memcpy


// Include QPoint here because we need its implementation for drawCircle
#include <QPoint>

OledDataModel::OledDataModel()
    // 初始化邏輯 buffer 為 128*64 的大小，所有值預設為 false (黑)
    : m_logical_buffer(OledConfig::DISPLAY_WIDTH * OledConfig::DISPLAY_HEIGHT, false)
{
}


void OledDataModel::clear()
{
    std::fill(m_logical_buffer.begin(), m_logical_buffer.end(), false);
}

// 核心操作變得極其簡單！
void OledDataModel::setPixel(int x, int y, bool on,int brushSize = 1)
{
    if (brushSize <= 1) {
        // 单点绘制
        // 如果笔刷大小大于 1，就画一个方块
        if (x >= 0 && x < OledConfig::DISPLAY_WIDTH && y >= 0 && y < OledConfig::DISPLAY_HEIGHT){
            m_logical_buffer[y * OledConfig::DISPLAY_WIDTH + x] = on;
        }


    }else {
        // 计算偏移量，使得笔刷以 (x, y) 为中心
        // 例如，3x3 的笔刷，offset 是 1。循环 dx 从 0 到 2。
        // x + dx - offset 的范围就是 x-1, x, x+1。
        int offset = (brushSize - 1) / 2;

        // 遍历笔刷覆盖的每一个点
        for (int dy = 0; dy < brushSize; ++dy) {
            for (int dx = 0; dx < brushSize; ++dx) {
                int px = x + dx - offset;
                int py = y + dy - offset;

                // 对每一个点都进行边界检查
                if (px >= 0 && px < OledConfig::DISPLAY_WIDTH && py >= 0 && py < OledConfig::DISPLAY_HEIGHT) {
                    m_logical_buffer[py * OledConfig::DISPLAY_WIDTH + px] = on;
                }
            }
        }
    }

}

bool OledDataModel::getPixel(int x, int y) const
{
    if (x >= 0 && x < OledConfig::DISPLAY_WIDTH && y >= 0 && y < OledConfig::DISPLAY_HEIGHT) {
        return m_logical_buffer[y * OledConfig::DISPLAY_WIDTH + x];
    }
    return false;
}


// --- 繪圖演算法 (現在它們都直接操作邏輯 buffer) ---

void OledDataModel::drawLine(int x0, int y0, int x1, int y1, bool on,int brushSize)
{
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        setPixel(x0, y0, on,brushSize); // 直接呼叫簡單的 setPixel
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void OledDataModel::drawRectangle(int x, int y, int w, int h, bool on, bool fill,int brushSize)
{
    int x0 = std::min(x, x + w);
    int y0 = std::min(y, y + h);
    int x1 = std::max(x, x + w);
    int y1 = std::max(y, y + h);

    if (fill) {
        for (int i = y0; i <= y1; ++i) {
            for (int j = x0; j <= x1; ++j) { // 實心矩形甚至可以更簡單
                setPixel(j, i, on, brushSize);
            }
        }
    } else {
        drawLine(x0, y0, x1, y0, on,brushSize);
        drawLine(x0, y1, x1, y1, on,brushSize);
        drawLine(x0, y0, x0, y1, on,brushSize);
        drawLine(x1, y0, x1, y1, on,brushSize);
    }
}

// drawCircle 保持不變，因為它也是基於 setPixel 的
void OledDataModel::drawCircle(const QPoint &p1, const QPoint &p2,int brushSize)
{
    int x0 = std::min(p1.x(), p2.x());
    int y0 = std::min(p1.y(), p2.y());
    int x1 = std::max(p1.x(), p2.x());
    int y1 = std::max(p1.y(), p2.y());
    if (x0 == x1 || y0 == y1) return;
    long xc = (x0 + x1) / 2, yc = (y0 + y1) / 2;
    long a = (x1 - x0) / 2, b = (y1 - y0) / 2;
    long a2 = a * a, b2 = b * b;
    long two_a2 = 2 * a2, two_b2 = 2 * b2;
    long x = 0, y = b;
    long p = b2 - a2 * b + (a2 / 4);
    while (two_b2 * x < two_a2 * y) {
        setPixel(xc + x, yc + y, true); setPixel(xc - x, yc + y, true);
        setPixel(xc + x, yc - y, true); setPixel(xc - x, yc - y, true);
        x++;
        if (p < 0) { p += two_b2 * x + b2; }
        else { y--; p += two_b2 * x + b2 - two_a2 * y; }
    }
    p = b2 * (x * x + x) + a2 * (y * y - y) - a2 * b2;
    while (y >= 0) {
        setPixel(xc + x, yc + y, true); setPixel(xc - x, yc + y, true);
        setPixel(xc + x, yc - y, true); setPixel(xc - x, yc - y, true);
        y--;
        if (p > 0) { p -= two_a2 * y + a2; }
        else { x++; p += two_b2 * x - two_a2 * y + a2; }
    }
}



// --- 翻譯層：只在這裡處理硬體格式 ---

// 翻譯官 1: 將內部邏輯 buffer 翻譯成硬體 buffer
std::vector<uint8_t> OledDataModel::getHardwareBuffer() const
{
    std::vector<uint8_t> hardware_buffer(OledConfig::RAM_PAGE_WIDTH * (OledConfig::DISPLAY_HEIGHT / 8), 0);

    for (int y = 0; y < OledConfig::DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < OledConfig::DISPLAY_WIDTH; ++x) {
            if (getPixel(x, y)) { // 讀取簡單的邏輯 buffer
                // 進行複雜的位元運算，寫入到硬體 buffer
                int page = y / 8;
                int bit_index = y % 8;
                int byte_index = page * OledConfig::RAM_PAGE_WIDTH + (x + OledConfig::COLUMN_OFFSET);
                if (byte_index < hardware_buffer.size()) {
                    hardware_buffer[byte_index] |= (1 << bit_index);
                }
            }
        }
    }
    return hardware_buffer;
}


// 翻譯官 2: 將外部硬體 buffer 翻譯並載入到內部邏輯 buffer
void OledDataModel::setFromHardwareBuffer(const uint8_t* data)
{
    clear(); // 先清空
    if (!data) return;

    for (int page = 0; page < (OledConfig::DISPLAY_HEIGHT / 8); ++page) {
        for (int x = 0; x < OledConfig::DISPLAY_WIDTH; ++x) {
            int byte_index = page * OledConfig::RAM_PAGE_WIDTH + (x + OledConfig::COLUMN_OFFSET);
            uint8_t byte = data[byte_index];
            for (int bit = 0; bit < 8; ++bit) {
                if ((byte >> bit) & 0x01) {
                    int y = page * 8 + bit;
                    setPixel(x, y, true); // 寫入簡單的邏輯 buffer
                }
            }
        }
    }
}

copiedLogicalData
QImage OledDataModel::copyRegionToLogicalFormat(const QRect& region) const
{
    // 确保 region 在有效范围内
    QRect validRegion = region.intersected(QRect(0, 0, OledConfig::DISPLAY_WIDTH, OledConfig::DISPLAY_HEIGHT));
    if (!validRegion.isValid()) {
        return QImage(); // 返回一个空的 QImage
    }

    QImage logicalCopy(validRegion.size(), QImage::Format_Mono);
    logicalCopy.fill(0);

    for (int y = 0; y < validRegion.height(); ++y) {
        for (int x = 0; x < validRegion.width(); ++x) {
            int sourceX = validRegion.left() + x;
            int sourceY = validRegion.top() + y;

            // 直接访问内部的 m_logical_buffer，效率更高
            if (getPixel(sourceX, sourceY)) {
                logicalCopy.setPixel(x, y, 1);
            }
        }
    }
    return logicalCopy;
}

// [新增] 实现 convertLogicalToHardwareFormat (作为 static 函数)
QVector<uint8_t> OledDataModel::convertLogicalToHardwareFormat(const QImage& logicalImage)
{
    // 确保传入的是单色图
    if (logicalImage.format() != QImage::Format_Mono) {
        // 如果不是，可以先转换或返回空
        return QVector<uint8_t>();
    }

    int w = logicalImage.width();
    int h = logicalImage.height();

    QVector<uint8_t> hardwareData;
    int pages = (h + 7) / 8;

    for (int page = 0; page < pages; ++page) {
        for (int x = 0; x < w; ++x) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                int currentY = page * 8 + bit;
                if (currentY < h) {
                    if (logicalImage.pixelIndex(x, currentY) == 1) {
                        byte |= (1 << bit);
                    }
                }
            }
            hardwareData.append(byte);
        }
    }
    return hardwareData;
}
