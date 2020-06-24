#include "tegrarcmgui.h"
#include "ui_tegrarcmgui.h"
#include "qutils.h"
#include <QPoint>

QMouseEvent MouseLeftButtonEvent(QEvent::MouseButtonPress, QPoint(0,0), Qt::LeftButton, nullptr, nullptr);

TegraRcmGUI * TegraRcmGUI::m_instance;
static void KUSB_API HotPlugEventCallback(KHOT_HANDLE Handle, KLST_DEVINFO_HANDLE DeviceInfo, KLST_SYNC_FLAG flag)
{
    Q_ASSERT(TegraRcmGUI::hasInstance());
    if (DeviceInfo == nullptr || (DeviceInfo->Common.Vid != RCM_VID && DeviceInfo->Common.Pid != RCM_PID))
        return;

    if (flag == KLST_SYNC_FLAG_ADDED)
        TegraRcmGUI::instance()->emit sign_hotPlugEvent(true, DeviceInfo);
    else if (flag == KLST_SYNC_FLAG_REMOVED)
        TegraRcmGUI::instance()->emit sign_hotPlugEvent(false, DeviceInfo);

}

TegraRcmGUI::TegraRcmGUI(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TegraRcmGUI)
{
    ui->setupUi(this);

    // Set a static instance to handle hotplug callback
    Q_ASSERT(!hasInstance());
    m_instance = this;

    // Init acces to builtin resources
    Q_INIT_RESOURCE(qresources);

    // Tray icon init
    trayIcon = new QSystemTrayIcon;
    trayIcon->setIcon(switchOffIcon);
    trayIcon->setVisible(true);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &TegraRcmGUI::trayIconActivated);
    trayIconMenu = trayIcon->contextMenu();

    // Load settings
    userSettings = new QSettings("nx", "TegraRcmGUI");

    // Create a qKourou instance to invoke Kourou methods (asynchronously) using signals and slots
    m_kourou = new QKourou(this, &m_device, this);

    m_kourou->autoLaunchAriane = userSettings->value("autoAriane").toBool();
    m_kourou->autoInjectPayload = userSettings->value("autoInject").toBool();

    // Load builtin Ariane payload
    QFile file(":/ariane_bin");
    if (file.open(QIODevice::ReadOnly))
    {
        m_kourou->ariane_bin = file.readAll();
        file.close();
    }

    // Init device at startup (in a concurrent thread)
    QtConcurrent::run(m_kourou, &QKourou::initDevice, true, nullptr);

    // Set the USB hotplug event
    qRegisterMetaType<KLST_DEVINFO_HANDLE>("KLST_DEVINFO_HANDLE");
    QObject::connect(this, SIGNAL(sign_hotPlugEvent(bool, KLST_DEVINFO_HANDLE)), this, SLOT(hotPlugEvent(bool, KLST_DEVINFO_HANDLE)));
    KHOT_PARAMS hotParams;
    memset(&hotParams, 0, sizeof(hotParams));
    hotParams.OnHotPlug = HotPlugEventCallback;
    hotParams.Flags = KHOT_FLAG_NONE;
    HotK_Init(&m_hotHandle, &hotParams);

    // Set timers
    QTimer *devInfoTimer = new QTimer(this);
    connect(devInfoTimer, SIGNAL(timeout()), this, SLOT(deviceInfoTimer()));
    devInfoTimer->start(60000*5); // Every 5 minutes
    QTimer *pushTimer = new QTimer(this);
    connect(pushTimer, SIGNAL(timeout()), this, SLOT(pushTimer()));
    pushTimer->start(1000); // Every minute


    /// GUI inits
    // Set stylesheets
    this->setStyleSheet(GetStyleSheetFromResFile(":/res/QMainWindow.qss"));
    ui->statusbar->setStyleSheet(GetStyleSheetFromResFile(":/res/QMainWindow.qss"));
    ui->tabWidget->setStyleSheet(GetStyleSheetFromResFile(":/res/QTabWidget.qss"));
    ui->titleBarFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_titleBar.qss"));
    ui->statusBoxFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box01.qss"));
    ui->deviceInfoBoxFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box01.qss"));

    // Init tabs
    ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);
    ui->push_layout->setAlignment(Qt::AlignTop);
    ui->pushLayoutWidget->setAttribute(Qt::WA_TransparentForMouseEvents);

    payloadTab = new QPayloadWidget(this);        
    ui->tabWidget->addTab(payloadTab, tr("PAYLOAD"));
    toolsTab = new qTools(this);
    ui->tabWidget->addTab(toolsTab, tr("TOOLS"));

    ui->closeAppBtn->setCursor(Qt::PointingHandCursor);
    connect(ui->closeAppBtn, SIGNAL(clicked()), this, SLOT(close()));

    clearDeviceInfo();
    Switch *_switch = new Switch(m_kourou->autoLaunchAriane ? true : false, 52);
    ui->verticalLayout->addWidget(_switch);
    connect(_switch, SIGNAL(clicked(bool)), this, SLOT(on_autoLaunchAriane_toggled(bool)));

    ui->centralwidget->setFixedSize(680, 400);
    this->adjustSize();
}

TegraRcmGUI::~TegraRcmGUI()
{
    if (m_hotHandle != nullptr)
        HotK_Free(m_hotHandle);

    delete trayIcon;
    delete ui;
}

void TegraRcmGUI::hotPlugEvent(bool added, KLST_DEVINFO_HANDLE deviceInfo)
{
    if (added)
        QtConcurrent::run(m_kourou, &QKourou::initDevice, true, deviceInfo);
    else
    {
        m_device.disconnect();
        on_deviceStateChange();
    }
}

void TegraRcmGUI::deviceInfoTimer()
{
    if (m_device.arianeIsReady())
        QtConcurrent::run(m_kourou, &QKourou::getDeviceInfo);
}

void TegraRcmGUI::on_deviceStateChange()
{
    ui->devStatusLbl_2->setText(m_device.getStatus() == CONNECTED ? tr("CONNECTED") : tr("DISCONNECTED"));
    ui->devStatusFrame->setStyleSheet(m_device.getStatus() == CONNECTED ? statusOnStyleSht : statusOffStyleSht);
    ui->rcmStatusLbl_2->setText(m_device.rcmIsReady() ? tr("READY") : tr("OFF"));
    QString arianeStatus;
    if (m_kourou->arianeIsLoading)
        arianeStatus.append(tr("LOADING"));
    else if (m_device.arianeIsReady())
        arianeStatus.append(tr("READY"));
    else
        arianeStatus.append(tr("OFF"));
    ui->arianeStatusLbl_2->setText(arianeStatus);
    ui->rcmStatusFrame->setStyleSheet(m_device.rcmIsReady() ? statusOnStyleSht : statusOffStyleSht);
    ui->arianeStatusFrame->setStyleSheet(m_device.arianeIsReady() ? statusOnStyleSht : statusOffStyleSht);
    if (!m_device.arianeIsReady())
        clearDeviceInfo();

    if (m_device.rcmIsReady() || m_device.arianeIsReady())
        trayIcon->setIcon(switchOnIcon);
    else
        trayIcon->setIcon(switchOffIcon);

    payloadTab->on_deviceStateChange();

}

void TegraRcmGUI::on_autoLaunchAriane_toggled(bool value)
{
    m_kourou->autoLaunchAriane = !m_kourou->autoLaunchAriane;
    userSettings->setValue("autoAriane", m_kourou->autoLaunchAriane);

    if (m_device.rcmIsReady() && m_kourou->autoLaunchAriane)
        QtConcurrent::run(m_kourou, &QKourou::initDevice, true, nullptr);
}

bool TegraRcmGUI::enableWidget(QWidget *widget, bool enable)
{
    widget->setEnabled(enable);
}

void TegraRcmGUI::clearDeviceInfo()
{
    ui->deviceInfoFrame->setStyleSheet(statusOffStyleSht);
    ui->deviceInfoLbl->setStyleSheet(statusOffStyleSht);
    ui->batteryLevel->hide();
    ui->batteryLevel->setValue(0);
    ui->burntFusesLbl2->setText("");
    ui->sdfsLbl2->setText("");
    ui->sdfsLbl2->setText("");
    ui->fsTotSizeLbl2->setText("");
    ui->fsFreeSpaceLbl2->setText("");
    if (!m_kourou->autoLaunchAriane)
    {
        ui->batteryLbl->hide();
        ui->burntFusesLbl1->hide();
        ui->sdfsLbl1->hide();
        ui->fsTotSizeLbl1->hide();
        ui->fsFreeSpaceLbl1->hide();
        ui->devInfoDisableLbl->show();
    }
    else ui->devInfoDisableLbl->hide();
}

void TegraRcmGUI::on_deviceInfo_received(UC_DeviceInfo di)
{
    ui->batteryLbl->show();
    ui->burntFusesLbl1->show();
    ui->sdfsLbl1->show();
    ui->fsTotSizeLbl1->show();
    ui->fsFreeSpaceLbl1->show();
    ui->devInfoDisableLbl->hide();
    ui->deviceInfoLbl->setStyleSheet(GetStyleSheetFromResFile(":/res/QLabel_title01.qss"));
    ui->batteryLevel->show();
    ui->batteryLevel->setValue(int(di.battery_capacity));
    ui->burntFusesLbl2->setText(QString().asprintf("%u", di.burnt_fuses));
    ui->sdfsLbl2->setText(QString().asprintf("%s", di.sdmmc_initialized ? "Yes" : "No"));
    if (di.sdmmc_initialized)
    {
        QString fs;
        if (di.emmc_fs_type == FS_FAT32) fs.append("FAT32");
        else if (di.emmc_fs_type == FS_EXFAT) fs.append("exFAT");
        else fs.append("UNKNOWN");
        ui->sdfsLbl2->setText(fs);

        qint64 fs_size = 0x200 * (qint64)di.emmc_fs_cl_size * ((qint64)di.emmc_fs_last_cl + 1);
        qint64 fs_free_space = 0x200 * (qint64)di.emmc_fs_cl_size * (qint64)di.emmc_fs_free_cl;
        ui->fsTotSizeLbl2->setText(GetReadableSize(fs_size));
        ui->fsFreeSpaceLbl2->setText(GetReadableSize(fs_free_space));
    }
    else
    {
        ui->sdfsLbl2->setText("N/A");
        ui->fsTotSizeLbl2->setText("N/A");
        ui->fsFreeSpaceLbl2->setText("N/A");
    }
}

void TegraRcmGUI::error(int error)
{
    QMessageBox::critical(this, "Error", QString().asprintf("Error %d", error));
}

void TegraRcmGUI::pushMessage(QString message)
{
    AnimatedLabel *push = new AnimatedLabel;
    QPropertyAnimation *animation = new QPropertyAnimation(push, "color");
    animation->setDuration(800);
    animation->setStartValue(QColor(0, 150, 136));
    animation->setEndValue(QColor(202, 202, 202));
    push->setText(message);
    push->setMinimumHeight(40);
    push->setMaximumHeight(80);
    ui->push_layout->addWidget(push);
    animation->start();

    push_ts.push_back(QDateTime::currentSecsSinceEpoch());
}

void TegraRcmGUI::pushTimer()
{

    qint64 now = QDateTime::currentSecsSinceEpoch();

    if (tsToDeleteCount && push_ts.count() == 0)
    {
        for (int i(0); i < tsToDeleteCount; i++)
            ui->pushLayoutWidget->findChildren<QLabel*>().at(i)->deleteLater();

        tsToDeleteCount = 0;
    }

    if (!push_ts.size())
        return;

    int i(0);
    int j(0);
    for (qint64 ts : push_ts)
    {
        if (now > ts + 5)
        {
            auto item = ui->pushLayoutWidget->findChildren<QLabel*>().at(j + tsToDeleteCount);
            QPropertyAnimation *animation = new QPropertyAnimation(item, "geometry");
            //QPropertyAnimation *animation = new QPropertyAnimation(item, "offset");
            animation->setDuration(500);
            int r = item->pos().y() + item->height() - 10;
            if (lastTsToDelete)
                r -= lastTsToDelete * item->pos().y();
            animation->setStartValue(QRect(item->pos().x(), item->pos().y(), ui->pushLayoutWidget->width(), r));
            animation->setEndValue(QRect(ui->pushLayoutWidget->width(), item->pos().y(), ui->pushLayoutWidget->width(), r));
            animation->start();
            lastTsToDelete = ts;
            i++;
            break;
        }
        j++;
    }
    push_ts.erase(push_ts.begin(), push_ts.begin() + i);
    tsToDeleteCount += i;
}

void TegraRcmGUI::on_Kourou_finished(int res)
{
    if (res == AUTO_INJECT || res == PAYLOAD_INJECT)
        pushMessage(tr("Payload successfully injected"));
}

void TegraRcmGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{

    int t;
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        t = 0;
        break;
    case QSystemTrayIcon::DoubleClick:
        t = 0;
        break;
    case QSystemTrayIcon::MiddleClick:
        t = 0;
        break;
    case QSystemTrayIcon::Context:
        drawTrayContextMenu();
        break;
    default:
        ;
    }
}

void TegraRcmGUI::drawTrayContextMenu()
{
    //trayIconMenu->addAction("Exit",this,SLOT(close()));
    QString menu_ss = "QMenu::item {"
                      "padding: 4px 20px 4px 4px;"
                      "}"
                      "QMenu::item:selected {"
                      "background-color: rgb(0, 85, 127);"
                      "color: rgb(255, 255, 255);"
                      "}";


    QMenu *menu = new QMenu;
    menu->setStyleSheet(menu_ss);
    menu->setContextMenuPolicy(Qt::CustomContextMenu);
    menu->addAction("Exit",this,SLOT(close()));

    QMenu *fav_menu = new QMenu;
    fav_menu->setStyleSheet(menu_ss);
    fav_menu->setContextMenuPolicy(Qt::CustomContextMenu);
    fav_menu->setTitle("Favorites");

    for (payload_t payload : payloadTab->getPayloads())
    {
        QMenu *p_menu = new QMenu;
        p_menu->setStyleSheet(menu_ss);
        p_menu->setContextMenuPolicy(Qt::CustomContextMenu);
        p_menu->setTitle(payload.name);
        fav_menu->addMenu(p_menu);
    }

    menu->addMenu(fav_menu);

    trayIcon->setContextMenu(menu);
    trayIcon->contextMenu()->popup(QCursor::pos());
}
