#ifndef CONFIG_H
#define CONFIG_H
#pragma once

#include <QApplication>
#include <QBuffer>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QImageReader>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QPoint>
#include <QScrollArea>
#include <QScrollBar>
#include <QSize>
#include <QTextStream>
#include <QTextEdit>
#include <QTransform>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>

#include <cstring>


// --- 硬體模擬常數 (Hardware Simulation Constants) ---
namespace OledConfig {

// 這是 OLED 螢幕的可視區域尺寸
constexpr int  DISPLAY_WIDTH   = 128;
constexpr int  DISPLAY_HEIGHT  = 64;

// 這是驅動晶片 (如 SH1106) 的記憶體頁寬度。
// SH1106 的 RAM 是 132x64，所以頁寬是 132。
// 如果是 SSD1306，它的 RAM 是 128x64，那這個值就是 128。
constexpr int RAM_PAGE_WIDTH = 132;
//constexpr int RAM_PAGE_WIDTH = 128;

// 這是顯示區域在 RAM 中的起始欄位偏移。
// SH1106 的 128 像素寬的顯示區域通常是從 RAM 的第 2 欄開始的。
// SSD1306 則沒有偏移，這個值會是 0。
constexpr int  COLUMN_OFFSET =  2;

}

#define QMessageBox_debug

#define Past_Function


#endif // CONFIG_H
