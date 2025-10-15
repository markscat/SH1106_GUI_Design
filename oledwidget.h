#ifndef OLEDWIDGET_H
#define OLEDWIDGET_H
#pragma once
#include <QWidget>
#include <QImage>

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

    // 顯示倍率 (pixel scale)

public slots: // 設為 public slot 方便從外部呼叫
    void setScale(int s) {
        scale = s > 0 ? s : 1;
        setMinimumSize(img.width()*scale, img.height()*scale);
        update();
    }

    //void setScale(int s) { scale = qMax(1, s); update(); }

protected:
    // 新增：覆寫滑鼠事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:

    // 新增：一個從 m_buffer 更新 QImage 的內部函式
    void updateImageFromBuffer();

    QImage img; // 黑白影像，用來顯示
    int scale = 6; // 放大倍率

    // 核心數據！128 * 64 / 8 = 1024 bytes
    uint8_t m_buffer[1024];

};

#endif // OLEDWIDGET_H
