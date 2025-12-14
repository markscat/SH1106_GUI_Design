


#include "imageimportdialog.h"
#include "ui_imageimportdialog.h"

ImageImportDialog::ImageImportDialog(const QImage &sourceImage, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageImportDialog),
    m_originalImage(sourceImage) // 保存原始圖片
{
    ui->setupUi(this);
    this->setWindowTitle("匯入精靈");
    ui->tabWidget->setCurrentIndex(0);



    // --- 初始化 UI 元件 ---
    // 1. 設定圖片 Tab 的初始狀態
    if (!m_originalImage.isNull()) {
        ui->label_ImgSize->setText(QString("原始尺寸: %1 x %2").arg(m_originalImage.width()).arg(m_originalImage.height()));
    } else {
        ui->label_ImgSize->setText("尚未載入圖片");
    }

    // 1. 顯示原始圖片尺寸
    //ui->label_ImgSize->setText(QString("原始尺寸: %1 x %2").arg(m_originalImage.width()).arg(m_originalImage.height()));

    // 2. 設定放大倍率選項 (例如 1 到 8 倍)
    ui->scaleSpinBox->setRange(1, 8);
    ui->scaleSpinBox->setValue(1); // 設定一個合理的預設值
    ui->scaleSpinBox->setSuffix(" 倍");

    // 3. 設定旋轉角度選項
    ui->rotationComboBox->addItem("0°", 0);
    ui->rotationComboBox->addItem("90° (順時針)", 90);
    ui->rotationComboBox->addItem("180°", 180);
    ui->rotationComboBox->addItem("270° (逆時針)", 270); // 或 -90

    // 4. 讓預覽圖的 QLabel 可以自動縮放內容
    ui->previewLabel->setScaledContents(false); // 我們手動設定大小，不讓 QLabel 自動縮放
    ui->previewLabel->setAlignment(Qt::AlignCenter);


    // 檔案 Tab 的預覽 Label
    if (ui->label_FilePreview) { // 加上判斷防止崩潰
        ui->label_FilePreview->setScaledContents(false);
        ui->label_FilePreview->setAlignment(Qt::AlignCenter);
    }


    // --- 連接信號與槽 ---
    connect(ui->scaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageImportDialog::updatePreview);
    connect(ui->rotationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageImportDialog::updatePreview);
    connect(ui->B_W_swap, &QRadioButton::toggled, this, &ImageImportDialog::updatePreview);

    // [新增] 按鈕連接 (如果 UI 編輯器沒有自動連接 on_..._clicked 的話，手動連比較保險)
    //connect(ui->OpenPicture_pushButton, &QPushButton::clicked, this, &ImageImportDialog::on_OpenPicture_pushButton_clicked);
    //connect(ui->Openfile_pushButton, &QPushButton::clicked, this, &ImageImportDialog::on_Openfile_pushButton_clicked);

    // --- 第一次載入時，立即更新一次預覽 ---
    updatePreview();
}

ImageImportDialog::~ImageImportDialog()
{
    delete ui;
}


// 這個 public 函數是關鍵，讓 MainWindow 可以拿到結果
QImage ImageImportDialog::getProcessedImage() const
{
    // 判斷目前使用者停留在哪個分頁
    if (ui->tabWidget->currentWidget() == ui->File_tab) {
        // 如果是檔案分頁，回傳解析出來的圖
        return m_fileRawImage;
    } else {
        // 否則回傳圖片分頁處理過的圖
        return m_processedImage;
    }
    //return m_processedImage;
}


// ================= Picture Tab 邏輯 =================

// 核心邏輯：根據 UI 設定來產生預覽圖
void ImageImportDialog::on_OpenPicture_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "選擇圖片", "", "Images (*.jpg *.jpeg *.bmp *.xpm)");

    if (filePath.isEmpty()) return;
    QImage loaded;

    if (loaded.load(filePath)) {
        // 檢查尺寸 (選擇性)
        if (loaded.width() > 128 || loaded.height() > 64) {
            QMessageBox::warning(this, "注意", "圖片尺寸大於 128x64，匯入後可能需要縮小。");
        }
        m_originalImage = loaded;
        ui->label_ImgSize->setText(QString("原始尺寸: %1 x %2").arg(m_originalImage.width()).arg(m_originalImage.height()));
        updatePreview(); // 更新預覽
    }
}

void ImageImportDialog::updatePreview(){

    if (m_originalImage.isNull()) {
        ui->previewLabel->setText("請先開啟圖片");
        //qDebug() << "Warning: m_originalImage is null. import factor might be zero.";
        return;
    }

    // 1. 取得設定值
    int scaleFactor = ui->scaleSpinBox->value();
    int rotation = ui->rotationComboBox->currentData().toInt();
    bool shouldInvert = ui->B_W_swap->isChecked();


    // 2. 建立一個局部的、用於處理的圖片副本
    //    我們從原始圖片開始，確保每次更新都是全新的計算
    QImage processedImage = m_originalImage;
    //QImage temp = m_originalImage;


    // 3. 【流水線步驟 A: 縮放】
    //    注意：SpinBox 的 value() 通常是百分比，所以要除以 100.0
    //    如果您的 SpinBox 直接是像素值，則不需要除法


    int targetWidth = m_originalImage.width() * scaleFactor;
    int targetHeight  = m_originalImage.height() * scaleFactor;
    //processedImage = processedImage.scaled(targetWidth, targetHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    processedImage = processedImage.scaled(
        targetWidth,
        targetHeight,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    if (processedImage.isNull()) {
        // 如果縮放失敗（例如 scale 為 0），就不要繼續執行
        return;
    }

    // 4. 【流水線步驟 B: 旋轉】
    if (rotation != 0) {
        QTransform transform;
        transform.rotate(rotation);
        processedImage = processedImage.transformed(transform);
    }

    if (processedImage.isNull()) {
        // 如果縮放失敗（例如 scale 為 0），就不要繼續執行
        return;
    }


    // 5. 【流水線步驟 C: 反轉】
    if (shouldInvert) {
        // 現在 processedImage 已經包含了縮放和旋轉的結果
        // 我們在此基礎上進行反轉
        processedImage.invertPixels(QImage::InvertRgb);
    }

    if (processedImage.isNull()) {
        // 如果縮放失敗（例如 scale 為 0），就不要繼續執行
        return;
    }


    // 6. 【流水線步驟 D: 轉換為單色圖】
    //    這是流水線的最後一步，得到最終的單色邏輯資料
    QImage monoImage = processedImage.convertToFormat(QImage::Format_Mono, Qt::ThresholdDither);

    if (monoImage.isNull()) {
        // 如果縮放失敗（例如 scale 為 0），就不要繼續執行
        return;
    }

    /*  這段就有差別,

    // 3. 顯示預覽
    // 為了讓使用者看得清楚，預覽圖本身可以在 Label 上放大顯示
    int displayScale = (scaleFactor < 2) ? 4 : 2;
    QPixmap px = QPixmap::fromImage(mono);
    // 這裡只是顯示放大，不改變 mono 本身數據
    ui->previewLabel->setPixmap(px.scaled(px.size() * displayScale, Qt::KeepAspectRatio));
    ui->previewLabel->resize(px.size() * displayScale);
    */


    // 7. 更新 UI 預覽
    //    用最終的 monoImage 來更新預覽

    // 讓 Label 跟著圖片大小改變
    int previewWidth  = monoImage.width() * scaleFactor;
    int previewHeight = monoImage.height() * scaleFactor;

    QPixmap pixmap = QPixmap::fromImage(
        monoImage.scaled(previewWidth, previewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );

    ui->previewLabel->setPixmap(pixmap);
    ui->previewLabel->resize(pixmap.size());  // 讓 Label 跟著圖片大小改變


    // 8. 將最終結果儲存到成員變數中，供 MainWindow 讀取
    //    這一步非常重要，確保 getProcessedImage() 返回正確的結果
    m_processedImage = monoImage;

}


void ImageImportDialog::on_Openfile_pushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "選擇標頭檔/原始碼",
        "",
        "C/C++ Header (*.h *.c *.cpp *.txt);;All Files (*)"
        );

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "錯誤", "無法開啟檔案！");
        return;
    }

    // 顯示路徑
    ui->lbl_FilePath->setText(filePath);

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 開始解析
    parseFileContentToImage(content);
}

void ImageImportDialog::parseFileContentToImage(const QString &content)
{
    // 1. 使用 Regex 提取 Hex
    QString dataContent = content;
    int startBrace = content.indexOf('{');
    int endBrace = content.lastIndexOf('}');

    // 如果找得到大括號，我們就只看大括號裡面的東西
    if (startBrace != -1 && endBrace != -1 && endBrace > startBrace) {
        dataContent = content.mid(startBrace, endBrace - startBrace + 1);
    }

    // --- 2. 嘗試解析尺寸 (這是新增的關鍵功能) ---
    // 目標：從註解中抓出 "(22x8 region" 這樣的資訊
    // Regex 解說：尋找括號，接著數字(寬)，接著x，接著數字(高)
    QRegularExpression sizeRegex("\\((\\d+)x(\\d+) region");
    auto sizeMatch = sizeRegex.match(content);

    int targetW = 0;
    int targetH = 0;

    if (sizeMatch.hasMatch()) {
        targetW = sizeMatch.captured(1).toInt();
        targetH = sizeMatch.captured(2).toInt();
    }

    //QRegularExpression hexRegex("0x[0-9a-fA-F]+|[0-9a-fA-F]{2}");
    QRegularExpression hexRegex("0[xX][0-9a-fA-F]+");
    auto matches = hexRegex.globalMatch(dataContent);

    std::vector<uint8_t> rawBuffer;
    while (matches.hasNext()) {
        auto match = matches.next();
        bool ok;
        int val = match.captured().toInt(&ok, 16);
        if (ok) rawBuffer.push_back(static_cast<uint8_t>(val));
    }

    if (rawBuffer.empty()) {
        ui->label_FilePreview->setText("未找到 Hex 數據");
        m_fileRawImage = QImage();
        return;
    }

    if (targetW == 0 || targetH == 0) {
        // [備案]: 如果 buffer 很小 (小於 128)，假設它是單行圖示 (高=8)
        if (rawBuffer.size() < 128) {
            targetW = rawBuffer.size();
            targetH = 8;
        } else {
            // 否則假設它是全寬度 128
            targetW = 128;
            targetH = (rawBuffer.size() + 127) / 128 * 8; // 向上取整的頁數
        }
    }

    // 建立一張「剛好大小」的圖片，而不是全螢幕
    QImage importImg(targetW, targetH, QImage::Format_Mono);
    importImg.fill(0);
    importImg.setColor(0, qRgb(0, 0, 0));
    importImg.setColor(1, qRgb(255, 255, 255));


    int p = 0;
    // 根據目標高度計算頁數 (每 8 pixel 一頁)
    int pages = (targetH + 7) / 8;

    for (int page = 0; page < pages; ++page) {
        for (int col = 0; col < targetW; ++col) {

            if (p >= rawBuffer.size()) break;

            uint8_t byte = rawBuffer[p++];

            for (int bit = 0; bit < 8; ++bit) {
                int y = page * 8 + bit;
                // 確保不要畫出界 (例如高度是 12，第二頁只能畫前 4 bit)
                if (y < targetH) {
                    if ((byte >> bit) & 1) {
                        importImg.setPixel(col, y, 1);
                    }
                }
            }
        }
    }

    // 4. 儲存與預覽
   // m_fileRawImage = result;

    m_fileRawImage =importImg;
    // 預覽放大 3 倍比較容易看
    QPixmap px = QPixmap::fromImage(importImg);

    // 如果圖片很小，放大顯示倍率可以設大一點 (例如 4 倍)
    //int scale = (targetW < 64) ? 4 : 2;

    ui->label_FilePreview->setPixmap(px.scaled(px.size() * 3, Qt::KeepAspectRatio));
    ui->label_FilePreview->resize(px.size() * 3);
}
/*
void ImageImportDialog::parseAndPreviewFile(const QString &fileContent)
{
    std::vector<uint8_t> buffer;

    // 1. 使用 Regex 提取 Hex 數值 (邏輯參考 SimulatorDialog)
    QRegularExpression hexRegex("0x[0-9a-fA-F]+|[0-9a-fA-F]{2}");
    auto matches = hexRegex.globalMatch(fileContent);

    while (matches.hasNext()) {
        auto match = matches.next();
        bool ok;
        int val = match.captured().toInt(&ok, 16);
        if (ok) {
            buffer.push_back(static_cast<uint8_t>(val));
        }
    }

    if (buffer.empty()) {
        ui->label_FilePreview->setText("未偵測到有效的 Hex 數據");
        m_fileImportImage = QImage(); // 清空
        return;
    }

    // 2. 檢查大小 (128x64 的 OLED 需要 1024 bytes)
    const size_t requiredSize = OledConfig::DISPLAY_WIDTH * OledConfig::DISPLAY_HEIGHT / 8;

    if (buffer.size() < requiredSize) {
        // 資料不足補 0
        buffer.resize(requiredSize, 0x00);
    } else if (buffer.size() > requiredSize) {
        // 資料過多截斷 (通常只取前 1024 bytes)
        buffer.resize(requiredSize);
    }

    // 3. 利用 OledDataModel 將 Hardware Buffer 轉回 QImage
    // 這裡我們建立一個暫時的 Model 來幫我們做苦工
    OledDataModel tempModel;
    tempModel.setFromHardwareBuffer(buffer.data());

    // 轉換為邏輯圖像 (Format_Mono)
    QImage resultImage = tempModel.copyRegionToLogicalFormat(
        QRect(0, 0, OledConfig::DISPLAY_WIDTH, OledConfig::DISPLAY_HEIGHT)
        );

    // 4. 存檔與預覽
    m_fileImportImage = resultImage;

    // 顯示預覽 (放大顯示比較清楚，例如放大 2 倍)
    int previewScale = 2;
    QPixmap pixmap = QPixmap::fromImage(resultImage.scaled(
        resultImage.width() * previewScale,
        resultImage.height() * previewScale,
        Qt::KeepAspectRatio
        ));

    ui->label_FilePreview->setPixmap(pixmap);
    ui->label_FilePreview->resize(pixmap.size());
}
*/

bool ImageImportDialog::isCoverMode() const {
    return ui->C_O_swap->isChecked();
}

