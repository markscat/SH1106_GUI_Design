#include "simulatordialog.h"

#include <QRegularExpression>

SimulatorDialog::SimulatorDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
}

SimulatorDialog::~SimulatorDialog() {
    // 雖然 Qt 的 Parent-Child 機制會自動刪除 child widget
    // 但如果有非 Qt 物件的資源要在這裡釋放
}

void SimulatorDialog::setupUi() {
    setWindowTitle("SH1106 硬體模擬器 (Hex Import)");
    resize(600, 400);

    // 使用垂直佈局
    QVBoxLayout *layout = new QVBoxLayout(this);

    // 說明標籤
    layout->addWidget(new QLabel("請貼上 C 語言陣列 (SH1106 垂直頁面格式, 1024 bytes):\n支援格式: 0xFF, 0xA1... 或純 hex 字串"));

    // 文字輸入框
    inputText = new QTextEdit(this);
    inputText->setPlaceholderText("例如: const unsigned char img[] = { 0xFF, 0x00, 0xA1, ... };");
    // 設定等寬字型，方便看對齊
    QFont font("Courier");
    font.setStyleHint(QFont::TypeWriter);
    inputText->setFont(font);
    layout->addWidget(inputText);

    // 按鈕
    QPushButton *btn = new QPushButton("載入並模擬", this);
    layout->addWidget(btn);

    // 連接信號
    connect(btn, &QPushButton::clicked, this, &SimulatorDialog::onSimulateClicked);
}

std::vector<uint8_t> SimulatorDialog::getBuffer() const {
    return m_buffer;
}

void SimulatorDialog::onSimulateClicked() {
    QString raw = inputText->toPlainText();
    m_buffer.clear();

    // 1. 強大的字串清洗與解析
    // 使用 Regular Expression 找出所有的 hex 數值 (例如 0xFF, FF, 0xff)
    QRegularExpression hexRegex("0x[0-9a-fA-F]+|[0-9a-fA-F]{2}");
    auto matches = hexRegex.globalMatch(raw);

    while (matches.hasNext()) {
        auto match = matches.next();
        bool ok;
        // 轉成 hex 整數
        int val = match.captured().toInt(&ok, 16);
        if (ok) {
            m_buffer.push_back(static_cast<uint8_t>(val));
        }
    }

    // 2. 關鍵：防止雪花雜訊的安全檢查
    // SH1106 / SSD1306 128x64 解析度的標準緩衝區大小
    const size_t requiredSize = 128 * 64 / 8; // 1024 bytes

    if (m_buffer.empty()) {
        QMessageBox::warning(this, "錯誤", "未偵測到有效的 Hex 數據！\n請檢查輸入內容。");
        return;
    }

    if (m_buffer.size() > requiredSize) {
        QMessageBox::information(this, "截斷警告",
                                 QString("資料長度 (%1 bytes) 超過標準大小 (1024 bytes)。\n系統將自動截斷多餘部分。").arg(m_buffer.size()));
        m_buffer.resize(requiredSize);
    }
    else if (m_buffer.size() < requiredSize) {
        // 如果資料不足，我們不視為錯誤，而是視為「部分更新」或「資料不全」
        // 自動補 0 (黑色) 以填滿緩衝區，防止讀取越界造成的雜訊
        m_buffer.resize(requiredSize, 0x00);
    }

    // 關閉視窗並回傳 Accepted 結果給 MainWindow
    accept();
}
