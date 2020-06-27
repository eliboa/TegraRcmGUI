#include "qtools.h"
#include "ui_qtools.h"

qTools::qTools(TegraRcmGUI *parent) : QWidget(parent),
    ui(new Ui::qTools), parent(parent)
{
    ui->setupUi(this);
    m_kourou = parent->m_kourou;
    m_device = &parent->m_device;
    connect(this, SIGNAL(error(int)), parent,  SLOT(error(int)));

    /// Stylesheets
    // Apply stylesheet to all buttons
    QString btnSs = GetStyleSheetFromResFile(":/res/QPushButton.qss");
    auto buttons = this->findChildren<QPushButton*>();
    for (int i = 0; i < buttons.count(); i++)
    {
        buttons.at(i)->setStyleSheet(btnSs);
        buttons.at(i)->setCursor(Qt::PointingHandCursor);
    }

    ui->autoRcmFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box02.qss"));
    ui->genricToolFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box02.qss"));
    ui->autoRcm_warningFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QLabel_warning.qss"));
    ui->autoRcmTitleLbl->setStyleSheet(GetStyleSheetFromResFile(":/res/QLabel_title02.qss"));

    // Buttons
    autoRCM_switch = new Switch(false, 50);
    ui->autoRcmLayout->addWidget(autoRCM_switch);
    connect(autoRCM_switch, SIGNAL(clicked()), this, SLOT(on_autoRcmSwitchToggled()));

    Switch *_switch2 = new Switch(false, 50);
    ui->genricToolLayout->addWidget(_switch2);
    //connect(_switch, SIGNAL(clicked()), this, SLOT(on_autoInject_toggled()));
}

qTools::~qTools()
{
    delete ui;
}

void qTools::on_deviceStateChange()
{
    //autoRcm_arianeLbl

    if (!m_device->arianeIsReady() || !parent->isDeviceInfoAvailable())
    {
        QString label;
        if (m_device->arianeIsReady())
            label.append(tr("Waiting for Ariane response"));
        else
        {
            label.append(tr("Ariane needs to be loaded!\n"));
            label.append(m_kourou->autoLaunchAriane ? tr("Boot device to RCM") : tr("Enable Ariane autoboot first"));
        }
        ui->autoRcm_warningLbl->setText(label);
        ui->autoRcm_warningFrame->show();
        autoRCM_switch->hide();
    }
    else
    {
        ui->autoRcm_warningFrame->hide();
        autoRCM_switch->show();
    }
}

void qTools::on_autoRcmSwitchToggled()
{
    if (!m_device->arianeIsReady())
    {
        emit error(ARIANE_NOT_READY);
        return;
    }

    QtConcurrent::run(m_kourou, &QKourou::setAutoRcmEnabled, autoRCM_switch->getState());
}
