/*
 *  **************************************************************************
 Project name    : SH1106_GUI_Design
 @file           : main.cpp
 @brief          : Main program body
 @Designer       : Ethan Yang and AI
 ******************************************************************************
 * 本程式主要是用來提供設計Sh1106系列 3.1吋（或以下）的UI設計程式
 *　很多玩阿都諾的竹……設計者，都會用網路上其他的工具。
 *　我是不知道那些本……設計者心裡在想什麼，我只知道當我灌好Qt之後，就一直想要找個題目來寫一個程式出來。
 *
 *　其實這個程式的風格和我之前C#的專案BMP2Array有異曲同工，但那個程式專住在轉換檔案以及製作字型.ｈ上
 *  並沒有真正規劃到『怎麼在螢幕上設計相對應的GUI*
 *
 *
 * V25.10.17
 *
 * 終於好多了.
 * 試著玩了一下，會有幾個問題：
 * 第一：沒辦法縮放
 * 因為我會習慣用ctrl+滑鼠滾輪把螢幕放大,這樣看起來比較輕鬆.
 * 縮放有兩種：
 *
 * 一種是工作區的放大。
 * 另一種是當我放大視窗的時候，工作區最好也跟著一起放大；但依舊必須局現在128*64的範圍
 *
 * 10/18
 *  是可以放大縮小,旁邊也有卷軸,但現在滑鼠右鍵無法動作,取消已經點選的那一點.
 * 10/24
 * 匯入圖片不支援png圖片
 * 畫線，畫點，圓和方都沒問題了
 *
 * 第二：
 *  * 橡皮擦
 *  * 10/24
 * 這個功能被筆刷大小給取代了.
 *
 * 第三：
 * ｕｎｄｏ(Ctrl+Z / Ctrl+Y)
 *
 * 第四：
 * 筆刷放大後,滑鼠右鍵拖曳就可以當成橡皮擦了
 * 工具區好難看……
 * 一般的是有個icon,在那邊然後飄出相對應的文字說明
 *
 * 第五:
 * 印章工具；我想做一個類似印章的工具，先用比較常見的七段顯示器的當作印章
 * 我點選了這個按鍵，然後跳出一個框來，可以選七段顯示器的線條粗細，整體大小，還有風格（斜體，還是米字），
 *
 *
 *
 *
 *版權：
 *GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 * Copyright (C) [2025] [Ethan]
 * 本程式是一個自由軟體：您可以依照 **GNU 通用公共授權條款（GPL）** 發佈和/或修改，
 * GPL 版本 3 或（依您選擇）任何更新版本。
 *
 * 本程式的發佈目的是希望它對您有幫助，但 **不提供任何擔保**，甚至不包含適銷性或特定用途適用性的默示擔保。
 * 請參閱 **GNU 通用公共授權條款** 以獲取更多詳細資訊。
 * 您應當已經收到一份 **GNU 通用公共授權條款** 副本。
 * 如果沒有，請參閱 <https://www.gnu.org/licenses/gpl-3.0.html>。
 *

*/

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QTimer>  // 添加這行
#include "sample.h"
#include "oledwidget.h"
#include "ToolType.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    // 1. 先設置視窗和容器
    resize(1024, 600);
    QList<int> initialSizes;
    initialSizes << 120 << 680;

    // 設置 Splitter 初始尺寸
    ui->splitter->setSizes(initialSizes);

   // 3. 創建滾動區域和 OLEDWidget
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(false); // 保持原始比例，不自動拉伸
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("background: transparent;");//不用嗎？

    // 2. 設置佈局（先建立容器結構）
    QVBoxLayout *layout = new QVBoxLayout(ui->oledPlaceholder);
    layout->addWidget(scrollArea);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);


    // --- 建立並注入 OLEDWidget ---
    m_oled = new OLEDWidget(this);
    scrollArea->setWidget(m_oled);


    // 6. 延遲設置尺寸（現在容器環境正確了）
    QTimer::singleShot(0, this, [this]() {
        m_oled->setScale(6);
        // 現在再獲取正確的原始尺寸
        m_originalOledSize = ui->oledPlaceholder->size();
    });



    // --- 3. 設定【繪圖工具】按鈕群組 ---`
    m_toolButtonGroup = new QButtonGroup(this);
    m_toolButtonGroup->setExclusive(true);


    // 假設您的按鈕 objectName 分別是 penButton, lineButton 等
    m_toolButtonGroup->addButton(ui->ToolPen, Tool_Pen);
    m_toolButtonGroup->addButton(ui->ToolLine, Tool_Line);
    m_toolButtonGroup->addButton(ui->ToolRectangle, Tool_Rectangle);
    m_toolButtonGroup->addButton(ui->ToolFilledRectangle, Tool_FilledRectangle);
    m_toolButtonGroup->addButton(ui->ToolCircle, Tool_Circle);


    // 設置所有按鈕為可選中狀態
    const QList<QAbstractButton*> &buttons = m_toolButtonGroup->buttons();
    for (QAbstractButton *button : buttons) {
        button->setCheckable(true);
    }

    // 設定預設工具
    ui->ToolPen->setChecked(true);

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


    // 在這個 lambda 函式中，我們將接收到的整數 ID
    // 轉換回 ToolType enum，然後呼叫 OLEDWidget 的方法來設定當前工具。
    connect(m_toolButtonGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, [this](int id) {
                m_oled->setCurrentTool(static_cast<ToolType>(id));
            });

    layout->setContentsMargins(0, 0, 0, 0);


    // --- 6. 載入預設內容 ---
    m_oled->setBuffer(sample_image);

    // ↓↓↓↓ 在这里加入以下代码来设置 Splitter 的初始尺寸 ↓↓↓↓


    QTimer::singleShot(100, this, [this]() {
        qDebug() << "oledPlaceholder 尺寸:" << ui->oledPlaceholder->size();
        qDebug() << "ScrollArea 尺寸:" << scrollArea->size();
        qDebug() << "OLEDWidget 尺寸:" << m_oled->size();

        // 檢查佈局拉伸因子
        qDebug() << "JobAreaLayout 拉伸因子:" << ui->JobAreaLayout->stretch(0) << ui->JobAreaLayout->stretch(1);
    });

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
/*
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
    */
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
    //QString content = QString("const unsigned char image_%1[] = {\n    ").arg(timestamp);
    // ... (这里是和 exportData 中一样的循环，用来生成 C 阵列字串) ...
    // ... 为了简洁，省略重复代码 ...

    QString c_array_content;
    QTextStream out(&c_array_content);
    out << "// Saved at: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    //out << QString("const unsigned char image_%1[%2] = {\n    ").arg(timestamp).arg(RAM_PAGE_WIDTH * 8);


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

