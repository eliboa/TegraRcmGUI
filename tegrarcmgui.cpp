#include "tegrarcmgui.h"
#include "ui_tegrarcmgui.h"
#include "qutils.h"
#include <QPoint>
#include <Windows.h>
#include <JlCompress.h>

QNetworkAccessManager *nMgr();

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

    // Init access to builtin resources
    Q_INIT_RESOURCE(qresources);

    // Load settings
    userSettings = new QSettings("nx", "TegraRcmGUI");

    // Start update manager
    /*
    qRegisterMetaType<download_t>("download_t");
    m_um = new UpdateManager(this);
    connect(m_um, &UpdateManager::new_update_available, this, &TegraRcmGUI::updateAvailable);
    //m_um->start();
    */

    // Tray icon init
    trayIcon = new QSystemTrayIcon;
    trayIcon->setIcon(switchOffIcon);
    trayIcon->setVisible(true);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &TegraRcmGUI::trayIconActivated);
    trayIconMenu = trayIcon->contextMenu();

    m_progressWidget = new qProgressWidget("", this);

    // Create a qKourou instance to invoke Kourou methods (asynchronously) using signals and slots
    m_kourou = new QKourou(this, &m_device, this);

    m_kourou->autoLaunchAriane = userSettings->value("autoAriane").toBool();
    m_kourou->autoInjectPayload = userSettings->value("autoInject").toBool();

    // Create tabs
    ui->tabWidget->tabBar()->setCursor(Qt::PointingHandCursor);
    ui->push_layout->setAlignment(Qt::AlignTop);
    ui->pushLayoutWidget->setAttribute(Qt::WA_TransparentForMouseEvents);    
    payloadTab = new QPayloadWidget(this);
    ui->tabWidget->addTab(payloadTab, tr("PAYLOAD"));
    hekateTab = new qHekate(this);
    ui->tabWidget->addTab(hekateTab, tr("HEKATE / AMS"));
    toolsTab = new qTools(this);
    ui->tabWidget->addTab(toolsTab, tr("TOOLS"));
    settingsTab = new qSettings(this);
    ui->tabWidget->addTab(settingsTab, tr("SETTINGS"));
    connect(ui->tabWidget, &QTabWidget::currentChanged, [=](int idx) {
        if (idx == 1) hekateTab->on_tabActivated();
    });

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
    pushTimer->start(1000); // Every second
    m_kourou->initNoDriverDeviceLookUpLoop();

    m_pkgs.setParent(this);

    /// GUI inits
    // Set stylesheets
    this->setStyleSheet(GetStyleSheetFromResFile(":/res/QMainWindow.qss"));
    ui->statusbar->setStyleSheet(GetStyleSheetFromResFile(":/res/QMainWindow.qss"));
    ui->tabWidget->setStyleSheet(GetStyleSheetFromResFile(":/res/QTabWidget.qss"));
    ui->titleBarFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_titleBar.qss"));
    ui->statusBoxFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box01.qss"));
    ui->deviceInfoBoxFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box01.qss"));

    ui->closeAppBtn->setCursor(Qt::PointingHandCursor);
    connect(ui->closeAppBtn, SIGNAL(clicked()), this, SLOT(on_appClose()));
    ui->minimizeAppBtn->setCursor(Qt::PointingHandCursor);
    connect(ui->minimizeAppBtn, SIGNAL(clicked()), this, SLOT(showMinimized()));

    clearDeviceInfo();
    ui->di_tableWidget->setColumnCount(2);
    ui->di_tableWidget->setColumnWidth(0, 95);
    ui->di_tableWidget->setColumnWidth(1, 75);
    ui->di_tableWidget->horizontalScrollBar()->setStyleSheet("QScrollBar {height:0px;}");
    ui->di_tableWidget->setStyleSheet(GetStyleSheetFromResFile(":/res/QTableWidget.qss"));

    Switch *_switch = new Switch(m_kourou->autoLaunchAriane ? true : false, 52);
    ui->verticalLayout->addWidget(_switch);
    connect(_switch, SIGNAL(clicked(bool)), this, SLOT(on_autoLaunchAriane_toggled(bool)));

    ui->centralwidget->setFixedSize(690, 400);
    this->adjustSize();
}

TegraRcmGUI::~TegraRcmGUI()
{
    if (m_hotHandle != nullptr)
        HotK_Free(m_hotHandle);

    delete trayIcon;
    delete ui;

    //QMainWindow::~QMainWindow();
}

void TegraRcmGUI::updateAvailable()
{
    /*
    auto queue = m_um->getDownloadQueue();
    if (!queue.size())
        return;

    QString message;
    QTextStream messageStr(&message);
    messageStr << tr("New updates available :\n\n");
    QString buff;
    for (auto item : queue)
    {
        if (item.dest_path != buff)
            messageStr <<  item.dest_path << "\n";

        messageStr << "- " << QFileInfo(item.url.path()).fileName() << "\n";
        buff = item.dest_path;
    }
    if(QMessageBox::question(this, "Update", message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        //QtConcurrent::run(m_um, &UpdateManager::download_all);
        //m_um->download_all();
        auto dialog = new UM_Dialog(m_um, this);
        dialog->exec();
    }
    */
}

void TegraRcmGUI::on_appClose()
{
    if (userSettings->value("minToTray").toBool())
    {
        showMinimized();
        trayIcon->showMessage("", tr("TegraRcmGUI is still running in system tray"), m_device.isReadyToReceivePayload() ? switchOnIcon : switchOffIcon, 3000);
    }
    else close();
}

void TegraRcmGUI::changeEvent(QEvent* e)
{
    int t;
    QEvent::Type typ = e->type();
    switch (e->type())
    {
        case QEvent::LanguageChange:
            this->ui->retranslateUi(this);
            break;
        case QEvent::WindowStateChange:
        {
            if (this->windowState() & Qt::WindowMinimized)
            {
                if (userSettings->value("minToTray").toBool())
                    QTimer::singleShot(250, this, SLOT(hide()));
            }
            this->adjustSize();
            break;
        }
        default:
            break;
    }

    QMainWindow::changeEvent(e);
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
    // Device status box
    ui->devStatusLbl_2->setText(tr("DEVICE STATUS"));
    ui->devStatusFrame->setStyleSheet(m_device.getStatus() == CONNECTED ? statusOnStyleSht : statusOffStyleSht);
    ui->rcmStatusLbl_2->setText(m_device.rcmIsReady() ? tr("READY") : tr("OFF"));
    QString arianeStatus = "", arianeStyle = "";
    if (m_kourou->arianeIsLoading)
    {
        arianeStatus.append(tr("LOADING"));
        arianeStyle = statusOffStyleSht;
    }
    else if (m_device.arianeIsReady())
    {
        arianeStatus.append(tr("READY"));
        arianeStyle = statusOnStyleSht;
    }
    else
    {
        arianeStatus.append(tr("OFF"));
        arianeStyle = m_kourou->autoLaunchAriane ? statusOffRedStyleSht : statusOffStyleSht;
    }
    ui->arianeStatusLbl_2->setText(arianeStatus);
    QString style = statusOffRedStyleSht;
    if (m_device.arianeIsReady())
        style = statusOffStyleSht;
    else if (m_device.rcmIsReady())
        style = statusOnStyleSht;
    ui->rcmStatusFrame->setStyleSheet(style);
    ui->arianeStatusFrame->setStyleSheet(arianeStyle);
    if (!m_device.arianeIsReady())
        clearDeviceInfo();

    // Icons
    if (m_device.isReadyToReceivePayload())
    {
        trayIcon->setIcon(switchOnIcon);
        setWindowIcon(switchOnIcon);
    }
    else
    {
        trayIcon->setIcon(switchOffIcon);
        setWindowIcon(switchOffIcon);
    }

    if (m_device.arianeIsReady() && m_pkgs.hasPendingDevInstalls())
        m_pkgs.devInstall();

    payloadTab->on_deviceStateChange();
    toolsTab->on_deviceStateChange();
    hekateTab->on_deviceStateChange();

}

void TegraRcmGUI::on_autoLaunchAriane_toggled(bool value)
{
    m_kourou->autoLaunchAriane = !m_kourou->autoLaunchAriane;
    userSettings->setValue("autoAriane", m_kourou->autoLaunchAriane);

    on_deviceStateChange();

    if (!m_kourou->autoLaunchAriane)
        return;

    if (m_device.rcmIsReady())
        QtConcurrent::run(m_kourou, &QKourou::initDevice, true, nullptr);
    else if (!m_device.arianeIsReady())
        pushMessage((m_device.getStatus() == CONNECTED ? tr("Reboot") : tr("Boot")) + tr(" device to RCM to launch Ariane"));
}

bool TegraRcmGUI::enableWidget(QWidget *widget, bool enable)
{
    widget->setEnabled(enable);
    return true;
}

void TegraRcmGUI::clearDeviceInfo()
{
    ui->deviceInfoFrame->setStyleSheet(statusOffStyleSht);
    ui->deviceInfoLbl->setStyleSheet(statusOffStyleSht);    

    ui->di_tableWidget->setRowCount(0);
    if (!m_kourou->autoLaunchAriane)
    {
        ui->devInfoDisableLbl->show();
    }
    else ui->devInfoDisableLbl->hide();
}

void TegraRcmGUI::on_deviceInfo_received(UC_DeviceInfo di)
{

    ui->devInfoDisableLbl->hide();
    ui->deviceInfoLbl->setStyleSheet(GetStyleSheetFromResFile(":/res/QLabel_title01.qss"));

    ui->di_tableWidget->setRowCount(0);
    auto addTableWidgetRow = [&](const QString &label, const QString &value, bool tooltip = false) {
        auto di_t = ui->di_tableWidget;
        di_t->insertRow(di_t->rowCount());
        auto label_i = new QTableWidgetItem(label + ":");
        label_i->setTextAlignment(Qt::AlignRight |Qt::AlignVCenter);
        label_i->setFlags(label_i->flags() ^ Qt::ItemIsEditable);
        auto value_i = new QTableWidgetItem(value);
        value_i->setTextAlignment(Qt::AlignLeft |Qt::AlignVCenter);
        value_i->setFlags(value_i->flags() ^ Qt::ItemIsEditable);
        if (tooltip) value_i->setToolTip(value);
        di_t->setItem(di_t->rowCount()-1, 0, label_i);
        di_t->setItem(di_t->rowCount()-1, 1, value_i);
    };

    if (strlen(di.deviceId))
        addTableWidgetRow(tr("NX ID"), QString(di.deviceId), true);

    addTableWidgetRow(tr("Battery charge"), QString().asprintf("%u%%", di.battery_capacity));
    addTableWidgetRow(tr("Burnt fuses"), QString().asprintf("%u", di.burnt_fuses));

    if (strlen(di.fw_version))
        addTableWidgetRow(tr("Firmware (SYS)"), QString(di.fw_version) + (di.exFat_driver ? tr(" exFAT") : ""), true);

    if (di.emunand_type)
        addTableWidgetRow(tr("Emunand"), di.emunand_type == FILEBASED ? tr("File based") : tr("RAW based"));

    if (strlen(di.emu_fw_version))
        addTableWidgetRow(tr("Firmware (EMU)"), QString(di.emu_fw_version) + (di.emu_exFat_driver ? tr(" exFAT") : ""), true);

    if (di.sdmmc_initialized)
    {
        QString fs;
        if (di.mmc_fs_type == FS_FAT32) fs.append("FAT32");
        else if (di.mmc_fs_type == FS_EXFAT) fs.append("exFAT");
        else fs.append("UNKNOWN");
        addTableWidgetRow(tr("SD Filesystem"), fs);

        qint64 fs_size = 0x200 * (qint64)di.mmc_fs_cl_size * ((qint64)di.mmc_fs_last_cl + 1);
        qint64 fs_free_space = 0x200 * (qint64)di.mmc_fs_cl_size * (qint64)di.mmc_fs_free_cl;
        addTableWidgetRow(tr("FS Total size"), GetReadableSize(fs_size));
        if (fs_free_space)
            addTableWidgetRow(tr("FS Free space"), GetReadableSize(fs_free_space));
    }   

    if (di.autoRCM != toolsTab->autoRCM_switch->isActive())
        toolsTab->autoRCM_switch->toggle();


    toolsTab->on_deviceStateChange();
    hekateTab->on_deviceInfo_received();
}

void TegraRcmGUI::error(int error)
{
    if (error == FAILED_TO_SET_AUTORCM)
        toolsTab->autoRCM_switch->toggle();

    QString err_label;
    for (ErrorLabel item : ErrorLabelArr)
    {
        if (item.error == error)
            err_label = item.label;
    }

    if (!err_label.size())
        err_label.append(QString().asprintf("Error %d", error));

    if (!isVisible())
        trayIcon->showMessage("", err_label, QSystemTrayIcon::Critical, 3000);
    else
        QMessageBox::critical(this, "Error", err_label);
}

void TegraRcmGUI::pushMessage(const QString message)
{
    // Send message from tray if app is not visible
    if (!isVisible())
    {
        trayIcon->showMessage("", message, QSystemTrayIcon::Information, 3000);
        return;
    }

    // Otherwise, push in-app message
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
        showNormal();
        activateWindow();
        break;
    case QSystemTrayIcon::DoubleClick:
        break;
    case QSystemTrayIcon::MiddleClick:
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

    if (nullptr != trayIconMenu)
        delete trayIconMenu;

    trayIconMenu = new QMenu;
    trayIconMenu->setStyleSheet(menu_ss);
    trayIconMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    if (!isVisible())
        trayIconMenu->addAction("Show",this,SLOT(showNormal()));
    trayIconMenu->addAction("Close",this,SLOT(close()));
    trayIconMenu->addSeparator();

    if (m_device.isReadyToReceivePayload())
    {
        QMenu *fav_menu = new QMenu;
        fav_menu->setStyleSheet(menu_ss);
        fav_menu->setContextMenuPolicy(Qt::CustomContextMenu);
        fav_menu->setTitle("Favorites");
        QSignalMapper* signalmapper = new QSignalMapper();
        for (payload_t payload : payloadTab->getPayloads())
        {

            QAction *action = new QAction(payload.name);
            connect (action, SIGNAL(triggered()), signalmapper, SLOT(map()));
            signalmapper ->setMapping (action, payload.path);
            connect (signalmapper , SIGNAL(mapped(QString)), payloadTab, SLOT(on_injectPayload(QString)));
            fav_menu->addAction(action);
        }
        trayIconMenu->addMenu(fav_menu);
    }
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->contextMenu()->popup(QCursor::pos());
}
