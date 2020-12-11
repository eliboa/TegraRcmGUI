#ifndef PACKAGEMANAGER_H
#define PACKAGEMANAGER_H

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include "packages.h"
#include "qutils.h"

class TegraRcmGUI;

class PackageManagerModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    PackageManagerModel(Packages* packages,  QObject *parent = nullptr) : QAbstractTableModel(parent), pkgs(packages), m_parent(parent) { }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
            return QVariant();

        if (section == 0)
            return QString("Package");
        else if (section == 1)
            return QString("Author");
        else if (section == 2)
            return QString("Type");
        else if (section == 3)
            return QString("Version");
        else
            return QString("Status");
    }
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return parent.isValid() ? 0 : int(pkgs->count());
    }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        return parent.isValid() ? 0 :  5;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override {
        if (!index.isValid())
            return Qt::ItemIsEnabled;
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
    void updateProgress(Packages::Package *pkg = nullptr) {
        for (int i=0; i < pkgs->count(); i++) if (pkg == nullptr || pkgs->at(i) == pkg ){
            auto ix = index(i, 4, QModelIndex());
            emit dataChanged(ix, ix, {Qt::DisplayRole});
            if (pkg->status() == INSTALLED)
            {
                ix = index(i, 1, QModelIndex());
                emit dataChanged(ix, ix, {Qt::DisplayRole});
            }
        }
    }
    void updateAll() {
        auto topLeft = index(0, 0, QModelIndex());
        auto bottomRight = index(rowCount()-1, columnCount()-1, QModelIndex());
        emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole});
    }
    Packages* pkgs;
private:
    QObject* m_parent;
};

// Item delegate for Update progress
class PackageManagerItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PackageManagerItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const ;
};


namespace Ui {
class PackageManager;
}

class PackageManager : public QDialog
{
    Q_OBJECT

public:
    explicit PackageManager(Packages* pkgs, TegraRcmGUI *parent);
    ~PackageManager();

    PackageManagerModel* model() { return m_um_model; }

public slots:
    void pkgUpdateVerFinished();
    void downloadProgress(Packages::Package*, qint64, qint64);

private slots:
    void on_checkForUpdateBut_clicked();

private:
    Ui::PackageManager *ui;
    Packages* m_pkgs;
    PackageManagerModel *m_um_model;
    TegraRcmGUI* m_parent;

};

#endif // PACKAGEMANAGER_H
