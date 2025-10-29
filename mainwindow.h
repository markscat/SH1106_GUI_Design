#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QPainter> // <--- 把这一行加进来！
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDialog>
#include <QPushButton>
#include <QFileDialog>
#include <QImageReader>
#include <QImage>
#include <QBuffer>
#include <QCheckBox>
#include <QScrollArea>  // ← 一定要加在最上方 include 區
#include <QWheelEvent>
#include <QScrollBar>
#include <QButtonGroup> // 加入標頭檔
#include <QSize>


#include "ToolType.h"

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

private:
    Ui::MainWindow *ui;
    OLEDWidget *m_oled;
    QButtonGroup *m_toolButtonGroup;
    QScrollArea* scrollArea;   // <- 必須有這行
    ToolType m_currentTool;          // 储存当前选中的工具
    QSize m_originalOledSize;; // 用於儲存 oledPlaceholder 的原始尺寸
};
#endif // MAINWINDOW_H
