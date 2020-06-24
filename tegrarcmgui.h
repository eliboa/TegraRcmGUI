#ifndef TEGRARCMGUI_H
#define TEGRARCMGUI_H
#include <QMainWindow>
#include <QtWidgets>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>
#include "qpayload.h"
#include "qtools.h"
#include "kourou/kourou.h"
#include "kourou/usb_command.h"
#include "qkourou.h"

class QPayloadWidget;
class QKourou;

QT_BEGIN_NAMESPACE
namespace Ui { class TegraRcmGUI; }
QT_END_NAMESPACE

class TegraRcmGUI : public QMainWindow
{
    Q_OBJECT    

    static TegraRcmGUI* m_instance;
public:
    TegraRcmGUI(QWidget *parent = nullptr);
    ~TegraRcmGUI();
    static bool hasInstance() { return m_instance; }
    static TegraRcmGUI * instance() {
      if (!m_instance) m_instance = new TegraRcmGUI;
      return m_instance;
    }
    QSettings *userSettings;
    QSettings *userPayloads;
    Kourou m_device;
    QKourou *m_kourou;
    QPayloadWidget *payloadTab;
    qTools *toolsTab;
    bool enableWidget(QWidget *widget, bool enable);

private slots:
    void on_deviceInfo_received(UC_DeviceInfo di);
    void on_autoLaunchAriane_toggled(bool value);
    void pushTimer();
    void on_Kourou_finished(int res);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);

public slots:
    void hotPlugEvent(bool added, KLST_DEVINFO_HANDLE deviceInfo);
    void deviceInfoTimer();
    void error(int error);
    void on_deviceStateChange();
    void pushMessage(QString message);

signals:
    void sign_hotPlugEvent(bool added, KLST_DEVINFO_HANDLE);

private:
    Ui::TegraRcmGUI *ui;
    KHOT_HANDLE m_hotHandle = nullptr;

    bool m_ready = false;
    std::string tmp_string;
    QVector<qint64> push_ts;
    int tsToDeleteCount = 0;
    qint64 lastTsToDelete = 0;

    const QIcon switchOnIcon = QIcon(":/res/switch_logo_on.png");
    const QIcon switchOffIcon = QIcon(":/res/switch_logo_off.png");
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    void drawTrayContextMenu();

    void clearDeviceInfo();
};

const QString statusOnStyleSht( "QFrame{border-radius: 10px; background-color: rgb(0, 150, 136); border-color: rgb(0, 0, 0);}"
                                "QLabel{font: 75 9pt \"Calibri\"; color: rgb(255, 255, 255);}");
const QString statusOffStyleSht("QFrame{border-radius: 10px; background-color: rgb(213, 213, 213); border-color: rgb(0, 0, 0);}"
                                "QLabel{font: 75 9pt \"Calibri\"; color: rgb(0, 0, 0);}");
const QString statusOffRedStyleSht("QFrame{border-radius: 10px; background-color: rgb(150, 35, 0); border-color: rgb(0, 0, 0);}"
                                   "QLabel{font: 75 9pt \"Calibri\"; color: rgb(255, 255, 255);}");

#endif // TEGRARCMGUI_H
