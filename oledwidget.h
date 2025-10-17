#ifndef OLEDWIDGET_H
#define OLEDWIDGET_H
#pragma once
#include <QWidget>
#include <QImage>
#include "ToolType.h"

class OLEDWidget : public QWidget {
    Q_OBJECT
public:

    explicit OLEDWidget(QWidget *parent = nullptr);

    // 新增：清除螢幕
    void clearScreen();

    // 新增：設定單一像素點，這是核心繪圖函式
    void setPixel(int x, int y, bool on);


    // 新增：取得緩衝區數據，用於匯出
    const uint8_t* getBuffer() const;

    // 保留 setBuffer，用於未來載入檔案
    void setBuffer(const uint8_t *buffer);

    // 載入位圖陣列（MSB 優先，每列 bytes = (w+7)/8）
    void loadBitmap(const uint8_t *data, int w, int h);



     // 設為 public slot 方便從外部呼叫
public slots:
    // 這個 slot 讓 MainWindow 能設定當前工具
    void setCurrentTool(ToolType tool);
    void setScale(int s);

protected:
    // 新增：覆寫滑鼠事件
    void mouseReleaseEvent(QMouseEvent *event) override; // <-- 新增 release 事件
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:


    QImage img; // 黑白影像，用來顯示
    // 核心數據！128 * 64 / 8 = 1024 bytes
    uint8_t m_buffer[1024];
    ToolType m_currentTool;  // 直接用 ToolType

    // 新增狀態變數
    bool m_isDrawing;
    QPoint m_startPoint;// 儲存繪圖的起始點 (128x64 座標系)


    QPoint m_endPoint;   // 儲存繪圖的結束點/當前點
    // 新增：一個從 m_buffer 更新 QImage 的內部函式
    void updateImageFromBuffer();

    // 新增繪圖函式 (基於 setPixel)
    void drawLine(int x0, int y0, int x1, int y1, bool on);
    void drawRectangle(int x, int y, int width, int height, bool on, bool fill);
    void drawCircle(int centerX, int centerY, int radius, bool on); // 簡化版圓形
    int scale = 6; // 放大倍率
};

#endif // OLEDWIDGET_H
