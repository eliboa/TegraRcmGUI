#include "qkourou.h"
#include "tegrarcmgui.h"
#include "qprogress_widget.h"
#include <winioctl.h>
#include <QDateTime>
#include <QThread>
#include <QDebug>

QKourou::QKourou(QWidget *parent, Kourou* device, TegraRcmGUI* gui) : QWidget(parent), m_device(device), parent(parent), m_gui(gui)
{
    qRegisterMetaType<UC_DeviceInfo>("UC_DeviceInfo");
    connect(this, SIGNAL(clb_deviceInfo(UC_DeviceInfo)), parent, SLOT(on_deviceInfo_received(UC_DeviceInfo)));
    connect(this, SIGNAL(clb_error(int)), parent, SLOT(error(int)));
    connect(this, SIGNAL(clb_deviceStateChange()), parent, SLOT(on_deviceStateChange()));
    connect(this, SIGNAL(clb_finished(int)), parent, SLOT(on_Kourou_finished(int)));
    connect(this, SIGNAL(pushMessage(const QString)), parent, SLOT(pushMessage(const QString)));
    connect(this, SIGNAL(initProgressWidget(const QString, int)), gui->m_progressWidget, SLOT(init_ProgressWidget(const QString, int)));
    connect(this, SIGNAL(sendStatusLbl(const QString)), gui->m_progressWidget, SLOT(setLabel(const QString)));
    connect(this, SIGNAL(closeProgressWidget()), gui->m_progressWidget, SLOT(close()));
}

QKourou::~QKourou()
{
    if (m_hekate_ini != nullptr)
        delete m_hekate_ini;
}

void QKourou::initDevice(bool silent, KLST_DEVINFO_HANDLE deviceInfo)
{    
    if (!waitUntilUnlock())
        return;   

    DWORD error = 0;
    bool devInfoSync = false;
    UC_DeviceInfo di;

    setLockEnabled(true);

    if (!m_device->initDevice(deviceInfo))
        error = GetLastError();

    //emit clb_deviceStateChange();

    bool b_autoInject = false;
    if (!error)
    {
        // Do auto inject if needed
        DWORD autoErr = autoInject();
        b_autoInject = !autoErr ? true : false;
        error = autoErr != WRONG_PARAM_GENERIC ? autoErr : error;

        // Auto boot Ariane
        if (!error && !b_autoInject && !m_device->arianeIsReady() && autoLaunchAriane && ariane_bin.size())
        {
            emit initProgressWidget(tr("Loading Ariane..."), 5);
            arianeIsLoading = true;
            emit clb_deviceStateChange();
            error = m_device->hack((u8*)ariane_bin.data(), (u32)ariane_bin.size());            
            QThread::msleep(1000); // Wait for Ariane to be fully loaded
            emit sendStatusLbl(tr("Retrieving device info..."));
            if (!error)
                m_device->arianeIsReady_sync(); //waitUntilArianeReady();

            arianeIsLoading = false;            
        }
    }    

    if (error && !silent)
        emit clb_error(int(error));

    if (!error && b_autoInject)
        emit clb_finished(AUTO_INJECT);

    // Get device info if ariane is ready
    if (m_device->arianeIsReady())        
        devInfoSync = !(m_device->getDeviceInfo(&di));

    if (devInfoSync) emit clb_deviceInfo(di);


    // Read hekate_ipl.ini from device
    if (devInfoSync && di.cbl_hekate)
    {
        emit sendStatusLbl(tr("Retrieving hekate's config"));
        Bytes ini_file;
        u32 bytesRead = 0;
        if(!m_device->sdmmc_readFile("bootloader/hekate_ipl.ini", &ini_file, &bytesRead))
        {
            if (m_hekate_ini != nullptr) delete m_hekate_ini;
            // Create new Hekate ini
            m_hekate_ini = new HekateIni(QByteArray(reinterpret_cast<const char*>(ini_file.data()), ini_file.size()));
            // Set configs ids if needed
            if (m_hekate_ini->setConfigsIds())
            {
                emit sendStatusLbl(tr("Updating hekate's config"));
                std::vector<u8> data(m_hekate_ini->data().constBegin(), m_hekate_ini->data().constEnd());
                m_device->sdmmc_writeFile(&data, "bootloader/hekate_ipl.ini", true);
            }
        }
    }

    emit closeProgressWidget();
    emit clb_deviceStateChange();   
    setLockEnabled(false);
}

DWORD QKourou::autoInject()
{
    if (!autoInjectPayload)
        return WRONG_PARAM_GENERIC;

    QString path = m_gui->userSettings->value("autoInjectPath").toString();
    if (!path.length())
        return WRONG_PARAM_GENERIC;

    if (!m_device->rcmIsReady() && !m_device->arianeIsReady())
        return DEVICE_NOT_READY;

    tmp_string.clear();
    tmp_string.append(path.toStdString());
    DWORD err = 0;

    // Reboot if ariane is loaded
    if (m_device->arianeIsReady() && !rebootToRcm())
        err = GetLastError();

    // Push payload
    if (!err) err = m_device->hack(tmp_string.c_str());

    if (!err && !m_device->deviceIsReady())
        m_device->setRcmReady(false);

    return err;
}

void QKourou::getDeviceInfo()
{
    //qDebug() << "QKourou::getDeviceInfo() execute in " << QThread::currentThreadId();

    if (!waitUntilUnlock())
        return;

    UC_DeviceInfo di;

    setLockEnabled(true);
    int res = m_device->getDeviceInfo(&di);
    setLockEnabled(false);

    if (res)
        emit clb_error(res);
    else
        emit clb_deviceInfo(di);

}

void QKourou::setAutoRcmEnabled(bool state)
{
    if (!waitUntilUnlock())
        return;

    setLockEnabled(true);
    bool res = m_device->setAutoRcmEnabled(state);
    setLockEnabled(false);

    if (!res)
        emit clb_error(FAILED_TO_SET_AUTORCM);
}

void QKourou::hack(const char* payload_path, u8 *payload_buff, u32 buff_size)
{
    if (!waitUntilUnlock())
        return;

    DWORD res = 0;
    setLockEnabled(true);

    // Push payload
    if(!res) res = payload_path != nullptr ? m_device->hack(payload_path) : m_device->hack(payload_buff, buff_size);

    if (!res && !m_device->deviceIsReady())
        m_device->setRcmReady(false);

    setLockEnabled(false);

    if (res)
        emit clb_error(int(res));
    else
        emit clb_finished(PAYLOAD_INJECT);

    emit clb_deviceStateChange();
}

void QKourou::hack(const char* payload_path)
{
    hack(payload_path, nullptr, 0);
}
void QKourou::hack(u8 *payload_buff, u32 buff_size)
{
    hack(nullptr, payload_buff, buff_size);
}

bool QKourou::rebootToRcm()
{
    DWORD err = !m_device->rebootToRcm() ? RCM_REBOOT_FAILED : 0;

    if (!err)
    {
        m_device->disconnect();
        m_device->resetCurrentBuffer();
        if(!waitUntilInit(3))
            err = RCM_REBOOT_FAILED;
    }

    if (err)
        SetLastError(err);

    return !err ? true : false;
}

bool QKourou::waitUntilUnlock(uint timeout_s)
{
    qint64 begin_timestamp = QDateTime::currentSecsSinceEpoch();
    while(isLocked())
    {
        if (QDateTime::currentSecsSinceEpoch() > begin_timestamp + timeout_s)
            return false;
    }
    return true;
}

bool QKourou::waitUntilRcmReady(uint timeout_s)
{
    qint64 begin_timestamp = QDateTime::currentSecsSinceEpoch();
    while(!m_device->deviceIsReady())
    {
        if (QDateTime::currentSecsSinceEpoch() > begin_timestamp + timeout_s)
            return false;
    }
    return true;
}

bool QKourou::waitUntilArianeReady(bool skip_rcm, uint timeout_s)
{
    qint64 begin_timestamp = QDateTime::currentSecsSinceEpoch();
    while(!m_device->arianeIsReady_sync(skip_rcm))
    {
        if (QDateTime::currentSecsSinceEpoch() > begin_timestamp + timeout_s)
            return false;
    }
    return true;
}

bool QKourou::waitUntilInit(uint timeout_s)
{
    qint64 begin_timestamp = QDateTime::currentSecsSinceEpoch();
    while(!m_device->initDevice())
    {
        if (QDateTime::currentSecsSinceEpoch() > begin_timestamp + timeout_s)
            return false;
    }
    return true;
}

void QKourou::initNoDriverDeviceLookUpLoop()
{
    connect(this, SIGNAL(clb_driverMissing()), m_gui->settingsTab, SLOT(on_driverMissing()));
    QTimer *lookup = new QTimer(this);
    connect(lookup, SIGNAL(timeout()), this, SLOT(noDriverDeviceLookUp()));
    lookup->start(1000); // Every second
    m_askForDriverInstall = true;
    m_APX_device_reconnect = true;
}

void QKourou::noDriverDeviceLookUp()
{
    if (m_device->getStatus() == CONNECTED)
        return;

    unsigned index;
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    TCHAR HardwareID[1024];
    bool found = false;
    // List all connected USB devices
    hDevInfo = SetupDiGetClassDevs(nullptr, TEXT("USB"), nullptr, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    for (index = 0; ; index++)
    {
        DeviceInfoData.cbSize = sizeof(DeviceInfoData);
        if (!SetupDiEnumDeviceInfo(hDevInfo, index, &DeviceInfoData))
            break;

        SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, nullptr, (BYTE*)HardwareID, sizeof(HardwareID), nullptr);
        if (_tcsstr(HardwareID, _T("VID_0955&PID_7321")))
        {
            // device found, check driver
            BYTE driverPath[256], zeroBuffer[256];
            memset(driverPath, 0, 256);
            memset(zeroBuffer, 0, 256);
            SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DRIVER, nullptr, driverPath, 256, nullptr);
            if (!memcmp(driverPath, zeroBuffer, 256))
            {
                found = true;
                // Driver not found
                if (!m_askForDriverInstall)
                {
                    if (m_APX_device_reconnect)
                        emit pushMessage(tr("Device detected but driver is missing\nInstall driver from SETTINGS tab"));
                }
                else
                {
                    emit clb_driverMissing();
                    m_askForDriverInstall = false;
                }
                m_APX_device_reconnect = false;
            }
            break;
        }
    }
    if (!found)
        m_APX_device_reconnect = true;
}

void QKourou::copyFiles(QList<Packages::Package*> in_pkgs)
{
    QList<Packages::Package*> pkgs;
    // Load install cpys from Json if necessary
    for (auto pkg : in_pkgs)
    {
        if (pkg->devInstallCpys().empty())
            pkg->getFilesForDeviceInstall();

        if (!pkg->devInstallCpys().empty())
            pkgs.append(pkg);
    }

    if (pkgs.empty())
        return;

    ///LAMBDA: Exit
    auto exit = [=](int err)  {
        setLockEnabled(false);
        if (err)
        {
            for (auto pkg : pkgs)
                pkg->setStatus(INSTALL_FAILED);
            emit clb_error(err);
        }
        emit closeProgressWidget();
    };

    if (!waitUntilUnlock())
        return exit(ARIANE_NOT_READY);

    setLockEnabled(true);

    for (auto pkg : pkgs)
        pkg->setStatus(DEV_INSTALLING);

    emit initProgressWidget(tr("Preparing files"), 15);

    for (auto pkg : pkgs)
    {
        auto cpys = pkg->devInstallCpys(); // Get copy of InstallCpys
        pkg->clearDevInstallCpys(); // Clear InstallCpys in package
        for (auto cpy : cpys)
        {
            QString dest_path = cpy.destination.mid(0, cpy.destination.lastIndexOf('/'));
            if (!m_device->sdmmc_mkPath(dest_path.toLocal8Bit().constData()))
                return exit(FAILED_TO_MKDIR);

            emit sendStatusLbl(tr("Copying ") + cpy.destination);
            int res = m_device->sdmmc_writeFile(cpy.source.toLocal8Bit().constData(), cpy.destination.toLocal8Bit().constData(), true);

            if (res != QFile(cpy.source).size())
                return exit(SD_FILE_WRITE_FAILED);

        }
        pkg->setStatus(INSTALLED);
    }

    emit sendStatusLbl(tr("Installation finished"));
    QThread::msleep(1500);
    UC_DeviceInfo di;
    if(m_device->getDeviceInfo(&di) == SUCCESS)
        emit clb_deviceInfo(di);

    return exit(SUCCESS);
}

void QKourou::installSDFiles(QString input_path, bool ignore_ini)
{
    auto exit = [&](int err)  {
        setLockEnabled(false);
        if (err)
            emit clb_error(err);
        emit closeProgressWidget();
    };

    emit initProgressWidget(tr("Preparing files"), 15);

    bool is_dir = false;
    if (QDir(input_path).exists())
        is_dir = true;
    else if (!QFile(input_path).exists())
        return exit(BAD_ARGUMENT);

    QStringList directories;
    QStringList files;

    if (is_dir)
    {
        QDir dir(input_path);
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        QDirIterator it(dir, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString item = it.next();
            if (QDir(item).exists())
                directories << item;
            else if (QFile(item).exists())
                files << item;
        }
    }
    else files << input_path;

    if (!waitUntilUnlock())
        return exit(ARIANE_NOT_READY);

    setLockEnabled(true);

    // Create dirs
    for (auto dir : directories)
    {
        QString outdir = dir.mid(input_path.length() + 1, dir.length());
        if (!m_device->sdmmc_isDir(outdir.toLocal8Bit().constData()))
        {
            if (!m_device->sdmmc_mkDir(outdir.toLocal8Bit().constData()))
                return exit(FAILED_TO_MKDIR);
        }
    }

    // Create files
    for (auto file : files)
    {        
        QString outfile = file.mid(input_path.length() + 1, file.length());
        if (ignore_ini && outfile.endsWith(".ini") && m_device->sdmmc_fileSize(outfile.toLocal8Bit().constData()))
            continue;

        emit sendStatusLbl(tr("Copying ") + outfile);
        int res = m_device->sdmmc_writeFile(file.toLocal8Bit().constData(), outfile.toLocal8Bit().constData(), true);

        if (res != QFile(file).size())
        {
            return exit(res ? res : SD_FILE_WRITE_FAILED);
        }
    }

    emit sendStatusLbl(tr("Installation finished"));
    QThread::msleep(1500);
    UC_DeviceInfo di;
    if(m_device->getDeviceInfo(&di) == SUCCESS)
        emit clb_deviceInfo(di);

    return exit(SUCCESS);

}

void QKourou::getKeys()
{
    if (!waitUntilUnlock())
        return;

    if (!m_device->arianeIsReady_sync())
        return;

    UC_Header uc;
    uc.command = GET_KEYS;
    // Send command
    m_device->write((const u8*)&uc, sizeof(uc));
    QThread::msleep(500);
}
