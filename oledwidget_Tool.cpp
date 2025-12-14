
#include "oledwidget_Paint.h"


// ================== 新增的 SLOT ==================
/**
 * @brief [SLOT] 設定目前使用的繪圖或互動工具。
 *
 * 這是一個 Qt 的槽（Slot），設計用來接收來自工具列按鈕或其他 UI 元件的訊號。
 * 當使用者點選不同的工具（如畫筆、橡皮擦、選取工具）時，這個槽會被呼叫，
 * 並更新內部狀態 `m_currentTool`。
 *
 * 這個狀態變數會直接影響滑鼠事件（如 `mousePressEvent`, `mouseMoveEvent`）的行為，
 * 從而決定使用者在畫布上的操作是進行繪製、擦除還是選取。
 *
 * @param[in] tool 要被啟用的新工具類型（來自 `ToolType` enum）。
 */
void OLEDWidget::setCurrentTool(ToolType tool) {
    m_currentTool = tool;
    /*
    if (tool != Tool_Select) {
        m_selectedRegion = QRect(); // 清除選取框
        m_isSelecting = false;
        m_isDraggingSelection = false;
        update(); // 觸發重繪
    }*/

}


/**
 * @brief [SLOT] 將像素資料轉換為 C/C++ 標頭檔陣列格式並顯示。
 *
 * 這是一個工具性槽函數，主要提供給開發者使用，以便將畫面上的圖形快速轉換為
 * 可用於嵌入式系統（如 Arduino, ESP32 等）的原始位元組陣列。
 *
 * 此函數會根據當前是否存在有效選取區 (`m_selectedRegion`) 來決定匯出的範圍：
 * - 如果有選取區，則只轉換選取區內的像素。
 * - 如果沒有選取區，則轉換整個畫面的像素。
 *
 * 轉換完成後，結果會被格式化成一個 C 語言的 `const unsigned char` 陣列，
 * 並在一個彈出視窗中顯示，方便使用者複製貼上。
 *
 * @see OledDataModel::copyRegionToLogicalFormat()
 * @see OledDataModel::convertLogicalToHardwareFormat()
 */
void OLEDWidget::showBufferDataAsHeader()
{
    // 步骤 1: 确定要导出的区域 (有选区就用选区，否则用整个屏幕)
    QRect region = m_selectedRegion.isValid() ? m_selectedRegion :
                       QRect(0, 0, OledConfig::DISPLAY_WIDTH, OledConfig::DISPLAY_HEIGHT);

    // 步骤 2: [复用!] 调用 model 将该区域转换为逻辑图像 QImage
    QImage logicalData = m_model.copyRegionToLogicalFormat(region);

    if (logicalData.isNull()) {
        return; // 如果区域无效或复制失败，则不继续
    }

    // 步骤 3: [复用!] 调用 model 将逻辑图像转换为硬件格式的 QVector
    QVector<uint8_t> hardwareData = OledDataModel::convertLogicalToHardwareFormat(logicalData);

    // ------------------------------------------------------------------
    // 到此为止，数据准备工作已完成，且没有任何重复的转换算法。
    // 下面的代码只负责将准备好的 hardwareData 格式化为字符串。
    // ------------------------------------------------------------------


    // 步骤 4: 将打包好的 hardwareData 格式化成 C 阵列字符串
    QString output;
    output += QString("// Image Data (%1x%2 region at (%3, %4))\n")
                  .arg(logicalData.width()).arg(logicalData.height())
                  .arg(region.left()).arg(region.top());
    output += QString("const uint8_t imageData[%1] = {\n    ").arg(hardwareData.size());

    for (int i = 0; i < hardwareData.size(); ++i) {
        //output += QString("0x%1, ").arg(hardwareData[i], 2, 16, QChar('0')).toUpper();

        QString hexVal = QString::number(hardwareData[i], 16).toUpper().rightJustified(2, '0');
        output += QString("0x%1, ").arg(hexVal);


        if ((i + 1) % 16 == 0 && i < hardwareData.size() - 1) {
            output += "\n    ";
        }
    }

    if (output.endsWith(", ")) {
        output.chop(2);
    }
    output += "\n};";


    // === 顯示在視窗中 ===
    QDialog *dialog = new QDialog(this);
    dialog->setStyleSheet("QDialog { background-color: white; border: 1px solid #ccc; }");
    dialog->setWindowTitle("SH1106 .h 格式輸出");
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    QTextEdit *textEdit = new QTextEdit(dialog);
    textEdit->setPlainText(output);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Courier", 10));
    textEdit->setStyleSheet("background-color: white; color: black;");

    QPushButton *copyButton = new QPushButton("複製到剪貼簿", dialog);
    connect(copyButton, &QPushButton::clicked, [output]() {
        QApplication::clipboard()->setText(output);
    });

    QPushButton *closeButton = new QPushButton("Close", dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(textEdit);

    layout->addWidget(copyButton);
    layout->addWidget(closeButton);

    dialog->resize(500, 400);
    dialog->exec();
}


