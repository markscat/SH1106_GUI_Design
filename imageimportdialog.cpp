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
    ui->scaleSpinBox->setValue(1); // 預設為 1 倍
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
    if (m_originalImage.isNull()) return;

    // 1. 取得設定值
    int scale = ui->scaleSpinBox->value();
    int rotation = ui->rotationComboBox->currentData().toInt();

    // 2. 進行縮放
    // 使用 Qt::FastTransformation 讓預覽反應更快
    QImage scaledImage = m_originalImage.scaled(
        m_originalImage.width() * scale,
        m_originalImage.height() * scale,
        Qt::KeepAspectRatio,
        Qt::FastTransformation
        );

    // 3. 進行旋轉
    if (rotation != 0) {
        QTransform transform;
        transform.rotate(rotation);
        m_processedImage = scaledImage.transformed(transform);
    } else {
        m_processedImage = scaledImage;
    }

    // 4. 更新預覽 QLabel
    ui->previewLabel->setPixmap(QPixmap::fromImage(m_processedImage));
    ui->previewLabel->adjustSize(); // 根據 Pixmap 的大小調整 QLabel 的大小
}
