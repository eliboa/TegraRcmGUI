#include "qtools.h"
#include "ui_qtools.h"

qTools::qTools(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::qTools)
{
    ui->setupUi(this);
}

qTools::~qTools()
{
    delete ui;
}
