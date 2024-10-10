#include "dialogcope.h"
#include "ui_dialogcope.h"

DialogCope::DialogCope(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCope)
{
    ui->setupUi(this);
}

DialogCope::~DialogCope()
{
    delete ui;
}
