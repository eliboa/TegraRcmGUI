#ifndef QKOUROU_H
#define QKOUROU_H

#include <QObject>
#include <QWidget>
#include <QThread>
#include "kourou/kourou.h"
#include "tegrarcmgui.h"

typedef enum _qKourouAction : int
{
    INIT_DEVICE,
    AUTO_INJECT,
    PAYLOAD_INJECT

} qKourouAction;


class TegraRcmGUI;

///
/// \brief The QKourou class is a Qt thread safe reimplemented class of Kourou class, for TegraRcmGUI
///

class QKourou : public QWidget {
    Q_OBJECT

public:
    QKourou(QWidget *parent, Kourou* device, TegraRcmGUI* gui);
    bool isLocked() { return m_locked; }
    QByteArray ariane_bin;
    bool autoLaunchAriane = true;
    bool autoInjectPayload = false;
    bool arianeIsLoading = false;

private:
    Kourou *m_device;
    TegraRcmGUI *m_gui;
    bool m_locked = false;
    bool m_force_lock = false;
    QWidget *parent;
    std::string tmp_string;
    void hack(const char* payload_path, u8 *payload_buff, u32 buff_size);
    void setLockEnabled(bool enable) { m_locked = enable; }
    bool waitUntilUnlock(uint timeout_s = 10);
    bool waitUntilRcmReady(uint timeout_s = 10);
    bool waitUntilInit(uint timeout_s = 10);
    bool rebootToRcm();
    DWORD autoInject();


public slots:
    void initDevice(bool silent, KLST_DEVINFO_HANDLE deviceInfo = nullptr);
    void getDeviceInfo();
    void hack(const char* payload_path);
    void hack(u8 *payload_buff, u32 buff_size);


signals:
    void clb_deviceInfo(UC_DeviceInfo di);
    void clb_error(int error);
    void clb_deviceStateChange();
    void clb_finished(int res);

};

#endif // QKOUROU_H
