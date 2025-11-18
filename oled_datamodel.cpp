
    #include "oled_datamodel.h"
    #include "oledwidget_Paint.h"

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
    /**
     * @brief 修改邏輯緩衝區中一個或多個像素的狀態。
     *
     * 此函數是資料模型最主要的繪圖操作介面。它不僅能設定單一像素點的開關，
     * 還支援基於筆刷大小（brushSize）的方形區域繪製。
     *
     * - 當筆刷大小為 1 (預設值) 時，它只會修改 (x, y) 座標上的單一像素。
     * - 當筆刷大小大於 1 時，它會以 (x, y) 為中心，繪製一個 `brushSize` x `brushSize` 的方形區域。
     *
     * 所有操作都會進行邊界檢查，以確保不會寫入到緩衝區範圍之外。
     *
     * @param[in] x         目標像素的 X 座標，或筆刷的中心 X 座標。
     * @param[in] y         目標像素的 Y 座標，或筆刷的中心 Y 座標。
     * @param[in] on        像素的目標狀態：`true` 為點亮，`false` 為熄滅（擦除）。
     * @param[in] brushSize 方形筆刷的邊長。預設為 1，表示單點繪製。
     */
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

    /**
     * @brief 取得邏輯緩衝區中指定像素的狀態。
     *
     * 這是一個唯讀（const）的存取函數，用於查詢特定座標 (x, y) 上的像素是點亮 (`true`) 還是熄滅 (`false`)。
     * 函數內部包含邊界檢查，如果查詢的座標超出了顯示範圍，將會安全地回傳 `false`。
     *
     * @param[in] x 要查詢的像素的 X 座標。
     * @param[in] y 要查詢的像素的 Y 座標。
     * @return bool 如果像素為點亮狀態，則回傳 `true`；如果為熄滅狀態或座標超出範圍，則回傳 `false`。
     * @see setPixel(int, int, bool, int)
     */

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
            setPixel(xc + x, yc + y, true,brushSize); setPixel(xc - x, yc + y, true,brushSize);
            setPixel(xc + x, yc - y, true,brushSize); setPixel(xc - x, yc - y, true,brushSize);
            x++;
            if (p < 0) { p += two_b2 * x + b2; }
            else { y--; p += two_b2 * x + b2 - two_a2 * y; }
        }
        p = b2 * (x * x + x) + a2 * (y * y - y) - a2 * b2;
        while (y >= 0) {
            setPixel(xc + x, yc + y, true,brushSize); setPixel(xc - x, yc + y, true,brushSize);
            setPixel(xc + x, yc - y, true,brushSize); setPixel(xc - x, yc - y, true,brushSize);
            y--;
            if (p > 0) { p -= two_a2 * y + a2; }
            else { x++; p += two_b2 * x - two_a2 * y + a2; }
        }
    }



    // --- 翻譯層：只在這裡處理硬體格式 ---

    // 翻譯官 1: 將內部邏輯 buffer 翻譯成硬體 buffer
    /**
     * @brief 取得符合硬體格式的顯示緩衝區。
     *
     * 此函數將內部儲存的邏輯像素緩衝區（二維概念）轉換為 OLED 硬體所需的特定一維頁面式（page-based）緩衝區格式。
     * 轉換過程涉及複雜的位元運算，將每個像素點對應到硬體緩衝區中特定位元組的特定位元。
     * OLED 的記憶體是分頁的，每頁 8 個像素高。此函數會將 (x, y) 座標轉換為 (page, column, bit) 的對應關係。
     *
     * @return std::vector<uint8_t> 一個包含可以直接寫入 OLED RAM 的原始資料的緩衝區。
     * @see setFromHardwareBuffer()
     */
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
                    //int byte_index = page * OledConfig::RAM_PAGE_WIDTH + x;

                    if (byte_index < hardware_buffer.size()) {
                        hardware_buffer[byte_index] |= (1 << bit_index);
                    }
                }
            }
        }
        return hardware_buffer;
    }


    // 翻譯官 2: 將外部硬體 buffer 翻譯並載入到內部邏輯 buffer
    /**
     * @brief 從硬體格式的緩衝區載入像素資料到內部邏輯緩衝區。
     *
     * 此函數執行與 getHardwareBuffer() 相反的操作。它接收一個指向原始硬體資料的指標，
     * 解析其分頁式（page-based）的結構，並將其轉換回內部使用的二維邏輯像素表示。
     * 這常用於從硬體讀取當前顯示內容時。
     *
     * @note 在載入新資料前，會先呼叫 clear() 清空當前的邏輯緩衝區。
     * @param[in] data 指向符合 OLED 硬體格式的原始資料緩衝區的指標。如果為 nullptr，則函數不做任何事。
     * @see getHardwareBuffer()
     */
    void OledDataModel::setFromHardwareBuffer(const uint8_t* data)
    {
        clear(); // 先清空
        if (!data) return;

        for (int page = 0; page < (OledConfig::DISPLAY_HEIGHT / 8); ++page) {
            for (int x = 0; x < OledConfig::DISPLAY_WIDTH; ++x) {

                int byte_index = page * OledConfig::RAM_PAGE_WIDTH + (x + OledConfig::COLUMN_OFFSET);
                //int byte_index = page * OledConfig::RAM_PAGE_WIDTH + x;

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

    /**
     * @brief 複製內部邏輯緩衝區的指定區域並回傳為一個 QImage。
     *
     * 此函數用於截取目前 OLED 資料模型中的一個矩形區域，並將其轉換為一個獨立的、
     * 使用邏輯格式（1-bit monochrome）的 QImage 物件。
     * 這對於實現複製/貼上功能或預覽局部區域非常有用。
     *
     * @param[in] region 要複製的來源區域，使用邏輯座標 (0,0 到 DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1)。
     * @return QImage 一個包含指定區域像素資料的 QImage。如果指定區域無效或在邊界外，則回傳一個空的 QImage。
     * @see convertLogicalToHardwareFormat()
     */
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
    /**
     * @brief [靜態] 將邏輯格式的 QImage 轉換為硬體緩衝區格式。
     *
     * 這是一個靜態工具函數，功能類似於 getHardwareBuffer()，但它操作的是一個外部傳入的 QImage 物件，
     * 而不是類別實例的內部緩衝區。它會處理欄位偏移（COLUMN_OFFSET）和頁尾填充，
     * 產生一個完整的、符合硬體規範的資料緩衝區。
     *
     * @param[in] logicalImage 要轉換的來源圖片，必須是 QImage::Format_Mono 格式。
     * @return QVector<uint8_t> 一個包含轉換後的硬體格式資料的 QVector。如果輸入圖片格式不正確，則回傳一個空的 QVector。
     * @note 請注意，此實現將 QImage 中像素索引為 0 的點（通常是白色）設置為硬體緩衝區中的亮點（對應位元為 1）。
     *       如果您的 QImage 使用相反的約定（例如，黑色為亮點），請在使用前進行相應的影像處理（如 invertPixels()）。
     * @see getHardwareBuffer()
     * @see copyRegionToLogicalFormat()
     */
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
            for (int x = 0; x < OledConfig::COLUMN_OFFSET; ++x) {
                hardwareData.append(0x00);
            }
            for (int x = 0; x < w; ++x) {
                uint8_t byte = 0;
                for (int bit = 0; bit < 8; ++bit) {
                    int currentY = page * 8 + bit;
                    if (currentY < h) {
                        if (logicalImage.pixelIndex(x, currentY) == 0) {
                            byte |= (1 << bit);
                        }
                    }
                }
                hardwareData.append(byte);
            }
            int filled = OledConfig::COLUMN_OFFSET + w;
            for (int x = filled; x < OledConfig::RAM_PAGE_WIDTH; ++x) {
                hardwareData.append(0x00);
            }
        }
        return hardwareData;
    }
