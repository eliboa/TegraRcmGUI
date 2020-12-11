#ifndef QKOUROU_H
#define QKOUROU_H

#include <QObject>
#include <QWidget>
#include <QThread>
#include "kourou/kourou.h"
#include "qobjects/hekate_ini.h"
#include "packages.h"

typedef enum _qKourouAction : int
{
    INIT_DEVICE,
    AUTO_INJECT,
    PAYLOAD_INJECT

} qKourouAction;


class Kourou;
class TegraRcmGUI;
class qProgressWidget;
class HekateIni;
class Packages;
///
/// \brief The QKourou class is a Qt thread safe reimplemented class of Kourou class, for TegraRcmGUI
///

class QKourou : public QWidget {
    Q_OBJECT

public:
    QKourou(QWidget *parent, Kourou* device, TegraRcmGUI* gui);
    ~QKourou();
    bool isLocked() { return m_locked; }
    QByteArray ariane_bin;
    bool autoLaunchAriane = true;
    bool autoInjectPayload = false;
    bool arianeIsLoading = false;
    HekateIni *m_hekate_ini = nullptr;
    HekateIni *&hekate_ini = m_hekate_ini;
    Kourou* device() { return m_device; }

private:
    QWidget *parent;
    Kourou *m_device;
    TegraRcmGUI *m_gui;
    bool m_locked = false;
    bool m_force_lock = false;
    bool m_askForDriverInstall = true;
    bool m_APX_device_reconnect = true;    
    std::string tmp_string;

    void hack(const char* payload_path, u8 *payload_buff, u32 buff_size);
    void setLockEnabled(bool enable) { m_locked = enable; }
    bool waitUntilUnlock(uint timeout_s = 10);
    bool waitUntilRcmReady(uint timeout_s = 10);
    bool waitUntilArianeReady(bool skip_rcm = false, uint timeout_s = 10);
    bool waitUntilInit(uint timeout_s = 10);
    bool rebootToRcm();
    DWORD autoInject();

public slots:
    void initDevice(bool silent, KLST_DEVINFO_HANDLE deviceInfo = nullptr);
    void getDeviceInfo();
    void hack(const char* payload_path);
    void hack(u8 *payload_buff, u32 buff_size);
    void initNoDriverDeviceLookUpLoop();
    void noDriverDeviceLookUp();
    void setAutoRcmEnabled(bool state);
    void installSDFiles(QString input_path, bool ignore_ini = false);
    void copyFiles(QList<Packages::Package*> pkgs);
    void getKeys();

signals:
    void clb_deviceInfo(UC_DeviceInfo di);
    void clb_error(int error);
    void clb_deviceStateChange();
    void clb_finished(int res);
    void clb_driverMissing();    
    void pushMessage(const QString);
    void initProgressWidget(const QString, int);
    void sendStatusLbl(const QString);
    void closeProgressWidget();
};

#endif // QKOUROU_H
