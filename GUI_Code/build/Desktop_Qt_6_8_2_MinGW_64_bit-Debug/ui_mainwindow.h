/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>
#include "mytablewidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *btnEditor;
    MyTableWidget *tableMC;
    QPushButton *btnSimulator;
    QPlainTextEdit *txtEditor;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *layoutCmdButtons;
    QPushButton *runButton;
    QPushButton *stepButton;
    QPushButton *prevButton;
    QPushButton *resetButton;
    QTableWidget *tableRegister;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *layoutMemButtons;
    QPushButton *btnRegistor;
    QPushButton *btnMemory;
    QPlainTextEdit *txtConsole;
    QTableWidget *tableMemory;
    QWidget *gridLayoutWidget;
    QGridLayout *layoutMemButtons_2;
    QPushButton *btnGoToMem;
    QLabel *labelJumpTo;
    QLabel *labelAddress;
    QLineEdit *txtMemAddress;
    QPushButton *btnMemDown;
    QPushButton *btnMemUp;
    QComboBox *selectMemJump;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1158, 615);
        MainWindow->setAutoFillBackground(false);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setAutoFillBackground(false);
        centralwidget->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0)"));
        btnEditor = new QPushButton(centralwidget);
        btnEditor->setObjectName("btnEditor");
        btnEditor->setEnabled(false);
        btnEditor->setGeometry(QRect(450, 0, 131, 51));
        btnEditor->setStyleSheet(QString::fromUtf8("QPushButton { font-size: 20px; } QPushButton:disabled { background-color: rgb(10, 14, 20); font-size: 20px; }"));
        btnEditor->setCheckable(false);
        btnEditor->setChecked(false);
        btnEditor->setFlat(true);
        tableMC = new MyTableWidget(centralwidget);
        if (tableMC->columnCount() < 6)
            tableMC->setColumnCount(6);
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem->setFont(font);
        tableMC->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem1->setFont(font);
        tableMC->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem2->setFont(font);
        tableMC->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        __qtablewidgetitem3->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem3->setFont(font);
        tableMC->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        __qtablewidgetitem4->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem4->setFont(font);
        tableMC->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        __qtablewidgetitem5->setTextAlignment(Qt::AlignLeading|Qt::AlignVCenter);
        __qtablewidgetitem5->setFont(font);
        tableMC->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        tableMC->setObjectName("tableMC");
        tableMC->setEnabled(true);
        tableMC->setGeometry(QRect(10, 111, 691, 351));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tableMC->sizePolicy().hasHeightForWidth());
        tableMC->setSizePolicy(sizePolicy);
        QFont font1;
        font1.setPointSize(12);
        tableMC->setFont(font1);
        tableMC->setStyleSheet(QString::fromUtf8(" QTableWidget::item { border: 0px; border-bottom: 1px solid grey;}       QHeaderView::section { border: 0px; border-bottom: 1px solid white; }"));
        tableMC->horizontalHeader()->setVisible(true);
        tableMC->horizontalHeader()->setDefaultSectionSize(225);
        tableMC->verticalHeader()->setVisible(false);
        btnSimulator = new QPushButton(centralwidget);
        btnSimulator->setObjectName("btnSimulator");
        btnSimulator->setEnabled(true);
        btnSimulator->setGeometry(QRect(580, 0, 131, 51));
        btnSimulator->setStyleSheet(QString::fromUtf8("QPushButton { font-size: 20px; } QPushButton:disabled { background-color: rgb(10, 14, 20); font-size: 20px; }"));
        btnSimulator->setFlat(true);
        txtEditor = new QPlainTextEdit(centralwidget);
        txtEditor->setObjectName("txtEditor");
        txtEditor->setEnabled(true);
        txtEditor->setGeometry(QRect(0, 50, 1151, 561));
        QFont font2;
        txtEditor->setFont(font2);
        txtEditor->setStyleSheet(QString::fromUtf8("    background-color: rgb(10, 14, 20); color: rgb(179,177,173); font-size: 16px;"));
        horizontalLayoutWidget = new QWidget(centralwidget);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(10, 60, 691, 61));
        layoutCmdButtons = new QHBoxLayout(horizontalLayoutWidget);
        layoutCmdButtons->setSpacing(20);
        layoutCmdButtons->setObjectName("layoutCmdButtons");
        layoutCmdButtons->setContentsMargins(20, 0, 20, 0);
        runButton = new QPushButton(horizontalLayoutWidget);
        runButton->setObjectName("runButton");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(runButton->sizePolicy().hasHeightForWidth());
        runButton->setSizePolicy(sizePolicy1);
        runButton->setFont(font1);

        layoutCmdButtons->addWidget(runButton);

        stepButton = new QPushButton(horizontalLayoutWidget);
        stepButton->setObjectName("stepButton");
        sizePolicy1.setHeightForWidth(stepButton->sizePolicy().hasHeightForWidth());
        stepButton->setSizePolicy(sizePolicy1);
        stepButton->setFont(font1);

        layoutCmdButtons->addWidget(stepButton);

        prevButton = new QPushButton(horizontalLayoutWidget);
        prevButton->setObjectName("prevButton");
        sizePolicy1.setHeightForWidth(prevButton->sizePolicy().hasHeightForWidth());
        prevButton->setSizePolicy(sizePolicy1);
        prevButton->setFont(font1);

        layoutCmdButtons->addWidget(prevButton);

        resetButton = new QPushButton(horizontalLayoutWidget);
        resetButton->setObjectName("resetButton");
        sizePolicy1.setHeightForWidth(resetButton->sizePolicy().hasHeightForWidth());
        resetButton->setSizePolicy(sizePolicy1);
        resetButton->setFont(font1);

        layoutCmdButtons->addWidget(resetButton);

        tableRegister = new QTableWidget(centralwidget);
        if (tableRegister->columnCount() < 2)
            tableRegister->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableRegister->setHorizontalHeaderItem(0, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableRegister->setHorizontalHeaderItem(1, __qtablewidgetitem7);
        tableRegister->setObjectName("tableRegister");
        tableRegister->setGeometry(QRect(710, 90, 441, 471));
        tableRegister->horizontalHeader()->setVisible(false);
        tableRegister->horizontalHeader()->setDefaultSectionSize(210);
        tableRegister->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        tableRegister->verticalHeader()->setVisible(false);
        horizontalLayoutWidget_2 = new QWidget(centralwidget);
        horizontalLayoutWidget_2->setObjectName("horizontalLayoutWidget_2");
        horizontalLayoutWidget_2->setGeometry(QRect(750, 60, 371, 33));
        layoutMemButtons = new QHBoxLayout(horizontalLayoutWidget_2);
        layoutMemButtons->setSpacing(0);
        layoutMemButtons->setObjectName("layoutMemButtons");
        layoutMemButtons->setContentsMargins(0, 0, 0, 0);
        btnRegistor = new QPushButton(horizontalLayoutWidget_2);
        btnRegistor->setObjectName("btnRegistor");
        sizePolicy1.setHeightForWidth(btnRegistor->sizePolicy().hasHeightForWidth());
        btnRegistor->setSizePolicy(sizePolicy1);
        QFont font3;
        font3.setPointSize(10);
        btnRegistor->setFont(font3);

        layoutMemButtons->addWidget(btnRegistor);

        btnMemory = new QPushButton(horizontalLayoutWidget_2);
        btnMemory->setObjectName("btnMemory");
        sizePolicy1.setHeightForWidth(btnMemory->sizePolicy().hasHeightForWidth());
        btnMemory->setSizePolicy(sizePolicy1);
        btnMemory->setFont(font3);

        layoutMemButtons->addWidget(btnMemory);

        txtConsole = new QPlainTextEdit(centralwidget);
        txtConsole->setObjectName("txtConsole");
        txtConsole->setGeometry(QRect(13, 464, 681, 141));
        txtConsole->setStyleSheet(QString::fromUtf8("border: 1px solid white;"));
        txtConsole->setReadOnly(true);
        tableMemory = new QTableWidget(centralwidget);
        if (tableMemory->columnCount() < 5)
            tableMemory->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableMemory->setHorizontalHeaderItem(0, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableMemory->setHorizontalHeaderItem(1, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableMemory->setHorizontalHeaderItem(2, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        tableMemory->setHorizontalHeaderItem(3, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tableMemory->setHorizontalHeaderItem(4, __qtablewidgetitem12);
        tableMemory->setObjectName("tableMemory");
        tableMemory->setGeometry(QRect(710, 90, 441, 471));
        tableMemory->horizontalHeader()->setVisible(true);
        tableMemory->horizontalHeader()->setDefaultSectionSize(210);
        tableMemory->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        tableMemory->verticalHeader()->setVisible(false);
        gridLayoutWidget = new QWidget(centralwidget);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(730, 556, 413, 51));
        layoutMemButtons_2 = new QGridLayout(gridLayoutWidget);
        layoutMemButtons_2->setObjectName("layoutMemButtons_2");
        layoutMemButtons_2->setContentsMargins(0, 0, 0, 0);
        btnGoToMem = new QPushButton(gridLayoutWidget);
        btnGoToMem->setObjectName("btnGoToMem");

        layoutMemButtons_2->addWidget(btnGoToMem, 6, 4, 1, 1);

        labelJumpTo = new QLabel(gridLayoutWidget);
        labelJumpTo->setObjectName("labelJumpTo");
        QFont font4;
        font4.setBold(true);
        labelJumpTo->setFont(font4);
        labelJumpTo->setTextFormat(Qt::TextFormat::AutoText);

        layoutMemButtons_2->addWidget(labelJumpTo, 0, 0, 1, 1);

        labelAddress = new QLabel(gridLayoutWidget);
        labelAddress->setObjectName("labelAddress");
        labelAddress->setFont(font4);

        layoutMemButtons_2->addWidget(labelAddress, 6, 0, 1, 1);

        txtMemAddress = new QLineEdit(gridLayoutWidget);
        txtMemAddress->setObjectName("txtMemAddress");

        layoutMemButtons_2->addWidget(txtMemAddress, 6, 1, 1, 3);

        btnMemDown = new QPushButton(gridLayoutWidget);
        btnMemDown->setObjectName("btnMemDown");

        layoutMemButtons_2->addWidget(btnMemDown, 0, 3, 1, 1);

        btnMemUp = new QPushButton(gridLayoutWidget);
        btnMemUp->setObjectName("btnMemUp");

        layoutMemButtons_2->addWidget(btnMemUp, 0, 2, 1, 1);

        selectMemJump = new QComboBox(gridLayoutWidget);
        selectMemJump->setObjectName("selectMemJump");

        layoutMemButtons_2->addWidget(selectMemJump, 0, 1, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        tableRegister->raise();
        btnEditor->raise();
        btnSimulator->raise();
        horizontalLayoutWidget->raise();
        tableMC->raise();
        horizontalLayoutWidget_2->raise();
        txtConsole->raise();
        tableMemory->raise();
        gridLayoutWidget->raise();
        txtEditor->raise();

        retranslateUi(MainWindow);

        btnEditor->setDefault(false);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        btnEditor->setText(QCoreApplication::translate("MainWindow", "Editor", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableMC->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "PC", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableMC->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Machine Code", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableMC->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "Assembly Code", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableMC->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "New Column", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableMC->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "New Column", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableMC->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "New Column", nullptr));
        btnSimulator->setText(QCoreApplication::translate("MainWindow", "Simulator", nullptr));
        runButton->setText(QCoreApplication::translate("MainWindow", "Run", nullptr));
        stepButton->setText(QCoreApplication::translate("MainWindow", "Step", nullptr));
        prevButton->setText(QCoreApplication::translate("MainWindow", "Prev", nullptr));
        resetButton->setText(QCoreApplication::translate("MainWindow", "Reset", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableRegister->horizontalHeaderItem(0);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "Reg", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableRegister->horizontalHeaderItem(1);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "Val", nullptr));
        btnRegistor->setText(QCoreApplication::translate("MainWindow", "Registers", nullptr));
        btnMemory->setText(QCoreApplication::translate("MainWindow", "Memory", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableMemory->horizontalHeaderItem(0);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "Address", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = tableMemory->horizontalHeaderItem(1);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "+3", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = tableMemory->horizontalHeaderItem(2);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("MainWindow", "+2", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = tableMemory->horizontalHeaderItem(3);
        ___qtablewidgetitem11->setText(QCoreApplication::translate("MainWindow", "+1", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = tableMemory->horizontalHeaderItem(4);
        ___qtablewidgetitem12->setText(QCoreApplication::translate("MainWindow", "+0", nullptr));
        btnGoToMem->setText(QCoreApplication::translate("MainWindow", "Go", nullptr));
        labelJumpTo->setText(QCoreApplication::translate("MainWindow", "Jump to", nullptr));
        labelAddress->setText(QCoreApplication::translate("MainWindow", "Address:", nullptr));
        btnMemDown->setText(QCoreApplication::translate("MainWindow", "Down", nullptr));
        btnMemUp->setText(QCoreApplication::translate("MainWindow", "Up", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
