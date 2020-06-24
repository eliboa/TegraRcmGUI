#include "qkourou.h"
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

    emit clb_deviceStateChange();

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
            arianeIsLoading = true;
            error = m_device->hack((u8*)ariane_bin.data(), (u32)ariane_bin.size());
            if (!error)
                m_device->arianeIsReady_sync();

            arianeIsLoading = false;
        }
    }

    setLockEnabled(false);

    if (error && !silent)
        emit clb_error(int(error));

    if (!error && b_autoInject)
        emit clb_finished(AUTO_INJECT);

    // Get device info if ariane is ready
    if (m_device->arianeIsReady())
        devInfoSync = !(m_device->getDeviceInfo(&di));

    setLockEnabled(false);
    emit clb_deviceStateChange();

    if (devInfoSync)
        emit clb_deviceInfo(di);
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
    qDebug() << "QKourou::getDeviceInfo() execute in " << QThread::currentThreadId();

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

void QKourou::hack(const char* payload_path, u8 *payload_buff, u32 buff_size)
{
    if (!waitUntilUnlock())
        return;

    DWORD res = 0;
    setLockEnabled(true);

    // Reboot if ariane is loaded
    if (m_device->arianeIsReady() && !rebootToRcm())
        res = GetLastError();

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
