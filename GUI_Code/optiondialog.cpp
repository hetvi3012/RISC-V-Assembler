#include "OptionDialog.h"

OptionDialog::OptionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Select Options");
    QVBoxLayout *layout = new QVBoxLayout(this);

    option1 = new QCheckBox("Enable Pipeline", this);
    option2 = new QCheckBox("Enable Data Forwarding", this);
    option3 = new QCheckBox("Enable Branch Prediction", this);

    layout->addWidget(option1);
    layout->addWidget(option2);
    layout->addWidget(option3);

    // Initially disable the rest
    option2->setEnabled(false);
    option3->setEnabled(false);

    connect(option1, &QCheckBox::stateChanged, this, &OptionDialog::onOption1Changed);

    // OK and Cancel buttons
    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(okButton);
    btnLayout->addWidget(cancelButton);
    layout->addLayout(btnLayout);

    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void OptionDialog::onOption1Changed(int state)
{
    bool enabled = (state == Qt::Checked);
    QList<QCheckBox*> subOptions = { option2, option3 };

    for (QCheckBox *box : subOptions) {
        box->setEnabled(enabled);
        if (!enabled)
            box->setChecked(false);
    }
}

QList<bool> OptionDialog::getOptions() const
{
    return {
        option1->isChecked(),
        option2->isChecked(),
        option3->isChecked()
    };
}
