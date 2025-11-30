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

    // 修改 1: 移除 sourceImage 參數，改成預設建構子
    explicit ImageImportDialog(QWidget *parent = nullptr);

    // 如果想要保留舊功能，也可以用多載 (Overloading)，但為了簡化，我們先改掉
    //explicit ImageImportDialog(const QImage &sourceImage, QWidget *parent = nullptr);

    ~ImageImportDialog();


    // 提供一個 public 函數讓 MainWindow 取得最終處理好的圖片
    QImage getProcessedImage() const;
    bool isCoverMode()const;

private slots:
    // 當 spinbox 或 combobox 的值改變時，呼叫此槽函數來更新預覽
    void updatePreview();

    // 新增 2: 兩個新按鈕的槽函數
    void on_OpenPicture_pushButton_clicked(); // Tab 1 開圖
    void on_Openfile_pushButton_clicked();    // Tab 2 開檔

private:
    Ui::ImageImportDialog *ui;
    QImage m_originalImage;   // 保存未經處理的原始圖片
    QImage m_processedImage;  // 保存當前預覽/最終處理的圖片

    // Helper: 將 Hex 字串轉成預覽圖片 (模擬器核心)
    QImage hexToImage(const QString &hexString);

};

#endif // IMAGEIMPORTDIALOG_H
