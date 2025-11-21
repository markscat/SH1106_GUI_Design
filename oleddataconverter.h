#ifndef OLEDDATACONVERTER_H
#define OLEDDATACONVERTER_H

#pragma once


#include "oleddatamodel.h" // 在 .cpp 中包含完整的定義
#include "config.h"      // 需要 OledConfig


// Forward declaration to avoid including the full header

class OledDataModel;

/**
 * @brief OLED 資料轉換與處理的工具類別。
 *
 * 這個類別提供了一系列的靜態函式，用於在不同的資料格式
 * (如 QImage、硬體緩衝區) 和 OledDataModel 之間進行轉換。
 * 所有的函式都是無狀態的 (stateless)，不儲存任何成員變數。
 */
class OledDataConverter
{
public:
    /**
     * @brief 將一個邏輯格式的 QImage 內容載入到 OledDataModel 中。
     *
     * 此函式會先清空目標 model，然後遍歷傳入的 QImage，
     * 將其像素資料轉換並寫入到 model 的內部邏輯緩衝區中。
     *
     * @param model 要被更新的 OledDataModel 物件的指標。不可為 nullptr。
     * @param image 來源圖片，必須是 QImage::Format_Mono 格式。
     *              圖片中像素索引為 0 (通常是黑色) 的點會被視為亮點。
     */
    static void updateModelFromImage(OledDataModel* model, const QImage& image);
};

#endif // OLEDDATACONVERTER_H
