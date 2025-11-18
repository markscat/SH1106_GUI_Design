#include "oledwidget_Paint.h"

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



    /**
 * @brief 取得符合硬體格式的顯示緩衝區。
 *
 * 這是一個方便的包裝函式（wrapper function），作為 `OLEDWidget`（View）對外的介面，
 * 用於取得底層資料模型（Model）所產生的硬體緩衝區。
 * 實際的格式轉換邏輯由 `m_model` 物件處理，此處僅做轉發呼叫。
 * 這樣設計符合 Model-View 的職責分離原則。
 *
 * @return std::vector<uint8_t> 一個包含可以直接寫入 OLED 硬體的原始資料緩衝區。
 * @see OledDataModel::getHardwareBuffer() const
 */
    std::vector<uint8_t> OLEDWidget::getHardwareBuffer() const
{
    // 直接返回从数据模型翻译过来的硬体 buffer
    return m_model.getHardwareBuffer();
}


