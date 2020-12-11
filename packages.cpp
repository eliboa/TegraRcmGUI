#include "packages.h"
#include <JlCompress.h>
#include "tegrarcmgui.h"
#include "QtConcurrent/QtConcurrent"

bool JsonArrayContains(QJsonArray &arr, const QString &name ) { for (auto it : arr) if (it.toObject().contains("name") && it.toObject()["name"].toString() == name) return true; return false; };


Packages::Package::Package(Packages *parent, const QString &package_name) : m_parent(parent)
{
    m_name = package_name;
    auto findPackage = [&](const QString &path) {
        QFile file(path);
        if (!file.exists() || !file.open(QIODevice::ReadOnly))
            return false;

        QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
        if (!json.contains("packages"))
            return false;

        for (auto pkg : json["packages"].toArray())
        {
            auto obj = pkg.toObject();
            if (obj.contains("name") && obj["name"].toString() == package_name)
            {
                loadFromObj(obj);
                return true;
            }
        }
        return false;
    };

    if(!findPackage("packages/packages.json")) // Load from external file
        findPackage(":/res/packages.json"); // Load from built_in packages
}

Packages::Package::Package(Packages *parent, QJsonObject package_obj) : m_parent(parent)
{
    loadFromObj(package_obj);
}

void Packages::Package::loadFromObj(QJsonObject obj)
{
    if (!m_name.length() && obj.contains("name"))
        m_name = obj["name"].toString();
    if (obj.contains("version"))
        m_version = obj["version"].toString();
    if (obj.contains("author"))
        m_author = obj["author"].toString();
    if (obj.contains("description"))
        m_description = obj["description"].toString();
    if (obj.contains("type"))
        m_type = obj["type"].toString();
    if (obj.contains("url"))
        m_url = obj["url"].toString();
    if (obj.contains("payload"))
        m_payload = obj["payload"].toString();
    if (obj.contains("status"))
        m_status = (PkgStatus) obj["status"].toInt();

    if (m_version.length() && (!exists() || m_status > 10))
        m_status = BROKEN;
}

bool Packages::Package::exists()
{
    return location().length() ? QFile(location() + "/package.json").exists() : false;
}

QResult Packages::Package::update()
{
    if (!m_update_url.isValid())
        setUpdateVer(m_latest_ver, m_latest_url);

    if (!m_update_url.isValid())
        return QResult(InvalidUrl, m_update_url.toString(), APPEND);

    QDir dir("packages/" + m_name + "/" + m_update_version);
    if (!dir.exists() && !dir.mkpath("."))
        return QResult(InvalidPath, dir.path(), APPEND);

    filePtr = new QFile(dir.path() + "/" + QFileInfo(m_update_url.toString()).fileName());
    if (!filePtr->open(QIODevice::WriteOnly))
        return QResult(FileCreateFailed, filePtr->fileName(), APPEND);

    QNetworkRequest request(m_update_url);
    auto reply = m_parent->m_nm.get(request);
    m_networkStatus = PENDING;
    m_status = DOWNLOADING;
    m_parent->m_pkgs_updating.append(this);

    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        m_status = DOWNLOADING;
        m_updateProgress = int(bytesReceived * 100 / bytesTotal);
        emit m_parent->updateProgress(this, bytesReceived, bytesTotal);
    });
    connect(reply, &QNetworkReply::readyRead, [=]() {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (!status_code.isValid() || status_code.toInt() != 200)
            return;
        filePtr->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::finished, [=]() {
        m_networkStatus = READY;
        filePtr->close();
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NetworkError::NoError)
        {
            filePtr->remove();
            m_status = INSTALL_FAILED;
        }
        else
        {
            auto res = unzip(filePtr->fileName());
            if (!res.success() && !exists())
            {
                m_status = INSTALL_FAILED;
                emit m_parent->error(res);
            }
            else
            {
                // Install package
                auto jObj = json();
                if (jObj.contains("payload"))
                {
                    m_payload = location() + "/" + jObj["payload"].toString();
                    if (m_parent->m_gui && jObj.contains("install"))
                    {
                        for (auto j_cmd : jObj["install"].toArray()) if (j_cmd.toObject().contains("add_to_favorites") && j_cmd.toObject()["add_to_favorites"].toBool())
                        {
                            QFile file(m_payload);
                            if (file.exists())
                            {
                                if (!m_parent->m_gui->payloadTab->addFavorite(QFileInfo(file).fileName(), m_payload))
                                {
                                    m_status = INSTALL_FAILED;
                                    emit m_parent->error(QResult(AddFavoriteFailed));
                                }
                                break;
                            }
                        }
                    }
                }
                if (m_status != INSTALL_FAILED)
                {
                    m_status = getFilesForDeviceInstall() ? DEV_INSTALL_PENDING : INSTALLED;
                    emit m_parent->updateProgress(this, 1, 1);
                    m_version = m_update_version;
                    m_parent->save(this);
                }

                m_parent->m_pkgs_updating.removeAt(m_parent->m_pkgs_updating.indexOf(this));
                if (m_parent->m_pkgs_updating.empty())
                    m_parent->emit updateFinished();
            }
        }
        delete filePtr;
    });

    return QResult(NoError);
}

QResult Packages::Package::unzip(const QString &file_path, QString out_dir_path) {

    QFileInfo zip_fi(file_path);
    if (!out_dir_path.size())
        out_dir_path = zip_fi.path();

    if (out_dir_path.lastIndexOf("/") != out_dir_path.size())
        out_dir_path.append("/");

    QuaZip zip(file_path);
    if (!zip.open(QuaZip::mdUnzip))
        return QResult(ArchiveOpenFailed, file_path, APPEND);

    qint64 totaUncompressed = 0, currentUncompressed = 0;
    for (auto fi : zip.getFileInfoList())
        totaUncompressed += fi.uncompressedSize;

    m_status = UNZIPING;
    m_updateProgress = int(currentUncompressed * 100 / totaUncompressed);
    emit m_parent->updateProgress(this, currentUncompressed, totaUncompressed);

    QuaZipFile zip_file(&zip);
    QuaZipFileInfo fi;
    // Process directories
    for (bool b = zip.goToFirstFile(); b ; b = zip.goToNextFile()) if (zip_file.getFileInfo(&fi) && fi.externalAttr != 0x10 && fi.name.endsWith("/"))
    {
        QDir dir(out_dir_path + fi.name);
        if (!dir.exists())
            dir.mkpath(".");
    }
    // Process file
    for (bool b = zip.goToFirstFile(); b ; b = zip.goToNextFile())  if (zip_file.getFileInfo(&fi) && fi.externalAttr == 0x20)
    {
        zip_file.open(QIODevice::ReadOnly);
        if (!zip_file.isOpen())
            return QResult(ArchiveOpenFailed, tr("Failed to open file ")+ fi.name + tr("in archive ") + zip_fi.fileName());

        QFile file(out_dir_path + fi.name);
        file.open(QIODevice::WriteOnly);
        if (!file.isOpen())
            return QResult(FileCreateFailed, file.fileName(), APPEND);

        char buffer[0x100000];
        while (qint64 bytesRead = zip_file.read(buffer, 0x100000))
        {
            currentUncompressed += file.write(buffer, bytesRead);
            m_updateProgress = int(currentUncompressed * 100 / totaUncompressed);
            emit m_parent->updateProgress(this, currentUncompressed, totaUncompressed);
        }
        zip_file.close();
        file.close();

        if (file.size() != fi.uncompressedSize)
        {
            file.remove();
            return QResult(UnZipFailed, tr("Failed to unzip file ")+ fi.name + tr("in archive ") + zip_fi.fileName());
        }
    }
    return QResult(NoError);
}

QJsonObject Packages::Package::json()
{
    QJsonObject obj;
    if (!exists())
        return obj;

    QFile file(location() + "/package.json");
    if (!file.open(QIODevice::ReadOnly))
        return obj;

    obj = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    return obj;
}

bool Packages::Package::getFilesForDeviceInstall()
{
    auto jobj = json();
    if (!jobj.contains("install"))
        return false;

    auto install_cmds = jobj["install"].toArray();

    m_dev_install_cpys.clear();
    for (auto cmd : install_cmds) if (cmd.toObject().contains("copy"))
    {
        auto cp_cmd = cmd.toObject()["copy"].toObject();
        if (!cp_cmd.contains("source") || !cp_cmd.contains("destination"))
            continue;

        QString source = location() + "/" + cp_cmd["source"].toString();

        QString destination = cp_cmd["destination"].toString();
        if (!destination.startsWith("sdmmc:"))
            continue;
        destination = destination.right(destination.length()-6);

        QDir dir(source);
        if (dir.exists())
        {
            if (!source.endsWith("/"))
                source.append("/");
            if (!destination.endsWith("/"))
                destination.append("/");

            dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
            QDirIterator it(dir, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                QString item = it.next();                
                QString path = item.mid(source.length(), item.length());
                m_dev_install_cpys.append({ source + path, destination + path});
            }
        }
        else if (QFile(source).exists())
            m_dev_install_cpys.append({ source, destination});
    }
    return !m_dev_install_cpys.empty();
}

void Packages::Package::setStatus(PkgStatus s, qint64 current, qint64 total)
{
    m_status = s;
    emit m_parent->updateProgress(this, current, total);
    m_parent->save();
}

Packages::Packages()
{
    qRegisterMetaType<QResult>("QResult");
    qRegisterMetaType<Package*>("Package*");
    connect(&m_nm, &QNetworkAccessManager::finished, [=](QNetworkReply* reply)  {
        if (reply->error() != QNetworkReply::NetworkError::NoError)
            emit error(QResult(NetworkError, reply->errorString()));
    });
    this->loadPackages();

    // Test
    /*auto ams = get("Atmosphere");
    if (ams)
        ams->getFilesForDeviceInstall();
        */
    save();
}

Packages::~Packages() {
    for (auto pkg : m_packages)
        delete pkg;
}

bool Packages::loadPackages()
{
    m_packages.clear();
    auto load = [&](const QString &path) {
        QFile ext_file(path);
        if (ext_file.exists() && ext_file.open(QIODevice::ReadOnly))
        {
            QJsonObject json = QJsonDocument::fromJson(ext_file.readAll()).object();
            if (json.contains("packages")) for (auto pkg : json["packages"].toArray())
            {
                if (!this->contains(pkg.toObject()["name"].toString()))
                    m_packages.push_back(new Packages::Package(this, pkg.toObject()));
            }
            ext_file.close();
        }
    };

    load("packages/packages.json");// Load from external file
    load(":/res/packages.json"); // Load built_in packages
    return m_packages.count();
}

QResult Packages::save(Package* in_pkg)
{
    QJsonArray json_pkgs;
    /// LAMBDA: Update a package JsonObject
    auto upd_pkg = [&json_pkgs](Package* from_pkg, int index) {
        auto to_obj = json_pkgs[index].toObject();
        to_obj["name"] = from_pkg->name();
        to_obj["version"] = from_pkg->version();
        if (from_pkg->payload().length())
            to_obj["payload"] = from_pkg->payload();
        to_obj["status"] = from_pkg->status();
        if (from_pkg->author().length())
            to_obj["author"] = from_pkg->author();
        if (from_pkg->description().length())
            to_obj["description"] = from_pkg->description();
        if (from_pkg->type().length())
            to_obj["type"] = from_pkg->type();
        if (from_pkg->url().length())
            to_obj["url"] = from_pkg->url();
        json_pkgs[index] = to_obj;
    };
    /// LAMBDA: Retrieve or create JsonObject in json_pkgs
    auto j_arr_get = [&json_pkgs](const QString &name) {
        for (int i=0; i<json_pkgs.count(); i++) if (json_pkgs[i].toObject()["name"].toString() == name)
            return i;
        json_pkgs.insert(json_pkgs.end(), QJsonObject());
        return json_pkgs.count()-1;
    };

    // Read Json packages from files
    for (auto path : {"packages/packages.json", ":/res/packages.json"}) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
            break;
        auto in_pkgs = QJsonDocument::fromJson(file.readAll()).object()["packages"].toArray();
        for (auto pkg : in_pkgs) if(!JsonArrayContains(json_pkgs, pkg.toObject()["name"].toString()))
            json_pkgs.insert(json_pkgs.end(), pkg.toObject());
        file.close();
    }

    // Update or create packages
    if (in_pkg != nullptr)
        upd_pkg(in_pkg, j_arr_get(in_pkg->name()));
    else for (auto pkg : m_packages)
        upd_pkg(pkg, j_arr_get(pkg->name()));

    if (json_pkgs.count())
    {
        QFile file("packages/packages.json");
        if (!file.open(QIODevice::WriteOnly))
            return QResult(FileOpenFailed, file.fileName(), APPEND);

        QJsonObject json;
        json["packages"] = json_pkgs;
        file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
        file.close();
    }

    return QResult(NoError);
}

bool Packages::contains(const QString &pkg_name)
{
    if (!m_packages.count())
        return false;

    for (auto pkg : m_packages) if (pkg->name() == pkg_name)
        return true;

    return false;
}

Packages::Package* Packages::first()
{
    if (m_packages.isEmpty())
        return nullptr;

    m_pkg_cur_ix = 0;
    return m_packages[m_pkg_cur_ix];
}

Packages::Package* Packages::next()
{

    if (m_packages.isEmpty() || m_packages.count() < m_pkg_cur_ix+2)
        return nullptr;

    m_pkg_cur_ix++;
    return m_packages[m_pkg_cur_ix];
}

void Packages::updateLatestVer()
{
    if (!m_packages.size())
    {
        emit updateLatestVerFinished();
        return;
    }

    QUrl url("https://www.eliboa.com/switch/tegrarcmgui/packages/index.php?export=json&latest");
    QNetworkRequest request(url);
    auto reply = m_nm.get(request);
    m_networkStatus = reply->isRunning() ? PENDING : READY;
    connect(reply, &QNetworkReply::readyRead, [=]() {
        QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        if (!json.contains("packages"))
            return;

        auto pkg_updates = json["packages"].toArray();
        for (auto pkg_update : pkg_updates) {
            auto update = pkg_update.toObject();
            if (auto pkg = this->get(update["name"].toString()))
            {
                // Package found
                pkg->setLatestVer(update["version"].toString(), update["url"].toString());
            }

        }
    });
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        m_networkStatus = READY;
        emit updateLatestVerFinished();
        reply->deleteLater();
    });
}

QResult Packages::update(QList<Package*> pkgs)
{
    QError err = NoError;
    QString errStr;
    auto dl = [&err, &errStr](Package* pkg) {
        auto res = pkg->update();
        if (!res.success())
        {
            err = Error;
            errStr += res.errorStr() + "\n";
        }
    };
    if (pkgs.empty()) for (auto pkg : m_packages)
    {
        if (pkg->isUpdateAvailable())
            dl(pkg);
    }
    else for (auto pkg : pkgs) dl(pkg);

    return QResult(err, errStr);
}

void Packages::devInstall()
{
    if (!m_gui || !m_gui->m_kourou || !m_gui->m_device.arianeIsReady())
        return;

    QList<Packages::Package *> upd_pkgs;
    for (auto pkg : m_packages) if (pkg->status() == DEV_INSTALL_PENDING)
        upd_pkgs.append(pkg);

    if (!upd_pkgs.empty())
        QtConcurrent::run(m_gui->m_kourou, &QKourou::copyFiles, upd_pkgs);
}

