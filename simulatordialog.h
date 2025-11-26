#ifndef SIMULATORDIALOG_H
#define SIMULATORDIALOG_H

#include "config.h"
#include <vector>
#include <cstdint> // for uint8_t


// 前置宣告，加快編譯速度
class QTextEdit;

class SimulatorDialog : public QDialog {
    Q_OBJECT

public:
    explicit SimulatorDialog(QWidget *parent = nullptr);
    ~SimulatorDialog(); // 記得要有解構子

    // 提供一個公開方法讓 MainWindow 拿資料
    std::vector<uint8_t> getBuffer() const;

private slots:
    void onSimulateClicked();

private:
    QTextEdit *inputText;
    std::vector<uint8_t> m_buffer;

    // 初始化 UI 的 helper
    void setupUi();
};


#endif // SIMULATORDIALOG_H
