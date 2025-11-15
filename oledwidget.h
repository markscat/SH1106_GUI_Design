#ifndef OLEDWIDGET_H
#define OLEDWIDGET_H
#pragma once


#include "ToolType.h"
#include "config.h"
#include "oleddatamodel.h"
#include "oleddataconverter.h"

class OLEDWidget : public QWidget {
    Q_OBJECT


public:
    // --- 构造函数 ---
    explicit OLEDWidget(QWidget *parent = nullptr);

    // --- 公开接口 (给 MainWindow 调用) ---
    // 清除螢幕
    void clearScreen();

    void setScale(int s);

     QImage getCurrentImage() const;

#ifdef modify_1107
    // [新增] 設置內部像素顯示縮放的方法 (如果你需要動態切換 1:1 和其他縮放)
    void setPixelScale(int newPixelScale);

    // [新增] 獲取當前內部像素顯示縮放的方法 (如果需要)
    int pixelScale() const { return m_pixelScale; }
#endif

    void setBrushSize(int size);

    // setBuffer，用於未來載入檔案
    void setBuffer(const uint8_t *buffer);

    // getHardwareBuffer 用于导出内部逻辑模型到硬体格式
    std::vector<uint8_t> getHardwareBuffer() const;


    // --- 工具 & 状态查询 ---
    void setCurrentTool(ToolType tool);
    QRect selectedRegion() const { return m_selectedRegion;}

    /**
     * @brief 從一個邏輯格式的 QImage 更新整個 OLED 顯示內容。
     * @param image 來源圖片，必須是 QImage::Format_Mono 格式。
     */
    void updateOledFromImage(const QImage& image);


// --- 公开槽 (Public Slots, 响应 UI 信号) ---

public slots:
    void handleCopy();
    void showBufferDataAsHeader();
    void commitPaste();
    void handleCut();
    void handlePaste(); // <-- 新增這個槽



#ifdef newcode_Buffer
    void handlePaste(); // <-- 新增這個槽
#endif


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

    // --- 私有辅助函式 ---
    void updateImageFromModel(); // 从模型更新 QImage
    QPoint convertToOLED(const QPoint &pos);

    void handleSelectPress(QMouseEvent *event);
    void handleSelectMove(QMouseEvent *event);
    void handleSelectRelease(QMouseEvent *event);
    void startPastePreview(const QImage& logicalImage);

    //QImage m_clipboardImage; // <-- 【核心】新增這個成員變數，作為持久化的剪貼簿
    //QImage m_selectionBuffer;  //新增這個成員變數，作為持久化的buffer
    QImage m_persistentBuffer;  // 持久化缓冲区
    bool m_hasValidBuffer;      // 标记缓冲区是否有效



    /* @brief 當前選取框的矩形範圍 */
    QRect m_selectedRegion; // 儲存選取區域（OLED 座標系）

    /* @brief 是否正在用左鍵畫選取框*/
    bool m_isSelecting = false;

    bool m_pastePreviewActive = false;

    QPoint m_pastePosition = QPoint(0, 0);

    QImage m_pastePreviewImage;

    QPoint m_dragStartPos;        // 滑鼠拖曳開始時的 widget 座標

    QPoint m_dragStartPastePos;   // 拖曳開始時的貼上預覽位置


};

#endif // OLEDWIDGET_H
