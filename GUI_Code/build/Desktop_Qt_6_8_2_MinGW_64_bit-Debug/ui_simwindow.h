/********************************************************************************
** Form generated from reading UI file 'simwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SIMWINDOW_H
#define UI_SIMWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_simWindow
{
public:
    QPushButton *btnSimulator;
    QPlainTextEdit *txtEditor;
    QPushButton *btnEditor;

    void setupUi(QDialog *simWindow)
    {
        if (simWindow->objectName().isEmpty())
            simWindow->setObjectName("simWindow");
        simWindow->resize(912, 542);
        simWindow->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0)"));
        btnSimulator = new QPushButton(simWindow);
        btnSimulator->setObjectName("btnSimulator");
        btnSimulator->setEnabled(false);
        btnSimulator->setGeometry(QRect(430, 0, 131, 51));
        btnSimulator->setStyleSheet(QString::fromUtf8("background-color: rgb(10, 14, 20)"));
        txtEditor = new QPlainTextEdit(simWindow);
        txtEditor->setObjectName("txtEditor");
        txtEditor->setGeometry(QRect(0, 50, 911, 491));
        txtEditor->setStyleSheet(QString::fromUtf8("background-color: rgb(10, 14, 20)"));
        btnEditor = new QPushButton(simWindow);
        btnEditor->setObjectName("btnEditor");
        btnEditor->setEnabled(true);
        btnEditor->setGeometry(QRect(300, 0, 131, 51));
        btnEditor->setStyleSheet(QString::fromUtf8(""));
        btnEditor->setCheckable(false);
        btnEditor->setChecked(false);
        btnEditor->setFlat(false);

        retranslateUi(simWindow);

        btnEditor->setDefault(false);


        QMetaObject::connectSlotsByName(simWindow);
    } // setupUi

    void retranslateUi(QDialog *simWindow)
    {
        simWindow->setWindowTitle(QCoreApplication::translate("simWindow", "Dialog", nullptr));
        btnSimulator->setText(QCoreApplication::translate("simWindow", "Simulator", nullptr));
        btnEditor->setText(QCoreApplication::translate("simWindow", "Editor", nullptr));
    } // retranslateUi

};

namespace Ui {
    class simWindow: public Ui_simWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SIMWINDOW_H
