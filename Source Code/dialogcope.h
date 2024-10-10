#ifndef DIALOGCOPE_H
#define DIALOGCOPE_H

#include <QDialog>

namespace Ui {
class DialogCope;
}

class DialogCope : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCope(QWidget *parent = nullptr);
    ~DialogCope();

private:
    Ui::DialogCope *ui;
};

#endif // DIALOGCOPE_H
