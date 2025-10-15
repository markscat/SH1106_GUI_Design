#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "oledwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void exportData(); // 聲明槽函數
    void saveData();
    void importImage(); // <-- 新增槽函式声明


private:
    Ui::MainWindow *ui;
    OLEDWidget *m_oled;
};
#endif // MAINWINDOW_H
