/*
 *  **************************************************************************
 Project name    : SH1106_GUI_Design
 @file           : main.cpp
 @brief          : Main program body
 ******************************************************************************
 * 本程式主要是用來提供設計Sh1106系列 3.1吋（或以下）的UI設計程式
 *　很多玩阿都諾的竹……設計者，都會用網路上其他的工具。
 *　我是不知道那些本……設計者心裡在想什麼，我只知道當我灌好Qt之後，就一直想要找個題目來寫一個程式出來。
 *
 *　其實這個程式的風格和我之前C#的專案BMP2Array有異曲同工，但那個程式專住在轉換檔案以及製作字型.ｈ上
 *  並沒有真正規劃到『怎麼在螢幕上設計相對應的GUI
 *
 *
 *
 * V25.10.17
 *
 * 終於好多了.
 * 試著玩了一下，會有幾個問題：
 * 第一：沒辦法縮放
 * 因為我會習慣用ctrl+滑鼠滾輪把螢幕放大,這樣看起來比較輕鬆.
 * 縮放有兩種：
 *
 *
 * 一種是工作區的放大。
 * 另一種是當我放大視窗的時候，工作區最好也跟著一起放大；但依舊必須局現在128*64的範圍
 * 10/18
 *  是可以放大縮小,旁邊也有卷軸,但現在滑鼠右鍵無法動作,取消已經點選的那一點.
 * 10/24
 * 匯入圖片不支援png圖片
 * 畫線，畫點，圓和方都沒問題了
 *
 * 第二：
 *  * 橡皮擦
 *  * 10/24
 * 這個功能被筆刷大小給取代了.
 *
 * 第三：
 * ｕｎｄｏ(Ctrl+Z / Ctrl+Y)
 *
 * 第四：
 * 筆刷放大後,滑鼠右鍵拖曳就可以當成橡皮擦了
 * 工具區好難看……
 * 一般的是有個icon,在那邊然後飄出相對應的文字說明
 *
 * 第五:
 * 印章工具；我想做一個類似印章的工具，先用比較常見的七段顯示器的當作印章
 * 我點選了這個按鍵，然後跳出一個框來，可以選七段顯示器的線條粗細，整體大小，還有風格（斜體，還是米字），
 *
 *
 *
 *
 *版權：
 *GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 * Copyright (C) [2025] [Ethan]
 * 本程式是一個自由軟體：您可以依照 **GNU 通用公共授權條款（GPL）** 發佈和/或修改，
 * GPL 版本 3 或（依您選擇）任何更新版本。
 *
 * 本程式的發佈目的是希望它對您有幫助，但 **不提供任何擔保**，甚至不包含適銷性或特定用途適用性的默示擔保。
 * 請參閱 **GNU 通用公共授權條款** 以獲取更多詳細資訊。
 * 您應當已經收到一份 **GNU 通用公共授權條款** 副本。
 * 如果沒有，請參閱 <https://www.gnu.org/licenses/gpl-3.0.html>。
 *
 *
 *
 *

*/





#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "SH1106_GUI_Design_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
