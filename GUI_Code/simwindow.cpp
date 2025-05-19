#include "simwindow.h"
#include "ui_simwindow.h"

simWindow::simWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::simWindow)
{
    ui->setupUi(this);
}

simWindow::~simWindow()
{
    delete ui;
}
