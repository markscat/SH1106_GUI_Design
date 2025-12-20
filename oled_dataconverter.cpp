#include <algorithm>     // 需要 std::min
#include "oled_dataconverter.h"

/**
 * @brief 將 QImage 的影像數據轉換並更新至 OledDataModel 中。
 *
 * 此函式會將傳入的 QImage 內容同步到 OLED 模型。這是一個「覆蓋式」的操作，
 * 函式執行開始時會先清空模型，隨後根據圖片的像素資訊進行填色。
 *
 * @param model 指向目標 OledDataModel 的指標。若為 nullptr 則不執行任何操作。
 * @param image 來源圖片。必須滿足以下條件：
 *              1. 圖片不為空 (isNull() == false)
 *              2. 格式必須為 QImage::Format_Mono (單色位圖)
 *
 * @note 轉換邏輯說明：
 * - 邊界處理：若圖片尺寸超過 OLED 顯示範圍 (OledConfig::DISPLAY_WIDTH/HEIGHT)，將進行裁切。
 * - 像素判定：針對 Format_Mono，此處將 pixelIndex 為 0 的點判定為「開啟 (True)」，
 *   通常對應於單色圖中的黑色部分（取決於調色盤設定）。
 *
 * @par 實作細節：
 * 1. 進行安全性檢查 (Null check)。
 * 2. 呼叫 model->clear() 重置畫布。
 * 3. 透過雙層迴圈遍歷有效區域內的像素。
 * 4. 呼叫 model->setPixel 更新數據。
 */
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
