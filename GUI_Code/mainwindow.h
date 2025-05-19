#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void on_btnSimulator_clicked();

    void on_btnEditor_clicked();

    void on_txtEditor_textChanged();

    void on_runButton_clicked();

    void on_stepButton_clicked();

    void on_prevButton_clicked();

    void on_resetButton_clicked();

    void on_btnRegistor_clicked();

    void on_btnMemory_clicked();

    void on_btnGoToMem_clicked();

    void on_txtMemAddress_returnPressed();

    void on_selectMemJump_currentIndexChanged(int index);

    void on_btnMemUp_clicked();

    void on_btnMemDown_clicked();

    void on_tableMC_cellClicked(int row, int column);

private:
    Ui::MainWindow *ui;
    void updateConsoleDisplay();
    void updateCMDStates();
    void updateMCDisplay();
    void updateMemoryDisplay();
    void updateRegisterDisplay();
};
#endif // MAINWINDOW_H
