/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *clearButton;
    QPushButton *saveButton;
    QPushButton *exportButton;
    QPushButton *importButton;
    QWidget *oledPlaceholder;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(14, 19, 771, 511));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        clearButton = new QPushButton(widget);
        clearButton->setObjectName("clearButton");

        horizontalLayout->addWidget(clearButton);

        saveButton = new QPushButton(widget);
        saveButton->setObjectName("saveButton");

        horizontalLayout->addWidget(saveButton);

        exportButton = new QPushButton(widget);
        exportButton->setObjectName("exportButton");

        horizontalLayout->addWidget(exportButton);

        importButton = new QPushButton(widget);
        importButton->setObjectName("importButton");

        horizontalLayout->addWidget(importButton);


        verticalLayout->addLayout(horizontalLayout);

        oledPlaceholder = new QWidget(widget);
        oledPlaceholder->setObjectName("oledPlaceholder");

        verticalLayout->addWidget(oledPlaceholder);

        verticalLayout->setStretch(1, 1);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 21));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        clearButton->setText(QCoreApplication::translate("MainWindow", "Clear", nullptr));
        saveButton->setText(QCoreApplication::translate("MainWindow", "\345\255\230\346\252\224", nullptr));
        exportButton->setText(QCoreApplication::translate("MainWindow", "Export", nullptr));
        importButton->setText(QCoreApplication::translate("MainWindow", "\345\214\257\345\205\245\345\234\226\346\252\224", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
