#ifndef PACKAGES_H
#define PACKAGES_H
#include <QWidget>
#include <QObject>
#include "qutils.h"
#include <QtNetwork>
#include "qerror.h"

class QKourou;
class TegraRcmGUI;

typedef enum _NetStatus {
    PENDING,
    READY
} NetStatus;

enum PkgStatus : int {
    UNSET = 0,
    BROKEN = 1,
    INSTALL_FAILED = 2,
    INSTALLED = 3,
    DEV_INSTALL_PENDING = 4,
    DOWNLOADING = 10,
    UNZIPING = 11,
    INSTALLING = 12,
    DEV_INSTALLING = 14,
};

class Packages : public QObject
{
    Q_OBJECT

public:
    Packages();
    ~Packages();

    // Nested class Package
    class Package
    {
    public:
        // Constructor
        Package(Packages* parent, const QString &package_name);
        Package(Packages* parent, QJsonObject package_obj);

        // Public members


        // Getters
        QString name() { return m_name; }
        QString version() { return m_version; }
        QString url() { return m_url; }
        QString author() { return m_author; }
        QString description() { return m_description; }
        QString type() { return m_type; }
        QString location() { return m_name.length() && m_version.length() ? "packages/" + m_name + "/" + m_version :  ""; }
        QString payload() { return m_payload; }
        QString latest_ver() { return m_latest_ver; }
        QUrl latest_ver_url() { return m_latest_url; }
        QString update_version() { return m_update_version; }
        QUrl update_url() { return m_update_url; }
        PkgStatus status() { return m_status; }
        int updateProgress() { return m_updateProgress; }
        Cpys devInstallCpys() {return m_dev_install_cpys; }
        Packages* parent() { return m_parent; }

        // Setters
        void setLatestVer(const QString &ver, const QUrl &url) { m_latest_ver = ver; m_latest_url = url; }
        void setUpdateVer(const QString &ver, const QUrl &url) { m_update_version = ver; m_update_url = url; }
        void setStatus(PkgStatus s, qint64 current = 1, qint64 total = 1);
        void clearDevInstallCpys() { m_dev_install_cpys.clear(); }

        // Public methods
        bool exists();
        bool isUpdateAvailable() { return m_status < 10 && m_latest_ver.length() && (m_latest_ver != m_version || !exists()) && m_latest_url.isValid(); }
        QResult update();
        QResult unzip(const QString &file_path, QString out_dir_path = "");
        QJsonObject json();
        bool getFilesForDeviceInstall();

    private:
        Packages *m_parent;
        QString m_name;
        QString m_version;
        QString m_author;
        QString m_description;
        QString m_type;
        QString m_url;
        QString m_payload;
        QString m_latest_ver;
        QUrl m_latest_url;
        QString m_update_version;
        QUrl m_update_url;
        QPointer<QFile> filePtr = nullptr;
        NetStatus m_networkStatus = READY;
        PkgStatus m_status = PkgStatus::UNSET;
        int m_updateProgress = 0;
        Cpys m_dev_install_cpys;

        // Private methods
        void loadFromObj(QJsonObject obj);
    };

    // Methods
    void setParent(TegraRcmGUI* parent) { m_gui = parent; }
    QResult save(Package* pkg = nullptr);
    bool loadPackages();
    bool contains(const QString &pkg_name);
    bool isPending() { return m_networkStatus == PENDING; }
    bool isUpdateAvailable() { for (auto pkg : m_packages) if (pkg->isUpdateAvailable()) return true; return false; }
    void updateLatestVer();
    QResult update(QList<Package*> pkgs = QList<Package*>());
    bool isUpdateInProcess() { return !m_pkgs_updating.empty(); }
    bool hasPendingDevInstalls() { for (auto pkg : m_packages) if (pkg->status() == DEV_INSTALL_PENDING) return true; return false; }
    void devInstall();

    // Getters
    Package* get(const QString &name) { for (auto pkg : m_packages) if (pkg->name() == name) return pkg; return nullptr; }
    Package* first();
    Package* next();
    Package* at(const int i) { return m_packages.at(i); }
    QVector<Package*> all() { return m_packages; }
    int count() { return m_packages.count(); }

signals:
    void updateLatestVerFinished();
    void updateFinished();
    void updateProgress(Package*, qint64, qint64);
    void error(QResult);

private:
    QVector<Packages::Package*> m_packages;
    int m_pkg_cur_ix = 0;

    QNetworkAccessManager m_nm;
    NetStatus m_networkStatus = READY;
    bool m_update_available = false;
    TegraRcmGUI *m_gui = nullptr;
    QList<Packages::Package*> m_pkgs_updating;

    QQueue<Cpy> dev_cpy_queue;
};

#endif // PACKAGES_H
