#include "stampdialog.h"
#include "ui_stampdialog.h"

StampDialog::StampDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StampDialog)
{
    ui->setupUi(this);
}

StampDialog::~StampDialog()
{
    delete ui;
}
