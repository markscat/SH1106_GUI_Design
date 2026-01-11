#ifndef IMAGEIMPORTDIALOG_H
#define IMAGEIMPORTDIALOG_H
#include "config.h"
#include "oled_datamodel.h"



namespace Ui {
class ImageImportDialog;
}

class ImageImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageImportDialog(const QImage &sourceImage = QImage(), QWidget *parent = nullptr);

    //explicit ImageImportDialog(const QImage &sourceImage, QWidget *parent = nullptr);
    ~ImageImportDialog();


    // 提供一個 public 函數讓 MainWindow 取得最終處理好的圖片
    QImage getProcessedImage() const;

    bool isCoverMode()const;

private slots:
    // 當 spinbox 或 combobox 的值改變時，呼叫此槽函數來更新預覽
    void updatePreview();

    // [新增] 處理 "Picture_tab" 的開啟圖片按鈕
    void on_OpenPicture_pushButton_clicked();

    // [新增] 處理 "File_tab" 的開啟檔案按鈕
    void on_Openfile_pushButton_clicked();



private:
    Ui::ImageImportDialog *ui;
    QImage m_originalImage;   // 保存未經處理的原始圖片
    QImage m_processedImage;  // 保存當前預覽/最終處理的圖片


    QImage m_fileImportImage;  // [新增] 檔案 Tab 解析出來的圖

    // --- File Tab 變數 ---
    QImage m_fileRawImage;     // 從 .h 檔解析出來的原始圖片


    // 輔助函式：解析文字內容並轉為圖片
    //void parseAndPreviewFile(const QString &fileContent);

    void parseFileContentToImage(const QString &content);
    QString m_currentFileContent; // 新增：儲存目前讀入的 .h 檔案文字內容

    void updateFilePreview();


    QString m_lastLoadedFileContent; // 暫存檔案內容，方便切換模式時重繪

};

#endif // IMAGEIMPORTDIALOG_H
