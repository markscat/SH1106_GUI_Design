#include "oledwidget.h"
//oledwidget_API.cpp:1:10: Expected "FILENAME" or <FILENAME>



//留下clearScreen
void OLEDWidget::clearScreen() {
    //oledwidget_API.cpp:7:6: Use of undeclared identifier 'OLEDWidget'

    // 將內部緩衝區全部填 0
    //memset(m_buffer, 0, sizeof(m_buffer));
    //updateImageFromBuffer(); // 更新顯示

    // 1. 调用数据模型来清除数据
    m_model.clear();

    // 2. 调用辅助函数，从更新后的模型同步到显示图像
    updateImageFromModel();
}



void OLEDWidget::setScale(int s) {
    const int minScale = 1;
    const int maxScale = 20; // 依需求調整最大放大倍數
    scale = std::clamp(s, minScale, maxScale);
    //scale = s > 0 ? s : 1;
    //setFixedSize(img.width() * scale, img.height() * scale);
    setFixedSize(m_image.width() * scale, m_image.height() * scale);

    update();
}


QImage OLEDWidget::getCurrentImage() const
{
    // m_image 是您用來在 paintEvent 中繪圖的那個 QImage
    return m_image;
}



void OLEDWidget::setBrushSize(int size)
{
    // 安全限制：筆刷大小只能是 1～6

    m_brushSize = std::clamp(size, 1, 6); // 限制笔刷大小在 1-6 之间

}


void OLEDWidget::setBuffer(const uint8_t *buffer){
    // 同步内部状态

    // 1. 调用数据模型的新方法，从硬体 buffer 载入数据并完成翻译
    m_model.setFromHardwareBuffer(buffer);

    // 2. 数据模型更新后，同步到显示图像
    updateImageFromModel();
}
