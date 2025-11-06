#ifndef OLEDWIDGET_H
#define OLEDWIDGET_H
#pragma once


#include "ToolType.h"
#include "config.h"
#include "oleddatamodel.h"

class OLEDWidget : public QWidget {
    Q_OBJECT


public:
    // --- 构造函数 ---
    explicit OLEDWidget(QWidget *parent = nullptr);

    // --- 公开接口 (给 MainWindow 调用) ---
    // 清除螢幕
    void clearScreen();

    void setScale(int s);

    void setBrushSize(int size);

    // setBuffer，用於未來載入檔案
    void setBuffer(const uint8_t *buffer);

    // getHardwareBuffer 用于导出内部逻辑模型到硬体格式
    std::vector<uint8_t> getHardwareBuffer() const;


    // --- 工具 & 状态查询 ---
    void setCurrentTool(ToolType tool);
    QRect selectedRegion() const { return m_selectedRegion;}


// --- 公开槽 (Public Slots, 响应 UI 信号) ---

public slots:
     void handleCopy();
    void showBufferDataAsHeader();



signals:
    // 現在 MOC 會看到並處理這個信號了
    void coordinatesChanged(const QPoint &pos);


protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override; // <-- 新增 release 事件
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;


private:
    // --- 私有辅助函式 ---
    void updateImageFromModel(); // 从模型更新 QImage
    QPoint convertToOLED(const QPoint &pos);

    void handleSelectPress(QMouseEvent *event);
    void handleSelectMove(QMouseEvent *event);
    void handleSelectRelease(QMouseEvent *event);
    //void startPastePreview(const QVector<uint8_t>& data, int width, int height);   //ok
    void startPastePreview(const QImage& logicalImage);
    void commitPaste();

    
    
    
    // 新增：取得緩衝區數據，用於匯出
    //理由: 此功能现在由 MainWindow 直接调用 getHardwareBuffer() 实现，OLEDWidget 不再需要返回一个可能失效的内部指针。
    //const uint8_t* getBuffer() const;

    // 載入位圖陣列（MSB 優先，每列 bytes = (w+7)/8）
    //理由: 此功能由 setBuffer(const uint8_t*) 完美取代，且新方法符合模型-视图架构。
    //void loadBitmap(const uint8_t *data, int w, int h);

    //void  pasteBlock(const QRect &region);
//void startPastePreview(const uint8_t* data, int width, int height)

private:

    OledDataModel m_model;      // [核心] 数据模型 (单一事实来源)
    QImage m_image;         // 用于在屏幕上绘制的缓存图像

    int scale = 7; // 放大倍率

    //直接用 ToolType
    ToolType m_currentTool;

    // 新增狀態變數
    bool m_isDrawing;

    //筆刷大小,預設為一個假象素
    // 注意：這裡的 `m_brushSize` 代表的是邊長，例如 1 代表 1x1，2 代表 2x2。
    int m_brushSize=1; // <-- 新增：筆刷大小 (1x1, 2x2, 3x3 等)

    // 儲存繪圖的起始點 (128x64 座標系)
    QPoint m_startPoint;

    // 儲存繪圖的結束點/當前點
    QPoint m_endPoint;



    // 新增：一個從 m_buffer 更新 QImage 的內部函式
    // [注释掉] 理由: 此函数用于从旧的 m_buffer 更新图像，已被 updateImageFromModel() 取代。
    //void updateImageFromBuffer();


    // 实际的显示缓冲区，现在大小是 8页 * 132字节/页 = 1056 字节
    // [注释掉] 理由: 这是旧的物理数据模型，已被 OledDataModel 内部的逻辑模型取代。
    //uint8_t m_buffer[OledConfig::RAM_PAGE_WIDTH * (OledConfig::DISPLAY_HEIGHT / 8)]; // 8页 * 132字节 = 1056字节

    // 【新增】这个是给内部绘图演算法用的"高效版"
    //void setPixel(int x, int y, bool on, uint8_t* buffer);


    //座標位置
     // [注释掉] 理由: 不再需要直接持有 QLabel 的指针，通过信号/槽与 MainWindow 解耦。
    //QLabel* m_labelCoordinate = nullptr;
    //座標位置


    //選取複製
#ifdef SelectCopy
/*
| 變數名稱                 | 功能說明                                  |
| ----------------------- | -----------------------------------------|
|  m_selectedRegion       | 目前選取框的位置與大小（你原本的框）           |
|  m_isSelecting`         | 是否正在用左鍵畫選取框                       |
|  m_isDraggingSelection` | 是否正在右鍵拖曳框                          |
|  m_dragOffset`          | 滑鼠點擊位置相對於框框左上角的偏移             |
|  m_dragStartRegion`     | 👉 拖曳開始那一刻的選取框位置，用來計算 offset |

*/
    /* @brief 當前選取框的矩形範圍 */
    QRect m_selectedRegion; // 儲存選取區域（OLED 座標系）

    /* @brief 是否正在用左鍵畫選取框*/
    bool m_isSelecting = false;

    // [新增/修改] 贴上功能的状态变量

    /* @brief 拖曳中滑鼠位置相對框框左上角的偏移*/
    //QPoint m_dragOffset;

    /* @brief 是否正在拖曳選取框*/
    //bool m_isDraggingSelection = false;

    bool m_pastePreviewActive = false;
    QPoint m_pastePosition = QPoint(0, 0);
   /* int m_pasteWidth = 0;
    int m_pasteHeight = 0;
    QVector<uint8_t> m_pasteData;*/
    QImage m_pastePreviewImage;

    // ----- 新增 -----
    /* @brief 儲存選取區的暫存圖 (Format_Mono) */
    //QImage m_copyBuffer;

    /* @brief 被複製區域在原圖的位置 (左上角) */
    //QPoint m_copyOrigin;        //

    //QRect m_dragStartRegion;   // ✅ 新增：記錄拖曳前的選取框位置

#endif

    //選取複製
    //void verifySelectionFlow(const QString &stage);
#ifdef SelectCopy_

    QImage m_clipboard; // 暫存複製的區塊
#endif

#ifdef DrawTool_

    // 新增繪圖函式 (基於 setPixel)
    void drawLine(int x0, int y0, int x1, int y1, bool on, uint8_t* buffer);
    void drawRectangle(int x, int y, int w, int h, bool on, bool fill, uint8_t* buffer);
    void drawCircle(const QPoint &p1, const QPoint &p2, uint8_t* buffer);
#endif

};

#endif // OLEDWIDGET_H
