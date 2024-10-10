#ifndef CREDIT_DIALOG_H
#define CREDIT_DIALOG_H

#include <QDialog>

namespace Ui {
class credit_dialog;
}

class credit_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit credit_dialog(QWidget *parent = nullptr);
    ~credit_dialog();


private slots:
    void on_pushButton_clicked();

private:
    Ui::credit_dialog *ui;
};

#endif // CREDIT_DIALOG_H
