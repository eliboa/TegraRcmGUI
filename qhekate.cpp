#include "tegrarcmgui.h"
#include "qhekate.h"
#include "ui_qhekate.h"
#include "qobjects/custombutton.h"
#include "qprogress_widget.h"

qHekate::qHekate(TegraRcmGUI *parent) : QWidget(parent), ui(new Ui::qHekate), parent(parent)
{
    ui->setupUi(this);
    m_kourou = parent->m_kourou;
    m_device = &parent->m_device;

    // Connect signals & slots
    connect(this, SIGNAL(error(int)), parent,  SLOT(error(int)));

    initHekatePayload();

    // Styles
    ui->umsFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box02.qss"));
    ui->configFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box02.qss"));
    ui->umsLayout->setAlignment(Qt::AlignLeft);
    ui->headerLayout->setAlignment(Qt::AlignLeft);
    ui->configLayout->setAlignment(Qt::AlignRight);

    ui->hconfigTableWidget->setColumnCount(2);
    ui->hconfigTableWidget->setColumnWidth(0, 110);
    ui->hconfigTableWidget->setColumnWidth(1, 120);
    ui->hconfigTableWidget->horizontalScrollBar()->setStyleSheet("QScrollBar {height:0px;}");
    ui->hconfigTableWidget->setStyleSheet(GetStyleSheetFromResFile(":/res/QTableWidget.qss"));
    ui->hconfigTableWidget->setAlternatingRowColors(true);

    // Meta types
    qRegisterMetaType<nyx_ums_type>("nyx_ums_type");

    // Draw boxes
    drawHeader();
    drawUmsBox();

    /*
     * TODO
     * - Que faire si aucun payload ?
     *
     */

}

qHekate::~qHekate()
{
    delete ui;
}

bool qHekate::initHekatePayload()
{
    auto hekate_pkg = parent->m_pkgs.get("Hekate");
    if (!hekate_pkg || !hekate_pkg->exists() || !hekate_pkg->payload().length())
        return false;

    QFile payload(hekate_pkg->payload());
    payload.open(QIODevice::ReadOnly);
    if (!payload.isOpen())
        return false;

    m_hekate_payload = payload.readAll();
    payload.close();

    // Check nyx bin & get version
    QFile nyx_bin(hekate_pkg->location() + "/bootloader/sys/nyx.bin");
    nyx_bin.open(QIODevice::ReadOnly);
    if (nyx_bin.isOpen())
    {
        nyx_bin.seek(0x99);
        QString magic(nyx_bin.read(3));
        if (magic == "CTC")
        {
            m_nyx_version.major = QString(nyx_bin.read(1)).toInt();
            m_nyx_version.minor = QString(nyx_bin.read(1)).toInt();
            m_nyx_version.micro = QString(nyx_bin.read(1)).toInt();
        }
        nyx_bin.close();
    }

    return true;
}

void qHekate::on_tabActivated()
{
    //new WarningBox("TEST", ui->fullLayout);
}

void qHekate::on_deviceStateChange()
{
    drawHeader();
    drawUmsBox();
    drawConfigBox();
}

void qHekate::on_deviceInfo_received()
{
    drawHeader();
    drawUmsBox();
}

void qHekate::drawHeader()
{
    // Clear all items in layout
    ClearLayout(ui->headerLayout);

    auto hekate_pkg = parent->m_pkgs.get("Hekate");
    // Hekate's badge
    if (m_hekate_payload.size() && hekate_pkg && hekate_pkg->version().length())
    {
        Badge *hekate_badge = new Badge("Hekate", hekate_pkg->version(), this);
        hekate_badge->setStatusTip(tr("Hekate's payload location: ") + hekate_pkg->payload());
        ui->headerLayout->addWidget(hekate_badge);
    }

    auto di = m_device->deviceInfo();
    if (!m_device->isDeviceInfoAvailable() || !di.sdmmc_initialized || !m_hekate_payload.size())
        return;

    // Nyx & AMS badges
    AppVersion nyx_ver;
    nyx_ver.major = QString(di.nyx_version[0]).toInt();
    nyx_ver.minor = QString(di.nyx_version[1]).toInt();
    nyx_ver.micro = QString(di.nyx_version[2]).toInt();
    Badge *nyx_badge = new Badge("Nyx", !di.cbl_nyx ? "Not found" : GetAppVersionAsQString(nyx_ver), this);
    ui->headerLayout->addWidget(nyx_badge);

    AppVersion ams_ver;
    ams_ver.major = di.ams_version[0];
    ams_ver.minor = di.ams_version[1];
    ams_ver.micro = di.ams_version[2];
    Badge *ams_badge = new Badge("AMS", !di.cfw_ams ? "Not found" : GetAppVersionAsQString(ams_ver), this);
    ui->headerLayout->addWidget(ams_badge);

    // Header button lambda
    auto createHeaderButton = [&](const QString& label, void(qHekate::* slotName)())
    {
        auto *button = new CustomButton(parent, label);
        button->addEnableCondition(C_ARIANE_READY);
        button->setFixedHeight(22);
        button->setStyleSheet("padding: 5px;");
        connect(button, &QPushButton::clicked, this, slotName);
        button->setStatusTip(tr("%1 on device (i.e needed files will be copied to SD card)").arg(label));
        ui->headerLayout->addWidget(button);
    };
    // Create buttons
    createHeaderButton(tr("Install Hekate/Nyx"), &qHekate::on_hekate_install);
    createHeaderButton(tr("Install AMS"), &qHekate::on_ams_install);
}

void qHekate::drawConfigBox()
{
    ClearLayout(ui->configLayout);
    if (!m_kourou->hekate_ini)
        return;

    auto cfg_list = ui->configComboBox;
    auto configs = m_kourou->hekate_ini->configs()->data();

    cfg_list->clear();
    for (auto config : configs)
    {
        QVariant id = config->getValue("id");
        if (id.toString().size())
            cfg_list->addItem(config->name(), id);
    }

    if (cfg_list->count())
    {
        auto *button = new CustomButton(parent, "Launch config");
        button->setFixedSize(100, 22);
        connect(button, &QPushButton::clicked, [=]() {
            emit on_launchHekateConfig();
        });
        button->setStatusTip(tr("Launch selected config"));
        button->addEnableCondition(C_READY_FOR_PAYLOAD);
        ui->configLayout->addWidget(button, 0, 0);
    }
}

void qHekate::drawUmsBox()
{
    // Clear all items in layout
    ClearLayout(ui->umsLayout);

    if (!m_device->arianeIsReady())
    {
        QString label(tr("Ariane needs to be loaded.\n"));
        label.append(m_kourou->autoLaunchAriane ? tr("Boot device to RCM") : tr("Enable Ariane autoboot first"));
        new WarningBox(label, ui->umsLayout);
        return;
    }

    auto di = m_device->deviceInfo();

    // We need available device infos + nyx installed on device + ariane loaded and ready
    if (!m_device->isDeviceInfoAvailable() || !di.cbl_nyx)
        return;

    // UMS button lambda
    auto createUmsButton = [&](const QString&  storage, nyx_ums_type ums_type, int row, int col)
    {
        auto *button = new CustomButton(parent, storage, LIGHT);
        button->setFixedSize(100, 22);
        connect(button, &QPushButton::clicked, [=]() {
            emit on_launchHekateUms(ums_type);
        });
        button->setStatusTip(tr("Mount %1 as an external hard drive").arg(storage));
        ui->umsLayout->addWidget(button, row, col);
    };

    // Create UMS buttons
    if (di.sdmmc_initialized) createUmsButton(tr("SD Card"), NYX_UMS_SD_CARD, 0, 0);
    createUmsButton(tr("eMMC RAWNAND"), NYX_UMS_EMMC_GPP,   0, 1);
    createUmsButton(tr("eMMC BOOT0"),   NYX_UMS_EMMC_BOOT0, 0, 2);
    createUmsButton(tr("eMMC BOOT1"),   NYX_UMS_EMMC_BOOT1, 0, 3);

    if (!di.emunand_enabled)
        return;

    createUmsButton(tr("EMU RAWNAND"), NYX_UMS_EMUMMC_GPP,   1, 0);
    createUmsButton(tr("EMU BOOT0"),   NYX_UMS_EMUMMC_BOOT0, 1, 1);
    createUmsButton(tr("EMU BOOT1"),   NYX_UMS_EMUMMC_BOOT0, 1, 2);
}

void qHekate::on_launchHekateUms(nyx_ums_type type)
{
    if (!m_hekate_payload.size())
        return;

    // Clear boot config
    for (int i=0x94; i < 0x105; i++)
        m_hekate_payload[i] = 0;

    // Set boot config
    m_hekate_payload[0x97] = EXTRA_CFG_NYX_UMS;
    m_hekate_payload[0x98] = type;

    QtConcurrent::run(m_kourou, &QKourou::hack, (u8*)m_hekate_payload.data(), (u32)m_hekate_payload.size());
}

void qHekate::on_launchHekateConfig()
{
    if (!m_hekate_payload.size())
        return;

    QString id = ui->configComboBox->currentData().toString();
    if (!id.size())
    {
        parent->pushMessage(tr("No config selected"));
        return;
    }

    // Clear boot config
    for (int i=0x94; i < 0x105; i++)
        m_hekate_payload[i] = 0;

    // Set boot config
    m_hekate_payload[0x94] = BOOT_CFG_AUTOBOOT_EN | BOOT_CFG_FROM_ID;
    m_hekate_payload.replace(0x98, id.size() < 8 ? id.size() : 8, id.toLocal8Bit().constData());

    QtConcurrent::run(m_kourou, &QKourou::hack, (u8*)m_hekate_payload.data(), (u32)m_hekate_payload.size());
}


void qHekate::on_hekate_install()
{
    QString path = QFileInfo(m_payloads.at(0).file_path).absolutePath();
    QtConcurrent::run(m_kourou, &QKourou::installSDFiles, path, false);
}

void qHekate::on_ams_install()
{
    QtConcurrent::run(m_kourou, &QKourou::installSDFiles, QString("atmosphere"), true);
}


void qHekate::on_configComboBox_currentIndexChanged(int index)
{
    QString id = ui->configComboBox->currentData().toString();
    auto tw = ui->hconfigTableWidget;
    tw->setRowCount(0);

    if (!m_kourou->hekate_ini)
        return;

    auto config = m_kourou->hekate_ini->configs()->getConfigById(id);
    if (!config)
        return;

    for (auto entry : config->entries())
    {
        tw->insertRow(tw->rowCount());
        auto label_i = new QTableWidgetItem(entry.name);
        label_i->setTextAlignment(Qt::AlignLeft |Qt::AlignVCenter);
        label_i->setFlags(label_i->flags() ^ Qt::ItemIsEditable);
        auto value_i = new QTableWidgetItem(entry.value.toString());
        value_i->setTextAlignment(Qt::AlignLeft |Qt::AlignVCenter);
        tw->setItem(tw->rowCount()-1, 0, label_i);
        tw->setItem(tw->rowCount()-1, 1, value_i);
    }
    tw->resizeColumnsToContents();
}
