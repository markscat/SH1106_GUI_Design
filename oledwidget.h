#ifndef OLEDWIDGET_H
#define OLEDWIDGET_H
#pragma once

#define RAM_PAGE_WIDTH 132       // SH1106 RAM 寬度
#define COLUMN_OFFSET 2          // SH1106 顯示起始 column 偏移
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QPoint> // 確保包含了 QPoint
#include <QScrollArea>
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

    // 設定筆刷大小的函式
    void setBrushSize(int size);

signals:
    // 現在 MOC 會看到並處理這個信號了
    void coordinatesChanged(const QPoint &pos);


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
    void leaveEvent(QEvent *event) override;

    //選取複製
    void handleSelectCopyPress(const QPoint &pos);
    void handleSelectCopyMove(const QPoint &pos);
    QPoint convertToOLED(const QPoint &pos);

    //選取複製

private:


    QImage img; // 黑白影像，用來顯示

    // 核心數據！128 * 64 / 8 = 1024 bytes

    //直接用 ToolType
    ToolType m_currentTool;

    // 新增狀態變數
    bool m_isDrawing;
    // 儲存繪圖的起始點 (128x64 座標系)
    QPoint m_startPoint;

    // 儲存繪圖的結束點/當前點
    QPoint m_endPoint;
    // 新增：一個從 m_buffer 更新 QImage 的內部函式
    void updateImageFromBuffer();

    int m_brushSize; // <-- 新增：筆刷大小 (1x1, 2x2, 3x3 等)
    // 注意：這裡的 `m_brushSize` 代表的是邊長，例如 1 代表 1x1，2 代表 2x2。

    // 实际的显示缓冲区，现在大小是 8页 * 132字节/页 = 1056 字节
    uint8_t m_buffer[RAM_PAGE_WIDTH * (DISPLAY_HEIGHT / 8)]; // 8页 * 132字节 = 1056字节



    // 【新增】这个是给内部绘图演算法用的"高效版"
    void setPixel(int x, int y, bool on, uint8_t* buffer);

    // 新增繪圖函式 (基於 setPixel)
    void drawLine(int x0, int y0, int x1, int y1, bool on, uint8_t* buffer);
    void drawRectangle(int x, int y, int w, int h, bool on, bool fill, uint8_t* buffer);
    void drawCircle(const QPoint &p1, const QPoint &p2, uint8_t* buffer);

    int scale = 7; // 放大倍率
    //座標位置
    QLabel* m_labelCoordinate = nullptr;
    //座標位置


    //選取複製
    bool m_isSelecting = false;
    QRect m_selectedRegion; // 儲存選取區域（OLED 座標系）

    //選取複製
};

#endif // OLEDWIDGET_H
