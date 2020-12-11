#ifndef QPAYLOADWIDGET_H
#define QPAYLOADWIDGET_H
#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QAbstractTableModel>
#include <QFileSystemModel>
#include "tegrarcmgui.h"

struct payload_t
{
    QString path;
    QString name;

    bool operator==(const payload_t &other) const
    {
        return path == other.path;
    }
};

inline QDataStream &operator<<(QDataStream &stream, const payload_t &payload)
{
    return stream << payload.name << payload.path;
}

inline QDataStream &operator>>(QDataStream &stream, payload_t &payload)
{
    return stream >> payload.name >> payload.path;
}


class TegraRcmGUI;
class Kourou;
class QKourou;

class PayloadModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    PayloadModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &index) override;
    bool removeRows(int position, int rows, const QModelIndex &index) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    const QVector<payload_t> &getPayloads() const;

private:
    QVector<payload_t> payloads;
};

QT_BEGIN_NAMESPACE
namespace Ui { class QPayloadWidget; }
QT_END_NAMESPACE

class QPayloadWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QPayloadWidget(TegraRcmGUI *parent = nullptr);
    std::string tmp_string;
    const QVector<payload_t> &getPayloads() const { return m_payloadModel->getPayloads(); }

    bool addFavorite(QString name, QString path);

private:
    Ui::QPayloadWidget *ui;
    PayloadModel *m_payloadModel;
    QFileSystemModel *m_fsModel;
    TegraRcmGUI *parent;
    QKourou *m_kourou;
    Kourou *m_device;


    bool readFavoritesFromFile();
    bool writeFavoritesToFile();

signals:

public slots:
    void on_deviceStateChange();
    void on_injectPayload(QString payload_path);

private slots:
    void on_browsePayloadBtn_clicked();
    void on_injectPayloadBtn_clicked();
    void on_addFavoriteBtn_clicked();
    void on_deleteFavoriteBtn_clicked();
    void on_autoInject_toggled();
    void on_payload_path_textChanged(const QString &arg1);

    void on_payload_tableView_doubleClicked(const QModelIndex &index);

signals:
    void error(int);
};

#endif // QPAYLOADWIDGET_H
