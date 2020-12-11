#include "um_dialog.h"
#include "ui_um_dialog.h"

UM_Dialog::UM_Dialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UM_Dialog)
{
    ui->setupUi(this);
}

UM_Dialog::~UM_Dialog()
{
    delete ui;
}
