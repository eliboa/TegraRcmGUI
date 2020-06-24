#include "tegrarcmgui.h"
#include "ui_tegrarcmgui.h"

TegraRcmGUI::TegraRcmGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TegraRcmGUI)
{
    ui->setupUi(this);
}

TegraRcmGUI::~TegraRcmGUI()
{
    delete ui;
}

