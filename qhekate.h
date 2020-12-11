#ifndef QHEKATE_H
#define QHEKATE_H

#include <QWidget>
#include "qutils.h"


#define BOOT_CFG_AUTOBOOT_EN (1 << 0)
#define BOOT_CFG_FROM_LAUNCH (1 << 1)
#define BOOT_CFG_FROM_ID     (1 << 2)
#define BOOT_CFG_TO_EMUMMC   (1 << 3)
#define BOOT_CFG_SEPT_RUN    (1 << 7)

#define EXTRA_CFG_KEYS    (1 << 0)
#define EXTRA_CFG_PAYLOAD (1 << 1)
#define EXTRA_CFG_MODULE  (1 << 2)

#define EXTRA_CFG_NYX_BIS    (1 << 4)
#define EXTRA_CFG_NYX_UMS    (1 << 5)
#define EXTRA_CFG_NYX_RELOAD (1 << 6)
#define EXTRA_CFG_NYX_DUMP   (1 << 7)

typedef enum _nyx_ums_type
{
    NYX_UMS_SD_CARD = 0,
    NYX_UMS_EMMC_BOOT0,
    NYX_UMS_EMMC_BOOT1,
    NYX_UMS_EMMC_GPP,
    NYX_UMS_EMUMMC_BOOT0,
    NYX_UMS_EMUMMC_BOOT1,
    NYX_UMS_EMUMMC_GPP
} nyx_ums_type;

struct HekatePayload
{
    QString file_path;
    AppVersion version;

    bool operator<(const HekatePayload &other) const
    {
        // Default is desc sort
        if (!(other.version == version))
            return other.version < version;
        else
            return other.file_path < file_path;
    }
};

class TegraRcmGUI;
class Kourou;
class QKourou;
class qProgressWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class qHekate; }
QT_END_NAMESPACE

class qHekate : public QWidget
{
    Q_OBJECT

public:
    explicit qHekate(TegraRcmGUI *parent = nullptr);
    ~qHekate();

    bool initHekatePayload();

private:
    Ui::qHekate *ui;
    TegraRcmGUI *parent;
    QKourou *m_kourou;
    Kourou *m_device;
    qProgressWidget *m_progressWidget;
    QVector<HekatePayload> m_payloads;
    AppVersion m_nyx_version;
    QByteArray m_hekate_payload;

    void drawHeader();
    void drawConfigBox();
    void drawUmsBox();

signals:
    void error(int);

public slots:
    void on_tabActivated();
    void on_deviceStateChange();
    void on_deviceInfo_received();
    void on_launchHekateUms(nyx_ums_type type);
    void on_launchHekateConfig();

private slots:
    void on_hekate_install();
    void on_ams_install();
    void on_configComboBox_currentIndexChanged(int index);
};

#endif // QHEKATE_H
