#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "sample.h"
#include "oledwidget.h"
#include "ToolType.h"

//#define test_1029
#define org_1025

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

/*
    if (ui->oledPlaceholder) {
        // 建立真正的 OLEDWidget
        OLEDWidget *oled = new OLEDWidget(this); // 父 widget 設為 MainWindow
        oled->setObjectName("oledPlaceholder");

        // 把原本的 placeholder 隱藏
        ui->oledPlaceholder->hide();

        // 直接用指標替換，ui 內存取會指向 OLEDWidget
        ui->oledPlaceholder = oled;
    }
*/

    m_originalOledSize = ui->oledPlaceholder->size(); // 或 m_oled->sizeHint()

    // --- 建立並注入 OLEDWidget ---
    m_oled = new OLEDWidget(this);

    scrollArea = new QScrollArea(this);

    scrollArea->setWidget(m_oled);

    scrollArea->setWidgetResizable(false); // 保持原始比例，不自動拉伸

    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    scrollArea->setFrameShape(QFrame::NoFrame);

    scrollArea->setStyleSheet("background: transparent;");


    QVBoxLayout *layout = new QVBoxLayout(ui->oledPlaceholder);
    layout->addWidget(scrollArea);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // --- 2. 連接【功能】按鈕信號 (Clear, Export, Save, Import) ---
    //清除畫面
    connect(ui->clearButton, &QPushButton::clicked, m_oled, &OLEDWidget::clearScreen);

    //匯出程式碼到對話框
    connect(ui->exportButton, &QPushButton::clicked, this, &MainWindow::exportData);

    //存檔;存檔位置在當前程式目錄之下的log檔案夾中,YYYY_MM_DD_hh_mm_ss.h
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::saveData);

    //匯入圖檔
    connect(ui->importButton, &QPushButton::clicked, this, &MainWindow::importImage);

    //重製繪圖框尺寸
    connect(ui->resetOledSizeButton, &QPushButton::clicked, this, &MainWindow::resetOledPlaceholderSize);

    // 【新增】將 OLEDWidget 的信號連接到 MainWindow 的槽
    connect(m_oled, &OLEDWidget::coordinatesChanged, this, &MainWindow::updateCoordinateLabel);

    //<筆刷功能>
    m_oled->setBrushSize(1); // 預設 1x1
    //加入brushSizeComboBox的功能選項
    ui->brushSizeComboBox->addItem("1x1", 1);
    ui->brushSizeComboBox->addItem("2x2", 2);
    ui->brushSizeComboBox->addItem("3x3", 3);
    ui->brushSizeComboBox->addItem("4x4", 4);
    ui->brushSizeComboBox->addItem("5x5", 5);
    ui->brushSizeComboBox->addItem("6x6", 6);

    connect(ui->brushSizeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                int brushSize = ui->brushSizeComboBox->itemData(index).toInt();
                m_oled->setBrushSize(brushSize);
            });

//</筆刷功能>

    // --- 3. 設定【繪圖工具】按鈕群組 ---`
    m_toolButtonGroup = new QButtonGroup(this);
    m_toolButtonGroup->setExclusive(true);

    // 假設您的按鈕 objectName 分別是 penButton, lineButton 等
    m_toolButtonGroup->addButton(ui->ToolPen, Tool_Pen);
    m_toolButtonGroup->addButton(ui->ToolLine, Tool_Line);
    m_toolButtonGroup->addButton(ui->ToolRectangle, Tool_Rectangle);
    m_toolButtonGroup->addButton(ui->ToolFilledRectangle, Tool_FilledRectangle);
    m_toolButtonGroup->addButton(ui->ToolCircle, Tool_Circle);

    //選取複製功能
/*
    Tool_Select,// pushButton_Select,
    Tool_Copy,// pushButton_Copy,
    Tool_Cut, //pushButton_Cut,
    Tool_Paste//pushButton_paste
 */
#ifdef SelectCopy
    // 隱藏按鍵的方式
    //ui->pushButton_Select->hide();
    //或
    //ui->pushButton_Select->setVisible(false);

    m_toolButtonGroup->addButton(ui->pushButton_Select, Tool_Select);
    m_toolButtonGroup->addButton(ui->pushButton_Copy, Tool_Copy);
    m_toolButtonGroup->addButton(ui->pushButton_paste, Tool_Paste);
    m_toolButtonGroup->addButton(ui->pushButton_Cut, Tool_Cut);


#endif
    const QList<QAbstractButton*> &buttons = m_toolButtonGroup->buttons();
    for (QAbstractButton *button : buttons) {
        button->setCheckable(true);
    }

    connect(m_toolButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int id) {
                // 在這個 lambda 函式中，我們將接收到的整數 ID
                // 轉換回 ToolType enum，然後呼叫 OLEDWidget 的方法來設定當前工具。
                m_oled->setCurrentTool(static_cast<ToolType>(id));
            });


    // 設定預設工具
    ui->ToolPen->setChecked(true);
    layout->setContentsMargins(0, 0, 0, 0);
    // ↓↓↓↓ 在这里加入以下代码来设置 Splitter 的初始尺寸 ↓↓↓↓

    // 创建一个 QList<int> 来存放每个区域的初始尺寸
    QList<int> initialSizes;
    initialSizes << 120 << 680; // 第一个数字是左区的初始宽度，第二个是右区的

    // 使用 objectName 'splitter' 来获取指向 QSplitter 的指标，并设置尺寸
    // 请确保这里的 "splitter" 和您在 .ui 文件中设置的 objectName 完全一致！
    ui->splitter->setSizes(initialSizes);


    m_oled->setBuffer(sample_image);



    // --- 設定一個合適的初始視窗大小 ---
    //resize(1050, 600);
    resize(m_oled->width() + 150, m_oled->height() + 150);
    setFixedSize(size()); // 鎖定大小，禁止拉伸

}


void MainWindow::updateCoordinateLabel(const QPoint &pos)
{
    // 檢查座標是否有效 (我們在 leaveEvent 中發送了 -1, -1)
    if (pos.x() < 0 || pos.y() < 0 || pos.x() >= 128 || pos.y() >= 64) {
        // 如果無效，顯示預設文字
        ui->label_coordinate->setText("(x, y): --, --");
    } else {
        // 如果有效，格式化字串並更新 QLabel
        QString text = QString("(x, y): %1, %2").arg(pos.x()).arg(pos.y());
        ui->label_coordinate->setText(text);
    }
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
    QString fileName = timestamp + ".c";
    QString filePath = dir.absoluteFilePath(fileName);

    // 3. 准备要写入的内容 (和 exportData 类似，但可以简化)setCodec
    const uint8_t* buffer = m_oled->getBuffer();

    QString c_array_content;
    QTextStream out(&c_array_content);
    out << "// Saved at: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << "#include <../" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << ".h>\n";

#define test2
    #ifdef test2
    out << QString("const unsigned char image_%1[%2] = {\n    ").arg(timestamp).arg(RAM_PAGE_WIDTH * 8);

    for (int i = 0; i < RAM_PAGE_WIDTH * 8; ++i) {
        out << QString("0x%1, ").arg(buffer[i], 2, 16, QChar('0'));
        if ((i + 1) % 16 == 0 && i < RAM_PAGE_WIDTH * 8 - 1) {
            out << "\n    ";
        }
    }

#endif

#ifdef org1
    out << QString("const unsigned char image_%1[1024] = {\n    ").arg(timestamp);
    for (int i = 0; i < 1024; ++i) {
        out << QString("0x%1, ").arg(buffer[i], 2, 16, QChar('0'));
        if ((i + 1) % 16 == 0 && i < 1023) {
            out << "\n    ";
        }
    }
#endif
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


// 實現新的槽函數
void MainWindow::resetOledPlaceholderSize()
{
    if (m_oled) {

    // 1. 恢復原始縮放比例
    m_oled->setScale(6);


    // 2. 重置 QScrollArea 的滾軸位置到左上角 (0,0)
    scrollArea->horizontalScrollBar()->setValue(0);
    scrollArea->verticalScrollBar()->setValue(0);


    // 3. 觸發布局更新
    m_oled->updateGeometry();
    scrollArea->widget()->updateGeometry();
    scrollArea->updateGeometry();
    ui->oledPlaceholder->updateGeometry();

    }

}

void MainWindow::importImage()
{
    // 1. 打开一个档案选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择一张图片",
        "", // 预设目录
        "图片档案 (*.bmp *.jpg *.jpeg)"
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
    }

    // --- 关键步骤：图片处理和转换 ---
    // 将图片转换为128x64的单色图
    // Qt::MonoOnly -> dither抖动算法，效果比较好
    // Qt::Threshold -> 阈值算法，黑白分

    QImage scaledImage = image.scaled(128, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 4. 创建一个 128x64 的标准单色画布，并填充背景色（白色索引=0）
    QImage canvas(128, 64, QImage::Format_Mono);
    canvas.fill(0); // 0 = 白色
    // 5. 使用 QPainter 将缩放后的图片绘制到画布中心

    QPainter painter(&canvas);
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
    uint8_t buffer[RAM_PAGE_WIDTH *8] = {0};
    memset(buffer, 0x00, RAM_PAGE_WIDTH * 8); // 清空整個 SH1106 buffer

    // 遍历处理后的 monoImage 的每一个像素
    for (int y = 0; y < monoImage.height(); ++y) {
        for (int x = 0; x < monoImage.width(); ++x) {

            int colorIndex = monoImage.pixelIndex(x, y);

            bool pixelIsOn = (colorIndex == 1);

            if (pixelIsOn) {
                // 计算像素在 buffer 中的位置 (和 setPixel 逻辑一样)
                int page = y / 8;
                int bit_index = y % 8;
                //int byte_index = page * 128 + x;
                int byte_index = page * RAM_PAGE_WIDTH + (x + COLUMN_OFFSET);
                 /*
                 mainwindow.cpp:340:41: Use of undeclared identifier 'RAM_PAGE_WIDTH'
                 mainwindow.cpp:340:63: Use of undeclared identifier 'COLUMN_OFFSET'
                 */

                if (byte_index >= 0 && byte_index < 1024) { // 边界检查
                    buffer[byte_index] |= (1 << bit_index);
                }
            }
        }
    }
    // 4. 将转换好的 buffer 发送到 OLEDWidget 显示
    m_oled->setBuffer(buffer);
}

void MainWindow::on_pushButton_Copy_clicked()
{
      qDebug() << "[Copy按鈕] 被點擊了";
       m_oled->handleCopy();
      m_oled->showBufferDataDebug(); // ✅ 呼叫你的除錯視窗

      /*
    if (auto oled = qobject_cast<OLEDWidget*>(ui->oledPlaceholder)) {
        qDebug() << "[Copy按鈕] oledPlaceholder 類型:" << oled->metaObject()->className();
        oled->handleCopy();
    }else {
        qDebug() << "[Copy按鈕] qobject_cast 失敗";
    }*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

