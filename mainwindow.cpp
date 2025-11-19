#include <vector> // 使用 std::vector<bool> 更安全、更靈活
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "sample.h"
#include "oledwidget_Paint.h"
#include "ToolType.h"
#include "config.h"


//#define test_1029
#define org_1025

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    m_originalOledSize = ui->oledPlaceholder->size(); // 或 m_oled->sizeHint()

    // --- 建立並注入 OLEDWidget ---
    m_oled = new OLEDWidget(this);

    scrollArea = new QScrollArea(this);
    scrollArea->viewport()->installEventFilter(this); // 讓 MainWindow 監視 viewport 的事件


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

    connect(ui->undo_Bottom, &QPushButton::clicked,this, &MainWindow::onUndoClicked);

    connect(ui->redo_Bottom, &QPushButton::clicked,this, &MainWindow::onRedoClicked);

    // 关键：将粘贴按钮的点击信号连接到 OLEDWidget 的 handlePaste 槽函数

    //你的版本
    connect(ui->pushButton_paste, &QPushButton::clicked, m_oled, &OLEDWidget::handlePaste);

    //剪貼簿的版本
    //connect(ui->pushButton_paste, &QPushButton::clicked, m_oled, &OLEDWidget::handlePaste);
    //mainwindow.cpp:67:62: No member named 'oledWidget' in 'Ui::MainWindow'


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
/* Note:
    Tool_Select,// pushButton_Select,
    Tool_Copy,// pushButton_Copy,
    Tool_Cut, //pushButton_Cut,
    Tool_Paste//pushButton_paste
 */

    // 隱藏按鍵的方式
    //ui->pushButton_Select->hide();
    //或
    //ui->pushButton_Select->setVisible(false);

    m_toolButtonGroup->addButton(ui->pushButton_Select, Tool_Select);
    m_toolButtonGroup->addButton(ui->pushButton_Copy, Tool_Copy);
    m_toolButtonGroup->addButton(ui->pushButton_paste, Tool_Paste);
    m_toolButtonGroup->addButton(ui->pushButton_Cut, Tool_Cut);

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

    history.pushState(captureCanvasState()); // 初始快照
}

void MainWindow::exportData()
{

    // 步骤 1: [修正] 调用新的方法来获取数据
    // getHardwareBuffer() 返回一个 std::vector<uint8_t> 对象。
    // 我们用一个新的同类型变量 buffer 来接收它。
    std::vector<uint8_t> buffer = m_oled->getHardwareBuffer();

    // 步骤 2: [新增] 健壮性检查
    // 在进行操作前，最好先检查一下 buffer 是否为空。
    if (buffer.empty()) {
        QMessageBox::information(this, "提示", "目前没有可汇出的数据。");
        return;
    }

    // 步骤 3: [修正] 动态地构建 C 阵列声明
    // 我们不再硬编码数组大小 1024，而是使用 buffer.size() 来获取实际大小。
    QString c_array = QString("const unsigned char screen_data[%1] = {\n    ")
                          .arg(buffer.size());

    int line_break_count = 0;


    // 步骤 4: [修正] 使用 std::vector 的方式来遍历数据
    for (size_t i = 0; i < buffer.size(); ++i) {
        // buffer[i] 现在是对 std::vector 元素的访问，语义清晰。
        c_array += QString("0x%1,").arg(buffer[i], 2, 16, QChar('0'));

        line_break_count++;
        if (line_break_count == 16 && i < buffer.size() - 1) { // 避免在最后一行后也换行
            c_array += "\n    ";
            line_break_count = 0;
        }
    }

    // 步骤 5: [修正] 更健壮的字符串结尾处理
    if (c_array.endsWith(", ")) {
        c_array.chop(2); // 移除最后多余的 ", "
    }
    c_array += "\n};";

    // 步骤 6: 显示对话框 (这部分逻辑保持不变)
    QDialog *exportDialog = new QDialog(this);
    exportDialog->setWindowTitle("汇出的 C 阵列 (可複製)");
    exportDialog->setAttribute(Qt::WA_DeleteOnClose); // 使用 WA_DeleteOnClose 更安全
    exportDialog->resize(600, 400);

    QTextEdit *textEdit = new QTextEdit(exportDialog);
    textEdit->setPlainText(c_array);
    textEdit->setReadOnly(true);
    textEdit->setFontFamily("Courier");

    QPushButton *closeButton = new QPushButton("關閉", exportDialog);
    connect(closeButton, &QPushButton::clicked, exportDialog, &QDialog::accept);

    QVBoxLayout *layout = new QVBoxLayout(exportDialog);
    layout->addWidget(textEdit);
    layout->addWidget(closeButton);

    exportDialog->exec();
    // 使用 WA_DeleteOnClose 后，不再需要手动 delete
}

void MainWindow::saveData()
{
    // 步骤 1: 创建 log 文件夹 (这部分逻辑不变)
    QDir dir(QApplication::applicationDirPath());
    if (!dir.exists("log")) {
        if (!dir.mkdir("log")) {
            QMessageBox::critical(this, "错误", "无法建立 log 资料夹！");
            return;
        }
    }
    dir.cd("log");

    // 步骤 2: 产生档案名称 (这部分逻辑不变)
    QString timestamp = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
    QString fileName = timestamp + ".c";
    QString filePath = dir.absoluteFilePath(fileName);

    // 步骤 3: [修正] 准备要写入的内容

    // 3.1 调用新的方法获取数据
    std::vector<uint8_t> buffer = m_oled->getHardwareBuffer();

    // 3.2 健壮性检查
    if (buffer.empty()) {
        QMessageBox::information(this, "提示", "没有可储存的数据。");
        return;
    }

    // 3.3 使用 QTextStream 构建文件内容字符串
    QString c_array_content;
    QTextStream out(&c_array_content);

    out << "// Saved at: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    out << "// File generated by OLED GUI Designer\n\n";
    // out << "#include <stdint.h>\n\n"; // 包含标准头文件是个好习惯

    // 3.4 [修正] 动态地构建 C 阵列
    out << QString("const unsigned char image_%1[%2] = {\n    ")
               .arg(timestamp)
               .arg(buffer.size());

    // 3.5 [修正] 使用 std::vector 的方式遍历
    for (size_t i = 0; i < buffer.size(); ++i) {
        out << QString("0x%1, ").arg(buffer[i], 2, 16, QChar('0'));
        if ((i + 1) % 16 == 0 && i < buffer.size() - 1) {
            out << "\n    ";
        }
    }

    // 移除最后多余的 ", "
    // (注意: QTextStream 不像 QString 那样有 chop 或 endsWith,
    // 所以先构建 QString 再处理末尾是更稳妥的方式)
    QString final_content = out.readAll(); // 从流中读出所有内容
    if (final_content.endsWith(", ")) {
        final_content.chop(2);
    }
    final_content += "\n};";

    // 步骤 4: 写入档案 (这部分逻辑不变)
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << final_content; // 写入处理过的最终字符串
        file.close();
        QMessageBox::information(this, "成功", QString("档案已储存至:\n%1").arg(filePath));
    } else {
        QMessageBox::critical(this, "错误", QString("无法写入档案:\n%1").arg(filePath));
    }
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

// 實現新的槽函數
void MainWindow::resetOledPlaceholderSize()
{
    // 安全检查，确保 m_oled 和 scrollArea 指针都有效
    if (m_oled && scrollArea) {

        // 步骤 1: 将 OLED widget 的缩放比例重置为一个默认值 (例如 6)
        // setScale 内部会处理 widget 尺寸的更新和重绘请求。
        m_oled->setScale(6);

        // 步骤 2: 将 QScrollArea 的滚动条位置都重置回原点 (左上角)
        scrollArea->horizontalScrollBar()->setValue(0);
        scrollArea->verticalScrollBar()->setValue(0);

        // [移除] 不再需要手动调用 updateGeometry()。
        // setScale() 导致的尺寸变化，会被 QScrollArea 自动侦测到，
        // Qt 的布局系统会负责处理剩下的更新。
    }

    //for undo and redo
    history.pushState(captureCanvasState());
}



void MainWindow::importImage()
{
    // 步骤 1: 打开档案选择对话框 (逻辑不变)
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择一张图片",
        "",
        "图片档案 (*.bmp *.jpg *.jpeg )" // 可以加上 png
        );

    if (filePath.isEmpty()) return;

    // 步骤 2: 读取图片 (逻辑不变)
    QImage originalImage;
    if (!originalImage.load(filePath)) {
        QMessageBox::warning(this, "错误", "无法载入图片档案！");
        return;
    }

    /*因為本程式主要是為了小尺寸的螢幕而設計,所以會限定大小
     * 後續會把限定的長寬比放寬,與其說放寬,不如說縮限,
     * 因為如果要使用2吋以上的彩色螢幕還帶觸摸屏的話,請去用其他更專業的工具,例如LVGL、TouchGFX、emWin
     * 或是花錢去用Qt for MCUs
     */
    if (originalImage.width() > 128 || originalImage.height() > 64) {
        QMessageBox::critical(
            this,
            "匯入錯誤：尺寸過大",
            QString("圖片尺寸不得超過 128x64 像素。\n\n您選擇的圖片尺寸為: %1x%2")
                .arg(originalImage.width())
                .arg(originalImage.height())
            );
        return; // 不符合條件，直接返回主視窗
    }

    // 驗證 3b: 檢查是否為彩色圖片
    // QImage::isGrayscale() 會對純黑白圖片也返回 true，所以這個檢查就夠了。
    // 如果一張圖片不是灰階的，那它一定是彩色的。
    if (!originalImage.isGrayscale()) {
        QMessageBox::critical(
            this,
            "匯入錯誤：格式不符",
            "只允許匯入黑白或灰階圖片。\n\n您選擇的圖片是彩色圖片。"
            );
        return; // 不符合條件，直接返回主視窗
    }

    // 步骤 3: [新流程] 弹出我们的自定义对话框，让用户进行设置
    ImageImportDialog importDialog(originalImage, this);


    if (importDialog.exec() == QDialog::Accepted) {
        QImage processedImage = importDialog.getProcessedImage();
        QImage monoImage = processedImage.convertToFormat(QImage::Format_Mono, Qt::ThresholdDither);

        m_oled->updateOledFromImage(monoImage);

    }
    //for undo and redo
    history.pushState(captureCanvasState());
}

void MainWindow::on_pushButton_Copy_clicked()
{
    qDebug() << "[Copy按鈕] 被點擊了";
    m_oled->handleCopy();
    m_oled->showBufferDataAsHeader();
}


void MainWindow::on_pushButton_paste_clicked()
{
    qDebug() << "[Paste按鈕] 被點擊了";
    if (m_oled) {
        m_oled->commitPaste();
    }
    //for undo and redo
    history.pushState(captureCanvasState());
}

void MainWindow::on_pushButton_Cut_clicked()
{
    qDebug() << "[Cut按鈕] 被點擊了";
    if (m_oled) {
        m_oled->handleCut();
    }
    //for undo and redo
    history.pushState(captureCanvasState());
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    static bool isForwardingEvent = false;

    if (isForwardingEvent) {
        return false; // 如果我們正在轉發事件，就不要處理任何新事件
    }

    if (obj == scrollArea->viewport()) {
        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            // 1. 獲取全域浮點數座標 (QPointF)
            const QPointF globalPosF = mouseEvent->globalPosition();

            // 2. 將全域浮點數座標映射到 m_oled 的座標系中
            //    結果也是一個浮點數座標 (QPointF)
            const QPointF oledPosF = m_oled->mapFromGlobal(globalPosF);

            // 3. 【關鍵修正】明確地將浮點數座標轉換為整數座標
            const QPoint oledPos = oledPosF.toPoint();

            // 4. 建立新的滑鼠事件

            QMouseEvent newEvent( // 注意：這裡沒有 *，newEvent 是一個物件，不是指標
                mouseEvent->type(),
                oledPos,
                globalPosF.toPoint(),
                mouseEvent->button(),
                mouseEvent->buttons(),
                mouseEvent->modifiers()
                );

            // 【關鍵修正】傳遞物件的位址給 sendEvent

             isForwardingEvent = true;
            QApplication::sendEvent(m_oled, &newEvent);

            isForwardingEvent = false;
            // 6. 處理完畢
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}



void MainWindow::onUndoClicked() {
    if (history.canUndo()) {
        QByteArray state = history.undo();
        applyCanvasState(state);
    }
}

void MainWindow::onRedoClicked() {
    if (history.canRedo()) {
        QByteArray state = history.redo();
        applyCanvasState(state);
    }
}


void MainWindow::applyCanvasState(const QByteArray& state) {
    if (state.isEmpty()) return;
    m_oled->setBuffer(reinterpret_cast<const uint8_t*>(state.constData()));

    /* m_oled->setBuffer(reinterpret_cast<const uint8_t*>(state.constData()));
     * 等同於：
     * const char* rawData = state.constData();
     * const uint8_t* buffer = reinterpret_cast<const uint8_t*>(rawData);
     * m_oled->setBuffer(buffer);
     */

    //update(); // 觸發 Qt 重繪
}


QByteArray MainWindow::captureCanvasState() {
    std::vector<uint8_t> buffer = m_oled->getHardwareBuffer();
    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

MainWindow::~MainWindow()
{
    delete ui;
}

