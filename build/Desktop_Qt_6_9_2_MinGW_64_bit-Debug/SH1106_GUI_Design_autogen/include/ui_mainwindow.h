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
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QSplitter *splitter;
    QWidget *ToolbarWdidget;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *ToolLineLayout;
    QSpacerItem *horizontalSpacer;
    QToolButton *ToolLine;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *ToolCircleLayout;
    QSpacerItem *horizontalSpacer_3;
    QToolButton *ToolCircle;
    QSpacerItem *horizontalSpacer_4;
    QHBoxLayout *ToolPenLayout;
    QSpacerItem *horizontalSpacer_5;
    QToolButton *ToolPen;
    QSpacerItem *horizontalSpacer_6;
    QHBoxLayout *ToolFilledRectangleLayout;
    QSpacerItem *horizontalSpacer_7;
    QToolButton *ToolFilledRectangle;
    QSpacerItem *horizontalSpacer_8;
    QHBoxLayout *ToolRectangleLayout;
    QSpacerItem *horizontalSpacer_9;
    QToolButton *ToolRectangle;
    QSpacerItem *horizontalSpacer_10;
    QWidget *JobAreaWidget;
    QWidget *layoutWidget;
    QVBoxLayout *JobAreaLayout;
    QHBoxLayout *FunctionLayout;
    QPushButton *clearButton;
    QPushButton *saveButton;
    QPushButton *exportButton;
    QPushButton *importButton;
    QPushButton *resetOledSizeButton;
    QWidget *oledPlaceholder;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1067, 593);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy);
        centralwidget->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName("splitter");
        splitter->setGeometry(QRect(0, 0, 1061, 551));
        sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy);
        splitter->setOrientation(Qt::Orientation::Horizontal);
        ToolbarWdidget = new QWidget(splitter);
        ToolbarWdidget->setObjectName("ToolbarWdidget");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(ToolbarWdidget->sizePolicy().hasHeightForWidth());
        ToolbarWdidget->setSizePolicy(sizePolicy1);
        widget = new QWidget(ToolbarWdidget);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(0, 10, 141, 201));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        ToolLineLayout = new QHBoxLayout();
        ToolLineLayout->setObjectName("ToolLineLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolLineLayout->addItem(horizontalSpacer);

        ToolLine = new QToolButton(widget);
        ToolLine->setObjectName("ToolLine");

        ToolLineLayout->addWidget(ToolLine);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolLineLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(ToolLineLayout);

        ToolCircleLayout = new QHBoxLayout();
        ToolCircleLayout->setObjectName("ToolCircleLayout");
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolCircleLayout->addItem(horizontalSpacer_3);

        ToolCircle = new QToolButton(widget);
        ToolCircle->setObjectName("ToolCircle");

        ToolCircleLayout->addWidget(ToolCircle);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolCircleLayout->addItem(horizontalSpacer_4);


        verticalLayout->addLayout(ToolCircleLayout);

        ToolPenLayout = new QHBoxLayout();
        ToolPenLayout->setObjectName("ToolPenLayout");
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolPenLayout->addItem(horizontalSpacer_5);

        ToolPen = new QToolButton(widget);
        ToolPen->setObjectName("ToolPen");

        ToolPenLayout->addWidget(ToolPen);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolPenLayout->addItem(horizontalSpacer_6);


        verticalLayout->addLayout(ToolPenLayout);

        ToolFilledRectangleLayout = new QHBoxLayout();
        ToolFilledRectangleLayout->setObjectName("ToolFilledRectangleLayout");
        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolFilledRectangleLayout->addItem(horizontalSpacer_7);

        ToolFilledRectangle = new QToolButton(widget);
        ToolFilledRectangle->setObjectName("ToolFilledRectangle");

        ToolFilledRectangleLayout->addWidget(ToolFilledRectangle);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolFilledRectangleLayout->addItem(horizontalSpacer_8);


        verticalLayout->addLayout(ToolFilledRectangleLayout);

        ToolRectangleLayout = new QHBoxLayout();
        ToolRectangleLayout->setObjectName("ToolRectangleLayout");
        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolRectangleLayout->addItem(horizontalSpacer_9);

        ToolRectangle = new QToolButton(widget);
        ToolRectangle->setObjectName("ToolRectangle");

        ToolRectangleLayout->addWidget(ToolRectangle);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        ToolRectangleLayout->addItem(horizontalSpacer_10);


        verticalLayout->addLayout(ToolRectangleLayout);

        splitter->addWidget(ToolbarWdidget);
        JobAreaWidget = new QWidget(splitter);
        JobAreaWidget->setObjectName("JobAreaWidget");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy2.setHorizontalStretch(1);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(JobAreaWidget->sizePolicy().hasHeightForWidth());
        JobAreaWidget->setSizePolicy(sizePolicy2);
        layoutWidget = new QWidget(JobAreaWidget);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(10, 10, 881, 531));
        JobAreaLayout = new QVBoxLayout(layoutWidget);
        JobAreaLayout->setObjectName("JobAreaLayout");
        JobAreaLayout->setContentsMargins(0, 0, 0, 0);
        FunctionLayout = new QHBoxLayout();
        FunctionLayout->setObjectName("FunctionLayout");
        clearButton = new QPushButton(layoutWidget);
        clearButton->setObjectName("clearButton");

        FunctionLayout->addWidget(clearButton);

        saveButton = new QPushButton(layoutWidget);
        saveButton->setObjectName("saveButton");

        FunctionLayout->addWidget(saveButton);

        exportButton = new QPushButton(layoutWidget);
        exportButton->setObjectName("exportButton");

        FunctionLayout->addWidget(exportButton);

        importButton = new QPushButton(layoutWidget);
        importButton->setObjectName("importButton");

        FunctionLayout->addWidget(importButton);

        resetOledSizeButton = new QPushButton(layoutWidget);
        resetOledSizeButton->setObjectName("resetOledSizeButton");

        FunctionLayout->addWidget(resetOledSizeButton);


        JobAreaLayout->addLayout(FunctionLayout);

        oledPlaceholder = new QWidget(layoutWidget);
        oledPlaceholder->setObjectName("oledPlaceholder");
        sizePolicy.setHeightForWidth(oledPlaceholder->sizePolicy().hasHeightForWidth());
        oledPlaceholder->setSizePolicy(sizePolicy);

        JobAreaLayout->addWidget(oledPlaceholder);

        JobAreaLayout->setStretch(1, 1);
        splitter->addWidget(JobAreaWidget);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1067, 25));
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
        ToolLine->setText(QCoreApplication::translate("MainWindow", "Line", nullptr));
        ToolCircle->setText(QCoreApplication::translate("MainWindow", "Circle", nullptr));
        ToolPen->setText(QCoreApplication::translate("MainWindow", "Pen", nullptr));
        ToolFilledRectangle->setText(QCoreApplication::translate("MainWindow", "FilledRectangle", nullptr));
        ToolRectangle->setText(QCoreApplication::translate("MainWindow", "Rectangle", nullptr));
        clearButton->setText(QCoreApplication::translate("MainWindow", "\346\270\205\351\231\244", nullptr));
        saveButton->setText(QCoreApplication::translate("MainWindow", "\345\255\230\346\252\224", nullptr));
        exportButton->setText(QCoreApplication::translate("MainWindow", "\345\214\257\345\207\272", nullptr));
        importButton->setText(QCoreApplication::translate("MainWindow", "\345\214\257\345\205\245\345\234\226\346\252\224", nullptr));
        resetOledSizeButton->setText(QCoreApplication::translate("MainWindow", "\351\207\215\350\243\275\347\271\252\345\234\226\346\241\206\345\260\272\345\257\270", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
