#include "oledwidget_Paint.h"



void OLEDWidget::commitPaste()
{
        qDebug() << "[commitPaste] function entered";
    // 步骤 1: 安全检查
    // 确保我们确实处于贴上模式，并且有有效的预览图像数据。
    if (!m_pastePreviewActive || m_pastePreviewImage.isNull()) {

        return;
    }

    bool anyPixelSet = false; // 添加一个标志，检查是否有任何像素被设置

    // 步骤 2: 遍历预览图像的每一个像素
    // 我们需要将 m_pastePreviewImage 中的像素，一个一个地“复制”到 m_model 中。
    for (int y = 0; y < m_pastePreviewImage.height(); ++y) {
        for (int x = 0; x < m_pastePreviewImage.width(); ++x) {
            // 步骤 3: 检查预览图像在该点是否有像素 (是否为亮色)
            // .pixelIndex() 对于单色图 (Format_Mono) 来说，
            // 返回 1 代表前景色 (亮)，返回 0 代表背景色 (暗)。
            if (m_pastePreviewImage.pixelIndex(x, y) == 1) {

                // 步骤 4: 如果有像素，计算它在主数据模型 m_model 中的目标坐标
                int targetX = m_pastePosition.x() + x;
                int targetY = m_pastePosition.y() + y;

                // 步骤 5: [核心] 调用 m_model 的 setPixel，将数据写入核心模型
                // 我们不需要关心 setPixel 内部是怎么实现的，只需要相信它能
                // 正确地在 (targetX, targetY) 位置上点亮一个像素。
                // 这里的笔刷大小应为 1，因为我们是逐像素复制。
                m_model.setPixel(targetX, targetY, true, 1);
                                anyPixelSet = true;
                 //qDebug() << "[commitPaste] anyPixelSet = true, updating image...";
            }
        }
    }

    if (anyPixelSet) {
            qDebug() << "[commitPaste] anyPixelSet = true, updating image...";
        updateImageFromModel();
        update();
    }

     //清理貼上狀態
    m_pastePreviewActive = false;
    m_pastePreviewImage = QImage();

    //updateImageFromModel();
}


/**
 * @brief [SLOT] 處理「複製」操作的槽函數。
 *
 * 當使用者觸發複製動作時（例如透過選單或快捷鍵），此槽函數會被呼叫。
 * 它的主要工作是從資料模型 `m_model` 中，將目前選取區域 (`m_selectedRegion`) 的像素資料
 * 複製成一個獨立的、邏輯格式的 QImage。
 *
 * @note 此函數的行為是「複製並立即準備貼上」。它不會將資料存放到系統的剪貼簿，
 *       而是直接呼叫 `startPastePreview()`，讓使用者可以立刻看到複製內容的預覽並移動它。
 *       如果當前沒有有效的選取區域，此函數將不會執行任何操作。
 *
 * @see OledDataModel::copyRegionToLogicalFormat()
 * @see startPastePreview()
 * @see handlePaste()
 */
void OLEDWidget::handleCopy(){
    // 步驟 1: 檢查是否有有效的選取區域
    if (!m_selectedRegion.isValid())
    {
        return; // 沒有選取框就不做
    }
    m_persistentBuffer = m_model.copyRegionToLogicalFormat(m_selectedRegion);
    m_hasValidBuffer = !m_persistentBuffer.isNull();  // 标记有效
    //m_selectedRegion = QRect();
    update();

}

/**
 * @brief [SLOT] 處理「剪下」操作的槽函數。
 *
 * 剪下操作是一個複合動作，依序執行以下三件事：
 * 1.  **複製 (Copy)**: 將目前選取區域 (`m_selectedRegion`) 的像素資料複製到一個暫存的 QImage 中。
 * 2.  **刪除 (Delete)**: 將原選取區域的像素從資料模型 (`m_model`) 中清除（設為熄滅）。
 * 3.  **貼上預覽 (Paste Preview)**: 立刻進入「貼上預覽」模式，讓使用者可以移動剛剪下的內容到新位置。
 *
 * 如果當前沒有有效的選取區域，此函數將不會執行任何操作。
 *
 * @see handleCopy()
 * @see OledDataModel::copyRegionToLogicalFormat()
 * @see OledDataModel::drawRectangle()
 * @see startPastePreview()
 */
void OLEDWidget::handleCut() {
    // 步驟 1: 檢查是否有有效的選取區域
    if (!m_selectedRegion.isValid()) {

        return; // 沒有選取框就不做任何事
    }

    // 保存到持久化缓冲区
    m_persistentBuffer = m_model.copyRegionToLogicalFormat(m_selectedRegion);
    m_hasValidBuffer = !m_persistentBuffer.isNull();

    if (m_persistentBuffer.isNull()) {
        return;
    }

    // ================== 2. 刪除 (Delete) ==================
    // [優化!] 指揮 model 在原選區位置畫一個「熄滅的、實心的」矩形，
    // 這比逐點清除像素的效率高得多。
    m_model.drawRectangle(
        m_selectedRegion.x(),
        m_selectedRegion.y(),
        m_selectedRegion.width(),
        m_selectedRegion.height(),
        false, // on = false 代表「擦除」
        true,  // fill = true 代表「實心」
        1      // 筆刷大小在此不重要
        );


    startPastePreview(m_persistentBuffer);
    m_pastePosition = m_selectedRegion.topLeft();
    update();
    m_selectedRegion = QRect();
    update();
    setFocus(); // 【新增】确保OLEDWidget获得焦点
}


/**
 * @brief [SLOT] 處理「貼上」操作的槽函數。
 *
 * 當使用者觸發貼上動作時，此槽函數被呼叫。
 * 它會檢查內部剪貼簿 (`m_persistentBuffer`) 是否有有效的圖像資料。
 * 如果有，它會呼叫 `startPastePreview()`，使用剪貼簿中的圖像
 * 來啟動一個新的貼上預覽流程，讓使用者可以決定貼上的位置。
 *
 * @note 此函數可以被多次呼叫，每次都會從同一個剪貼簿內容創建一個新的貼上預覽。
 * @see startPastePreview()
 * @see m_clipboardImage
 */
void OLEDWidget::handlePaste()
{
    // 步驟 1: 檢查剪貼簿是否為空
    if (!m_hasValidBuffer || m_persistentBuffer.isNull())  {
        return; // 剪貼簿沒東西，就不做任何事
    }

    m_pastePreviewImage = m_persistentBuffer;
    m_pastePreviewActive = true;

    // 启动粘贴预览（使用持久化缓冲区数据）
    startPastePreview(m_persistentBuffer);
    m_pastePosition = QPoint(0, 0);  // 或设置为当前鼠标位置

    update();
    setFocus(); // 【新增】确保OLEDWidget获得焦点

}
