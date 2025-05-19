#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>

class OptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionDialog(QWidget *parent = nullptr);
    QList<bool> getOptions() const;

private slots:
    void onOption1Changed(int state);

private:
    QCheckBox *option1;
    QCheckBox *option2;
    QCheckBox *option3;
    QCheckBox *option4;
    QCheckBox *option5;
    QCheckBox *option6;

    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // OPTIONDIALOG_H
