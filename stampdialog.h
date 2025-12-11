#ifndef STAMPDIALOG_H
#define STAMPDIALOG_H

#include <QDialog>

namespace Ui {
class StampDialog;
}

class StampDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StampDialog(QWidget *parent = nullptr);
    ~StampDialog();

private:
    Ui::StampDialog *ui;
};

#endif // STAMPDIALOG_H
