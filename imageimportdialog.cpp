#include "imageimportdialog.h"
#include "ui_imageimportdialog.h"

ImageImportDialog::ImageImportDialog(const QImage &sourceImage, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageImportDialog),
    m_originalImage(sourceImage) // 保存原始圖片
{
    ui->setupUi(this);
    this->setWindowTitle("圖片匯入選項");

    // --- 初始化 UI 元件 ---

    // 1. 顯示原始圖片尺寸
    ui->label_ImgSize->setText(QString("原始尺寸: %1 x %2").arg(m_originalImage.width()).arg(m_originalImage.height()));

    // 2. 設定放大倍率選項 (例如 1 到 8 倍)
    ui->scaleSpinBox->setRange(1, 8);
    ui->scaleSpinBox->setValue(1); // 設定一個合理的預設值，例如 100%
    ui->scaleSpinBox->setSuffix(" 倍");

    // 3. 設定旋轉角度選項
    ui->rotationComboBox->addItem("0°", 0);
    ui->rotationComboBox->addItem("90° (順時針)", 90);
    ui->rotationComboBox->addItem("180°", 180);
    ui->rotationComboBox->addItem("270° (逆時針)", 270); // 或 -90

    // 4. 讓預覽圖的 QLabel 可以自動縮放內容
    ui->previewLabel->setScaledContents(false); // 我們手動設定大小，不讓 QLabel 自動縮放
    ui->previewLabel->setAlignment(Qt::AlignCenter);

    // --- 連接信號與槽 ---
    connect(ui->scaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ImageImportDialog::updatePreview);
    connect(ui->rotationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageImportDialog::updatePreview);

    connect(ui->B_W_swap, &QRadioButton::toggled, this, &ImageImportDialog::updatePreview);

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
    return m_processedImage;
}

// 核心邏輯：根據 UI 設定來產生預覽圖
void ImageImportDialog::updatePreview()
{
    if (m_originalImage.isNull()) {
        qDebug() << "Warning: m_originalImage is null. import factor might be zero.";
        return;
    }

    // 1. 取得設定值
    int scaleFactor = ui->scaleSpinBox->value();
    int rotation = ui->rotationComboBox->currentData().toInt();
    bool shouldInvert = ui->B_W_swap->isChecked();
    //int scale = ui->scaleSpinBox->value();
    //int rotation = ui->rotationComboBox->currentData().toInt();

    // 2. 建立一個局部的、用於處理的圖片副本
    //    我們從原始圖片開始，確保每次更新都是全新的計算
    QImage processedImage = m_originalImage;

    // 3. 【流水線步驟 A: 縮放】
    //    注意：SpinBox 的 value() 通常是百分比，所以要除以 100.0
    //    如果您的 SpinBox 直接是像素值，則不需要除法


    int targetWidth = m_originalImage.width() * scaleFactor;
    int targetHeight  = m_originalImage.height() * scaleFactor;


    qDebug() << "targetWidth:" << targetWidth;
    qDebug() << "targetHeight:" << targetHeight;

    processedImage = processedImage.scaled(
        targetWidth,
        targetHeight,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    qDebug() << "targetWidth:" << targetWidth;
    qDebug() << "targetHeight:" << targetHeight;


    if (processedImage.isNull()) {
        // 如果縮放失敗（例如 scale 為 0），就不要繼續執行
        // 可以選擇在這裡印出除錯訊息
        qDebug() << "Warning: scale Image is null. Scale factor might be zero.";
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
        // 可以選擇在這裡印出除錯訊息
        qDebug() << "Warning: rotate Image is null. Scale factor might be zero.";
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
        // 可以選擇在這裡印出除錯訊息
        qDebug() << "Warning: invertPixels is null. Scale factor might be zero.";
        return;
    }


    // 6. 【流水線步驟 D: 轉換為單色圖】
    //    這是流水線的最後一步，得到最終的單色邏輯資料
    QImage monoImage = processedImage.convertToFormat(QImage::Format_Mono, Qt::ThresholdDither);

    if (monoImage.isNull()) {
        // 如果縮放失敗（例如 scale 為 0），就不要繼續執行
        // 可以選擇在這裡印出除錯訊息
        qDebug() << "Warning: Finel convertToFormat is null. Scale factor might be zero.";
        return;
    }



    // 7. 更新 UI 預覽
    //    用最終的 monoImage 來更新預覽
    ui->previewLabel->setPixmap(QPixmap::fromImage(monoImage.scaled(
        ui->previewLabel->size(),
        Qt::KeepAspectRatio,
        Qt::FastTransformation
        )));

    // 8. 將最終結果儲存到成員變數中，供 MainWindow 讀取
    //    這一步非常重要，確保 getProcessedImage() 返回正確的結果
    m_processedImage = monoImage;

}


