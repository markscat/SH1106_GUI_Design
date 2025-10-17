#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include "oledwidget.h"
#include "ToolType.h"
#include <QButtonGroup> // 加入標頭檔

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
    void exportData(); // 聲明槽函數
    void saveData();
    void importImage(); // <-- 新增槽函式声明


private:
    Ui::MainWindow *ui;
    OLEDWidget *m_oled;
    //QButtonGroup *m_toolButtonGroup; // 用于管理工具按钮
    QButtonGroup *m_toolButtonGroup;



    ToolType m_currentTool;          // 储存当前选中的工具

};
#endif // MAINWINDOW_H
