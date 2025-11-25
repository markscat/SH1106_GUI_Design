#ifndef IMAGEIMPORTDIALOG_H
#define IMAGEIMPORTDIALOG_H
#include "config.h"


namespace Ui {
class ImageImportDialog;
}

class ImageImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageImportDialog(const QImage &sourceImage, QWidget *parent = nullptr);
    ~ImageImportDialog();


    // 提供一個 public 函數讓 MainWindow 取得最終處理好的圖片
    QImage getProcessedImage() const;
    bool isCoverMode()const;

private slots:
    // 當 spinbox 或 combobox 的值改變時，呼叫此槽函數來更新預覽
    void updatePreview();

private:
    Ui::ImageImportDialog *ui;
    QImage m_originalImage;   // 保存未經處理的原始圖片
    QImage m_processedImage;  // 保存當前預覽/最終處理的圖片

};

#endif // IMAGEIMPORTDIALOG_H
