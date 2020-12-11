#include "packagemanager.h"
#include "ui_packagemanager.h"
#include "tegrarcmgui.h"
#include <QDialog>
#include <QStyledItemDelegate>
#include <QtConcurrent/QtConcurrent>

PackageManager::PackageManager(Packages* pkgs, TegraRcmGUI* parent) :
    QDialog(parent),
    ui(new Ui::PackageManager), m_parent(parent)
{
    ui->setupUi(this);
    m_pkgs = pkgs;

    // Create model & connect to view
    const auto view = ui->tableView;
    m_um_model = new PackageManagerModel(m_pkgs, parent);
    view->setModel(m_um_model);
    view->setColumnWidth(0, 120);
    view->setColumnWidth(1, 60);
    view->setColumnWidth(2, 80);
    view->setColumnWidth(3, 60);
    view->setColumnWidth(4, 230);
    view->setItemDelegateForColumn(4,new PackageManagerItemDelegate(this));
    view->setAlternatingRowColors(true);
    view->show();

    // Connect signals & slots
    connect(m_pkgs, &Packages::updateProgress, this, &PackageManager::downloadProgress);
    connect(m_pkgs, SIGNAL(updateLatestVerFinished()), this, SLOT(pkgUpdateVerFinished()));
    connect(m_pkgs, &Packages::error, [=](QResult err) {
        QMessageBox::critical(this, "Error", err.errorStr());
    });
    connect(m_pkgs, &Packages::updateFinished, [=]() {
        if (m_pkgs->hasPendingDevInstalls())
            m_pkgs->devInstall();
    });

    // Stylesheet
    view->setStyleSheet(GetStyleSheetFromResFile(":/res/QTableView.qss"));

    m_pkgs->updateLatestVer();
}

PackageManager::~PackageManager()
{
    m_pkgs->disconnect();
    delete ui;
}

void PackageManager::on_checkForUpdateBut_clicked()
{
    // Update packages latest version
    //m_pkgs->updateLatestVer();

    if (!m_pkgs->isUpdateAvailable())
        QMessageBox::information(this, "Update", "Everything is up to date");
    else pkgUpdateVerFinished();

}

void PackageManager::downloadProgress(Packages::Package* pkg, qint64 current, qint64 total)
{
    m_um_model->updateProgress(pkg);
}

void PackageManager::pkgUpdateVerFinished()
{

    QString updateStr;
    for (auto pkg : m_pkgs->all()) if (pkg->isUpdateAvailable())
        updateStr += "- "+ pkg->name() +" "+ pkg->latest_ver() +"\n";

    if (updateStr.length())
    {
        QString message(tr("Update available for the following packages:\n")+ updateStr +tr("\n Do you want to download & update ?"));
        if (QMessageBox::question(this, tr("Update available"), message , QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
        {
            auto res = m_pkgs->update();
            if (!res.success())
                QMessageBox::critical(this, "Error", res.errorStr());
        }
    }
}


QVariant PackageManagerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= pkgs->count() || index.row() < 0)
        return QVariant();

    auto pkg = pkgs->at(index.row());
    auto col = index.column();

    if (col == 0)
    {
        if (role == Qt::DisplayRole)
            return pkg->name();
        else if (role == Qt::ToolTipRole)
            return pkg->description();
    }
    else if (col == 1)
    {
        if (role == Qt::DisplayRole)
            return pkg->author();
    }
    else if (col == 2)
    {
        if (role == Qt::DisplayRole)
            return pkg->type();
    }
    else if (col == 3)
    {
        if (role == Qt::DisplayRole)
            return pkg->version();
    }
    return QVariant();
}

void PackageManagerItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto model = reinterpret_cast<const PackageManagerModel*>(index.model());
    auto pkg = model->pkgs->at(index.row());
    QString msg;

    switch (pkg->status()) {
        case UNSET:
            msg.append("");
            break;
        case BROKEN:
            msg.append(tr("Package is broken. Update needed"));
            break;
        case INSTALLED:
            msg.append(tr("Package installed"));
            break;
        case INSTALL_FAILED:
            msg.append(tr("Failed to install package"));
            break;
        case DOWNLOADING:
            msg.append(tr("Downloading package..."));
            break;
        case UNZIPING:
            msg.append(tr("Extracting package..."));
            break;
        case INSTALLING:
            msg.append(tr("Installing package..."));
            break;
        case DEV_INSTALLING:
            msg.append(tr("Installing on device..."));
            break;
        case DEV_INSTALL_PENDING:
            msg.append(tr("Connect device to finish install"));
            break;
    }

    if (!is_in(pkg->status(), {DOWNLOADING, UNZIPING, INSTALLING}))
    {
        QStyleOptionViewItem itemOption(option);
        initStyleOption(&itemOption, index);
        itemOption.text = msg;
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &itemOption, painter);
        return;
    }

    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect;
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.progress = pkg->updateProgress();
    progressBarOption.text = msg + (progressBarOption.progress ? QString::number(progressBarOption.progress) + "%" : "");
    progressBarOption.textVisible = true;
    progressBarOption.textAlignment = Qt::AlignCenter;
    QApplication::style()->drawControl(QStyle::CE_ProgressBar,  &progressBarOption, painter);
}
