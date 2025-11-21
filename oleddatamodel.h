#ifndef OLEDDATAMODEL_H
#define OLEDDATAMODEL_H


#include <cstdint> // for uint8_t
#include <vector> // 使用 std::vector<bool> 更安全、更靈活
#include "config.h"



// 包含 DISPLAY_WIDTH, RAM_PAGE_WIDTH 等常數

// Forward declaration for QPoint, avoids including full Qt header here
class QPoint;

class OledDataModel
{
public:
    OledDataModel();

    // --- 核心 Buffer 操作 ---
    void setPixel(int x, int y, bool on,int brushSize);

    bool getPixel(int x, int y) const;
    void clear();

    // --- 底層繪圖演算法 ---
    void drawLine(int x0, int y0, int x1, int y1, bool on,int brushSize);
    void drawRectangle(int x, int y, int w, int h, bool on, bool fill,int brushSize);
    void drawCircle(const QPoint &p1, const QPoint &p2,int brushSize);

    // --- 資料存取 ---
    //const uint8_t* getBuffer() const;
    //void setBuffer(const uint8_t* data);


    // --- 資料交換 (這裡是翻譯發生的地方) ---
    std::vector<uint8_t> getHardwareBuffer() const; // 返回硬體格式的 buffer
    void setFromHardwareBuffer(const uint8_t* data); // 從硬體格式設定

    // [新增] 负责将模型的一部分数据复制为一个独立的逻辑图像 (QImage)
    QImage copyRegionToLogicalFormat(const QRect& region) const;

    // [新增] 负责将一个逻辑图像 (QImage) 转换为硬件格式的字节向量
    static QVector<uint8_t> convertLogicalToHardwareFormat(const QImage& logicalImage);

private:
    // --- 私有輔助函式 ---
    // 這個 "raw" setPixel 是給內部繪圖演算法呼叫的，效率更高
    //void setRawPixel(int x, int y, bool on);

    //uint8_t m_buffer[OledConfig::RAM_PAGE_WIDTH * (OledConfig::DISPLAY_HEIGHT / 8)];

    std::vector<bool> m_logical_buffer;

};

#endif // OLEDDATAMODEL_H
