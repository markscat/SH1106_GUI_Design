#ifndef MAINWINDOW_H
#define MAINWINDOW_H



#include "ToolType.h"
#include "oledwidget.h"
#include "imageimportdialog.h"
#include "oleddatamodel.h"
#include "config.h"



class OLEDWidget; // 前向聲明

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    Q_ENUM(ToolType) // 注册枚举，使其在 Qt 元对象系统可用

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    ToolType getCurrentTool() const; // 提供一个给外部获取当前工具的接口

private slots:
    void resetOledPlaceholderSize(); // 新的槽函數，用於重置尺寸
    void exportData(); // 聲明槽函數
    void saveData();
    void importImage(); // <-- 新增槽函式声明
    void updateCoordinateLabel(const QPoint &pos);

    void on_pushButton_Copy_clicked();
    void on_pushButton_paste_clicked();


private:
    Ui::MainWindow *ui;
    OLEDWidget *m_oled;
    QButtonGroup *m_toolButtonGroup;
    QScrollArea* scrollArea;   // <- 必須有這行
    ToolType m_currentTool;          // 储存当前选中的工具
    QSize m_originalOledSize;; // 用於儲存 oledPlaceholder 的原始尺寸
};
#endif // MAINWINDOW_H
