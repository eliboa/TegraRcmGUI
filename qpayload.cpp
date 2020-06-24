#include <QDebug>
#include "qpayload.h"
#include "ui_qpayload.h"
#include "qutils.h"

/////////////////////////////////////////////////////////////
/// \brief QPayloadWidget::QPayloadWidget
/// \param parent
/////////////////////////////////////////////////////////////

QPayloadWidget::QPayloadWidget(TegraRcmGUI *parent) : QWidget(parent)
  , ui(new Ui::QPayloadWidget), parent(parent)
{
    ui->setupUi(this);
    m_kourou = parent->m_kourou;
    m_device = &parent->m_device;
    connect(this, SIGNAL(error(int)), parent,  SLOT(error(int)));

    // Payload model
    m_payloadModel = new PayloadModel(this);
    /// Populate model
    readFavoritesFromFile();
    /// Connect model to table view
    ui->payload_tableView->setModel(m_payloadModel);       

    // Payload view
    const auto view = ui->payload_tableView;
    view->verticalHeader()->hide();
    view->horizontalHeader()->hide();
    view->hideColumn(1);
    view->setColumnWidth(0, view->width());
    view->setAlternatingRowColors(true);
    view->show();   

    auto item = ui->payload_tableView->findChildren<QAbstractItemView*>();
    for (int i = 0; i < item.count(); i++)
    {
        item.at(i)->setStatusTip("test");
    }


    // Timers
    QTimer *payloadBtnStatus_timer = new QTimer(this);
    connect(payloadBtnStatus_timer, SIGNAL(timeout()), this, SLOT(payloadBtnStatusTimer()));
    payloadBtnStatus_timer->start(1000); // Every second



    /// Stylesheets
    // Apply stylesheet to all buttons
    QString btnSs = GetStyleSheetFromResFile(":/res/QPushButton.qss");
    auto buttons = this->findChildren<QPushButton*>();
    for (int i = 0; i < buttons.count(); i++)
    {
        buttons.at(i)->setStyleSheet(btnSs);
        buttons.at(i)->setCursor(Qt::PointingHandCursor);

    }
    ui->payload_tableView->setStyleSheet(GetStyleSheetFromResFile(":/res/QTableView.qss"));
    ui->payloadFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box02.qss"));

    // Buttons
    Switch *_switch = new Switch(parent->m_kourou->autoInjectPayload ? true : false, 70);
    ui->horizontalLayout->addWidget(_switch);
    connect(_switch, SIGNAL(clicked()), this, SLOT(on_autoInject_toggled()));
    //ui->injectPayloadBtn->setCursor(Qt::PointingHandCursor);
    //ui->browsePayloadBtn->setCursor(Qt::PointingHandCursor);

    // ToolTip
    ui->addFavoriteBtn->setStatusTip(tr("Add current payload selection to favorites"));
    ui->deleteFavoriteBtn->setStatusTip(tr("Remove selected item from favorites (payload file will not be deleted)"));

    QString ppath = parent->userSettings->value("autoInjectPath").toString();
    ui->payload_path->setText(ppath);

}
void QPayloadWidget::payloadBtnStatusTimer()
{
    qint64 time = QDateTime::currentSecsSinceEpoch();
    if (payloadBtnStatusLbl_time && (time + 5 > payloadBtnStatusLbl_time))
    {
        ui->payloadBtnStatusLbl->setText("");
        payloadBtnStatusLbl_time = 0;
    }
}
void QPayloadWidget::on_browsePayloadBtn_clicked()
{
    QString path = FileDialog(this, open_file).toLocal8Bit();
    if (path.length())
        ui->payload_path->setText(path);
}

void QPayloadWidget::on_injectPayloadBtn_clicked()
{
    if (!m_device->rcmIsReady() && !m_device->arianeIsReady())
        return;

    if (payloadBtnLock)
    {
        ui->payloadBtnStatusLbl->setText(tr("Work in progress"));
        payloadBtnStatusLbl_time = QDateTime::currentSecsSinceEpoch();
        return;
    }

    QFile file(ui->payload_path->text().toLocal8Bit().constData());
    if (!file.open(QIODevice::ReadOnly))
        return parent->error(0x005);
    if (file.size() > PAYLOAD_MAX_SIZE)
    {
        file.close();
        return parent->error(PAYLOAD_TOO_LARGE);
    }
    file.close();

    tmp_string.clear();
    tmp_string.append(ui->payload_path->text().toStdString());
    payloadBtnLock = true;
    QtConcurrent::run(m_kourou, &QKourou::hack, tmp_string.c_str());
}

void QPayloadWidget::on_deviceStateChange()
{
    if ((m_device->rcmIsReady() && !m_kourou->autoLaunchAriane) || m_device->arianeIsReady())
    {
        payloadBtnLock = false;
        ui->injectPayloadBtn->setCursor(Qt::PointingHandCursor);
        ui->injectPayloadBtn->setStatusTip("Inject current payload to device");
        if (m_payloadModel->rowCount())
            ui->payload_tableView->setStatusTip(tr("Double-click to inject payload"));
    }
    else
    {
        payloadBtnLock = true;
        QString tip = m_device->getStatus() == CONNECTED ? tr("RCM device is not ready. Reboot to RCM") : tr("RCM device undetected!");
        if (m_kourou->arianeIsLoading)
            tip = tr("Wait for Ariane to be fully loaded");
        //ui->injectPayloadBtn->setToolTip(tip);
        ui->injectPayloadBtn->setStatusTip(tip);
        ui->injectPayloadBtn->setCursor(Qt::ForbiddenCursor);
        if (m_payloadModel->rowCount())
            ui->payload_tableView->setStatusTip(tr("Double-click to push favorite into current selection"));
    }
}

bool QPayloadWidget::addFavorite(QString name, QString path)
{
    QFile file(path);

    if (!file.exists())
        return false;

    if (!m_payloadModel->getPayloads().contains({ name, path }))
    {
        m_payloadModel->insertRows(0, 1, QModelIndex());
        QModelIndex index = m_payloadModel->index(0, 0, QModelIndex());
        m_payloadModel->setData(index, name, Qt::EditRole);
        index = m_payloadModel->index(0, 1, QModelIndex());
        m_payloadModel->setData(index, path, Qt::EditRole);
        return true;
    }
    return false;
}

void QPayloadWidget::on_addFavoriteBtn_clicked()
{
    QString path = ui->payload_path->text();
    if (!path.size())
    {
        QMessageBox::information(this, tr("Argument missing"), tr("Payload path is missing"));
        return;
    }
    QFile file(path);
    if (!file.exists())
    {
        QMessageBox::information(this, tr("File missing"), tr("Payload path does not point to a file"));
        return;
    }

    QFileInfo info(path);
    QString name = info.fileName();

    if (m_payloadModel->getPayloads().contains({ path, name }))
    {
        QMessageBox::information(this, tr("Duplicate Path"), tr("Payload already in favorites").arg(name));
        return;
    }

    if(addFavorite(name, path))
        writeFavoritesToFile();
}

void QPayloadWidget::on_deleteFavoriteBtn_clicked()
{
    int row = ui->payload_tableView->currentIndex().row();
    if (row >= 0 && !m_payloadModel->removeRows(row, 1, QModelIndex()))
        emit error(-1);

    writeFavoritesToFile();
}

void QPayloadWidget::on_autoInject_toggled()
{
    m_kourou->autoInjectPayload = !m_kourou->autoInjectPayload;
    parent->userSettings->setValue("autoInject", m_kourou->autoInjectPayload);

    if (m_kourou->autoInjectPayload && (m_device->arianeIsReady() || m_device->rcmIsReady()))
        QtConcurrent::run(m_kourou, &QKourou::initDevice, false, nullptr);
}

void QPayloadWidget::on_payload_path_textChanged(const QString &arg1)
{
    if (!ui->payload_path->text().length())
        return;

    QString curPath = parent->userSettings->value("autoInjectPath").toString();

    if (curPath == ui->payload_path->text())
        return;

    parent->userSettings->setValue("autoInjectPath", ui->payload_path->text());

    if (m_kourou->autoInjectPayload && (m_device->arianeIsReady() || m_device->rcmIsReady()))
         QtConcurrent::run(m_kourou, &QKourou::initDevice, false, nullptr);
}


bool QPayloadWidget::readFavoritesFromFile()
{
    QFile pFile(QStringLiteral("payloads.json"));

    if (!pFile.open(QIODevice::ReadOnly))
        return false;

    QByteArray pData = pFile.readAll();

    pFile.close();

    QJsonDocument loadDoc(QJsonDocument::fromJson(pData));

    QJsonObject json = loadDoc.object();

    QJsonArray pArray = json["favorites"].toArray();
    int i(0);
    for (i = 0; i < pArray.size(); i++)
    {
        QJsonObject payloadEntry = pArray[i].toObject();
        QString payload_name = payloadEntry["name"].toString();
        QString payload_path = payloadEntry["path"].toString();
        addFavorite(payload_name, payload_path);
    }

    return i ? true : false;

}
bool QPayloadWidget::writeFavoritesToFile()
{

    QFile pFile(QStringLiteral("payloads.json"));

    if (!pFile.open(QIODevice::WriteOnly))
        return false;

    QJsonArray pArray;
    QVector<payload_t> payloads = m_payloadModel->getPayloads();
    for (payload_t paylaod : payloads)
    {
        QJsonObject payloadEntry;
        payloadEntry["name"] = paylaod.name;
        payloadEntry["path"] = paylaod.path;
        pArray.append(payloadEntry);
    }
    QJsonObject json;
    json["favorites"] = pArray;
    QJsonDocument saveDoc(json);
    pFile.write(saveDoc.toJson());

    pFile.close();

    return true;
}


/////////////////////////////////////////////////////////////
/// \brief PayloadModel::PayloadModel
/// \param parent
////////////////////////////////////////////////////////////

PayloadModel::PayloadModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

int PayloadModel::rowCount(const QModelIndex &parent) const
{
   return parent.isValid() ? 0 : int(payloads.size());
}

int PayloadModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 :  2;
}

QVariant PayloadModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
            return QVariant();

    if (index.row() >= payloads.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole)
    {
       const auto &payload = payloads.at(index.row());
       return !index.column() ? payload.name : payload.path;
    }

    return QVariant();
}

bool PayloadModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index)
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        payloads.insert(position, { QString(), QString() });

    endInsertRows();
    return true;
}

bool PayloadModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index)
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        payloads.removeAt(position);

    endRemoveRows();
    return true;
}

bool PayloadModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        const int row = index.row();
        auto payload = payloads.value(row);

        if (!index.column())
            payload.name = value.toString();
        else
            payload.path = value.toString();

        payloads.replace(row, payload);
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        return true;
    }

    return false;
}

Qt::ItemFlags PayloadModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

const QVector<payload_t> &PayloadModel::getPayloads() const
{
    return payloads;
}


void QPayloadWidget::on_payload_tableView_doubleClicked(const QModelIndex &index)
{
    QString path = m_payloadModel->getPayloads().at(index.row()).path;
    ui->payload_path->setText(path);
    on_injectPayloadBtn_clicked();
}
