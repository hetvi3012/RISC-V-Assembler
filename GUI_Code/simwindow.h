#ifndef SIMWINDOW_H
#define SIMWINDOW_H

#include <QDialog>

namespace Ui {
class simWindow;
}

class simWindow : public QDialog
{
    Q_OBJECT

public:
    explicit simWindow(QWidget *parent = nullptr);
    ~simWindow();

private:
    Ui::simWindow *ui;
};

#endif // SIMWINDOW_H
