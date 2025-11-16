#include <algorithm>     // 需要 std::min
#include "oled_dataconverter.h"


void OledDataConverter::updateModelFromImage(OledDataModel* model, const QImage& image)
{
    // --- 步驟 1: 安全性檢查 ---
    if (!model) {
        // 如果傳入的 model 指標是空的，直接返回，防止程式崩潰
        return;
    }
    if (image.isNull() || image.format() != QImage::Format_Mono) {
        // 如果圖片無效或格式不對，也直接返回
        return;
    }

    // --- 步驟 2: 清空模型 ---
    // 這是「載入」而不是「貼上」，所以先清空整個畫布
    model->clear();

    // --- 步驟 3: 遍歷 QImage 並更新模型 ---
    // 計算需要遍歷的範圍，避免超出 OLED 的邊界
    int width_to_copy = std::min(image.width(), OledConfig::DISPLAY_WIDTH);
    int height_to_copy = std::min(image.height(), OledConfig::DISPLAY_HEIGHT);

    for (int y = 0; y < height_to_copy; ++y) {
        for (int x = 0; x < width_to_copy; ++x) {
            // 對於 QImage::Format_Mono:
            // pixelIndex(x, y) 返回 0 代表前景 (通常是黑色)
            // pixelIndex(x, y) 返回 1 代表背景 (通常是白色)
            // 我們假設亮點是黑色。如果您的圖片是白底黑字，這個邏輯就是對的。
            if (image.pixelIndex(x, y) == 0) {
                // 直接呼叫 model 提供的基礎繪圖函式
                model->setPixel(x, y, true,1);
            }
        }
    }
}
