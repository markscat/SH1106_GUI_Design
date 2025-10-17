#include "mainwindow.h"

/*
 * mainwindow.cpp:1:10: In included file: use of undeclared identifier 'MainWindow'; did you mean 'QMainWindow'?
oledwidget.h:34:25: error occurred here
qmainwindow.h:24:24: 'QMainWindow' declared here
 */
#include "./ui_mainwindow.h"

 // <--- 【關鍵檢查點】請確保這一行存在且沒有被註解掉！

// 在 mainwindow.cpp 顶部
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QPainter> // <--- 把这一行加进来！
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QImageReader>
#include <QImage>
#include <QBuffer>
#include <QCheckBox>

#include "sample.h"
#include "oledwidget.h"
#include "ToolType.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- 建立並注入 OLEDWidget ---
    //m_oled = new OLEDWidget(this);
    m_oled = new OLEDWidget(this);
    /*
     * mainwindow.cpp:48:18: Allocation of incomplete type 'OLEDWidget' (fix available)
mainwindow.h:9:7: forward declaration of 'OLEDWidget'
     */



    QVBoxLayout *layout = new QVBoxLayout(ui->oledPlaceholder);






    layout->addWidget(m_oled);
    //mainwindow.cpp:38:23: Cannot initialize a parameter of type 'QWidget *' with an lvalue of type 'OLEDWidget *'
    //qboxlayout.h:38:29: passing argument to parameter here

    layout->setContentsMargins(0, 0, 0, 0);


    // --- 2. 連接【功能】按鈕信號 (Clear, Export, Save, Import) ---
    //清除畫面
    connect(ui->clearButton, &QPushButton::clicked, m_oled, &OLEDWidget::clearScreen);
    /*
     * mainwindow.cpp:69:5: No matching member function for call to 'connect'
     * qobject.h:209:36: candidate function not viable: no known conversion from 'void (QAbstractButton::*)(bool)' to 'const char *' for 2nd argument
     * qobject.h:212:36: candidate function not viable: no known conversion from 'void (QAbstractButton::*)(bool)' to 'const QMetaMethod' for 2nd argument
     * qobject.h:405:41: candidate function not viable: no known conversion from 'void (QAbstractButton::*)(bool)' to 'const char *' for 2nd argument
     * qobject.h:230:9: candidate template ignored: substitution failure [with Func1 = void (QAbstractButton::*)(bool), Func2 = void (OLEDWidget::*)()]:
     * no type named 'ContextType' in 'QtPrivate::ContextTypeForFunctor<void (OLEDWidget::*)()>'
     * qobject.h:279:9: candidate function template not viable: requires 3 arguments, but 4 were provided
     */
    //在彈出對話框中產生程式碼
    connect(ui->exportButton, &QPushButton::clicked, this, &MainWindow::exportData);

    //存檔;存檔位置在當前程式目錄之下的log檔案夾中,YYYY_MM_DD_hh_mm_ss.h
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::saveData);

    //匯入圖檔
    connect(ui->importButton, &QPushButton::clicked, this, &MainWindow::importImage);


    // --- 3. 設定【繪圖工具】按鈕群組 ---`

    m_toolButtonGroup = new QButtonGroup(this);
    m_toolButtonGroup->setExclusive(true);


    // 假設您的按鈕 objectName 分別是 penButton, lineButton 等
    m_toolButtonGroup->addButton(ui->ToolPen, Tool_Pen);
    m_toolButtonGroup->addButton(ui->ToolLine, Tool_Line);
    m_toolButtonGroup->addButton(ui->ToolRectangle, Tool_Rectangle);
    m_toolButtonGroup->addButton(ui->ToolFilledRectangle, Tool_FilledRectangle);
    m_toolButtonGroup->addButton(ui->ToolCircle, Tool_Circle);

    for (QAbstractButton *button : m_toolButtonGroup->buttons()) {
        button->setCheckable(true);
    }
    /*
     * D:\for work\temp\workshorp\PC\Qt\SH1106_GUI_Design\mainwindow.cpp:99:5: c++11 range-loop might detach Qt container (QList) [clazy-range-loop-detach]
     */

    // *** 關鍵不同點：我們不直接呼叫 OLEDWidget 的方法 ***
    // *** 而是連接到 MainWindow 自己的 slot，或者直接在 lambda 中處理 ***
    connect(m_toolButtonGroup, &QButtonGroup::idClicked, this, [this](int id){
        // MainWindow 只需要知道當前工具是什麼即可
        // 繪圖的觸發由 OLEDWidget 自己的滑鼠事件處理
    });

    connect(m_toolButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int id) {
                // 在這個 lambda 函式中，我們將接收到的整數 ID
                // 轉換回 ToolType enum，然後呼叫 OLEDWidget 的方法來設定當前工具。
                m_oled->setCurrentTool(static_cast<ToolType>(id));
            });

    // 設定預設工具
    ui->ToolPen->setChecked(true);


    //QVBoxLayout *layout = new QVBoxLayout(ui->oledPlaceholder);
    //layout->addWidget(m_oled);

    layout->setContentsMargins(0, 0, 0, 0);



    // --- 把從 main.cpp 移過來的邏輯放在這裡 ---
    // 在程式啟動時，載入預設的範例圖片
    m_oled->setBuffer(sample_image);


    // ↓↓↓↓ 在这里加入以下代码来设置 Splitter 的初始尺寸 ↓↓↓↓

    // 创建一个 QList<int> 来存放每个区域的初始尺寸
    QList<int> initialSizes;
    initialSizes << 120 << 680; // 第一个数字是左区的初始宽度，第二个是右区的

    // 使用 objectName 'splitter' 来获取指向 QSplitter 的指标，并设置尺寸
    // 请确保这里的 "splitter" 和您在 .ui 文件中设置的 objectName 完全一致！
    ui->splitter->setSizes(initialSizes);



    // --- 設定一個合適的初始視窗大小 ---
    resize(1024, 600);

}


void MainWindow::exportData()
{
    const uint8_t *buffer = m_oled->getBuffer();

    QString c_array = "const unsigned char screen_data[1024] = {\n    ";
    int line_break_count = 0;

    for (int i = 0; i < 1024; ++i) {
        c_array += QString("0x%1, ").arg(buffer[i], 2, 16, QChar('0'));
        line_break_count++;
        if (line_break_count == 16) {
            c_array += "\n    ";
            line_break_count = 0;
        }
    }
    c_array.chop(c_array.endsWith(", \n    ") ? 7 : 2);
    c_array += "\n};";

    // --- 使用一個新的 QDialog 來顯示結果 ---

    // 1. 建立一個對話方塊視窗
    QDialog *exportDialog = new QDialog(this); // `this` 讓它成為 MainWindow 的子視窗
    exportDialog->setWindowTitle("匯出的 C 陣列 (可複製)");
    exportDialog->resize(600, 400);

    // 2. 建立文字編輯器和關閉按鈕
    QTextEdit *textEdit = new QTextEdit(exportDialog);
    textEdit->setPlainText(c_array);
    textEdit->setReadOnly(true);
    textEdit->setFontFamily("Courier"); // 使用等寬字體，對齊更好看

    QPushButton *closeButton = new QPushButton("關閉", exportDialog);
    connect(closeButton, &QPushButton::clicked, exportDialog, &QDialog::accept);

    // 3. 建立一個佈局來管理對話方塊內的元件
    QVBoxLayout *layout = new QVBoxLayout(exportDialog);
    layout->addWidget(textEdit);
    layout->addWidget(closeButton);

    // 4. 顯示對話方塊 (以模態方式)
    exportDialog->exec();

    // 5. 對話方塊關閉後，釋放記憶體
    delete exportDialog;
}

void MainWindow::saveData()
{
    // 1. 建立 log 资料夹
    QDir dir(QApplication::applicationDirPath()); // 获取程式执行档所在的目录
    if (!dir.exists("log")) {
        if (!dir.mkdir("log")) {
            QMessageBox::critical(this, "错误", "无法建立 log 资料夹！");
            return;
        }
    }
    dir.cd("log"); // 进入 log 资料夹

    // 2. 产生档案名称
    QString timestamp = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
    QString fileName = timestamp + ".h";
    QString filePath = dir.absoluteFilePath(fileName);

    // 3. 准备要写入的内容 (和 exportData 类似，但可以简化)setCodec
    const uint8_t* buffer = m_oled->getBuffer();
    QString content = QString("const unsigned char image_%1[] = {\n    ").arg(timestamp);
    // ... (这里是和 exportData 中一样的循环，用来生成 C 阵列字串) ...
    // ... 为了简洁，省略重复代码 ...

    QString c_array_content;
    QTextStream out(&c_array_content);
    out << "// Saved at: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << QString("const unsigned char image_%1[1024] = {\n    ").arg(timestamp);
    for (int i = 0; i < 1024; ++i) {
        out << QString("0x%1, ").arg(buffer[i], 2, 16, QChar('0'));
        if ((i + 1) % 16 == 0 && i < 1023) {
            out << "\n    ";
        }
    }
    out.seek(out.pos() - 2); // 移除最后多余的 ", "
    out << "\n};";


    // 4. 写入档案
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        //stream.setCodec("UTF-8");
        stream << c_array_content;
        file.close();
        QMessageBox::information(this, "成功", QString("档案已储存至:\n%1").arg(filePath));
    } else {
        QMessageBox::critical(this, "错误", QString("无法写入档案:\n%1").arg(filePath));
    }
}


void MainWindow::importImage()
{
    // 1. 打开一个档案选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择一张图片",
        "", // 预设目录
        "图片档案 (*.png *.bmp *.jpg *.jpeg)"
        );

    if (filePath.isEmpty()) {
        return; // 用户取消了选择
    }

    // 2. 读取图片
    QImage image;
    if (!image.load(filePath)) {
        QMessageBox::warning(this, "错误", "无法载入图片档案！");
        return;
    }



    // --- 弹出对话框，询问是否旋转 ---
    QDialog rotateDialog(this);
    rotateDialog.setWindowTitle("汇入选项");
    QVBoxLayout layout(&rotateDialog);
    QCheckBox *rotateCheckbox = new QCheckBox("将图片旋转90度 (用于竖屏)", &rotateDialog);
    QPushButton *okButton = new QPushButton("确定", &rotateDialog);

    connect(okButton, &QPushButton::clicked, &rotateDialog, &QDialog::accept);
    layout.addWidget(rotateCheckbox);
    layout.addWidget(okButton);

    bool rotate = false;
    if (rotateDialog.exec() == QDialog::Accepted) {
        rotate = rotateCheckbox->isChecked();
    }

    // 如果需要旋转
    if (rotate) {
        QTransform transform;
        transform.rotate(90);
        image = image.transformed(transform); // <-- 对原始 image 进行旋转
        //monoImage = monoImage.transformed(transform);
        //monoImage = monoImage.scaled(128, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // --- 关键步骤：图片处理和转换 ---
    // 将图片转换为128x64的单色图
    // Qt::MonoOnly -> dither抖动算法，效果比较好
    // Qt::Threshold -> 阈值算法，黑白分明


   /* QImage monoImage = image.scaled(128, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                           .convertToFormat(QImage::Format_Mono, Qt::MonoOnly);
    */

    QImage scaledImage = image.scaled(128, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 4. 创建一个 128x64 的标准单色画布，并填充背景色（白色索引=0）
    QImage canvas(128, 64, QImage::Format_Mono);
    canvas.fill(0); // 0 = 白色
    // 5. 使用 QPainter 将缩放后的图片绘制到画布中心

    QPainter painter(&canvas);
    /*
     * mainwindow.cpp:223:14: Variable has incomplete type 'QPainter' (fix available) qwindowdefs.h:28:7: forward declaration of 'QPainter'
     */
    int x = (canvas.width() - scaledImage.width()) / 2;
    int y = (canvas.height() - scaledImage.height()) / 2;
    // 将 scaledImage 临时转换为 RGB 来绘制，以避免颜色索引问题
    painter.drawImage(x, y, scaledImage.convertToFormat(QImage::Format_RGB32));
    painter.end(); // 结束绘制，非常重要！


    // 6. 将绘制好内容的画布，整体转换为最终的单色格式
    QImage monoImage = canvas.convertToFormat(QImage::Format_Mono, Qt::DiffuseDither);

    // [可选的除错步骤] 保存这张最终的画布，看看是否居中
    monoImage.save("debug_centered_mono_image.png");


    // --- 将处理好的 QImage 转换为 SH1106 的 buffer 格式 ---
    // 这是最复杂的一步，我们需要写一个转换函式
    uint8_t buffer[1024] = {0};

    // 遍历处理后的 monoImage 的每一个像素
    for (int y = 0; y < monoImage.height(); ++y) {
        for (int x = 0; x < monoImage.width(); ++x) {

            int colorIndex = monoImage.pixelIndex(x, y);

            bool pixelIsOn = (colorIndex == 1);

            if (pixelIsOn) {
                // 计算像素在 buffer 中的位置 (和 setPixel 逻辑一样)
                int page = y / 8;
                int bit_index = y % 8;
                int byte_index = page * 128 + x;
                if (byte_index >= 0 && byte_index < 1024) { // 边界检查
                    buffer[byte_index] |= (1 << bit_index);
                }
            }
        }
    }


    // 4. 将转换好的 buffer 发送到 OLEDWidget 显示
    m_oled->setBuffer(buffer);
}


MainWindow::~MainWindow()
{
    delete ui;
}
