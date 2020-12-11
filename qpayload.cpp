
#include <QDebug>
#include "qpayload.h"
#include "ui_qpayload.h"
#include "qutils.h"
#include "qobjects/custombutton.h"

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

    // Stylesheets
    this->setStyleSheet(GetStyleSheetFromResFile(":/res/QMainWindow.qss"));
    ui->payload_tableView->setStyleSheet(GetStyleSheetFromResFile(":/res/QTableView.qss"));
    ui->payloadFrame->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box02.qss"));
    ui->payloadPathLbl->setStyleSheet(GetStyleSheetFromResFile(":/res/QLabel_title02.qss"));
    ui->favoritesLbl->setStyleSheet(GetStyleSheetFromResFile(":/res/QLabel_title02.qss"));

    // Buttons
    Switch *_switch = new Switch(parent->userSettings->value("autoInject").toBool() ? true : false, 50);
    ui->horizontalLayout->addWidget(_switch);
    connect(_switch, SIGNAL(clicked()), this, SLOT(on_autoInject_toggled()));

    auto *browseB = new CustomButton(parent, tr("Browse"), LIGHT);
    browseB->setStatusTip(tr("Browse payload file from local file systems"));
    browseB->setFixedSize(60, 20);
    connect(browseB, &QPushButton::clicked, this, &QPayloadWidget::on_browsePayloadBtn_clicked);
    ui->browsePayloadLayout->addWidget(browseB);

    auto *injectB = new CustomButton(parent, tr("INJECT PAYLOAD"));
    injectB->setStatusTip(tr("Inject current payload to device"));
    injectB->addEnableCondition(C_READY_FOR_PAYLOAD);
    injectB->setFixedSize(100, 30);
    connect(injectB, &QPushButton::clicked, this, &QPayloadWidget::on_injectPayloadBtn_clicked);
    ui->injectPayloadLayout->addWidget(injectB);

    // Tool tips
    ui->addFavoriteBtn->setStatusTip(tr("Add current payload selection to favorites"));
    ui->deleteFavoriteBtn->setStatusTip(tr("Remove selected item from favorites (payload file will not be deleted)"));

    ui->payload_path->setText(parent->userSettings->value("autoInjectPath").toString());
}

void QPayloadWidget::on_browsePayloadBtn_clicked()
{
    QString path = FileDialog(this, open_file).toLocal8Bit();
    if (path.length())
        ui->payload_path->setText(path);
}

void QPayloadWidget::on_injectPayload(QString payload_path)
{
    ui->payload_path->setText(payload_path);
    on_injectPayloadBtn_clicked();
}

void QPayloadWidget::on_injectPayloadBtn_clicked()
{
    QFile file(ui->payload_path->text().toLocal8Bit().constData());
    if (!file.open(QIODevice::ReadOnly))
        return parent->error(0x005);

    if (file.size() > PAYLOAD_MAX_SIZE)
    {
        file.close();
        return parent->error(PAYLOAD_TOO_LARGE);
    }
    file.close();

    tmp_string = ui->payload_path->text().toStdString();
    QtConcurrent::run(m_kourou, &QKourou::hack, tmp_string.c_str());
}

void QPayloadWidget::on_deviceStateChange()
{
    if ((m_device->rcmIsReady() && !m_kourou->autoLaunchAriane) || m_device->arianeIsReady())
    {
        if (m_payloadModel->rowCount())
            ui->payload_tableView->setStatusTip(tr("Double-click to inject payload"));
    }
    else
    {
        if (m_payloadModel->rowCount())
            ui->payload_tableView->setStatusTip(tr("Double-click to push favorite into current selection"));
    }
}

bool QPayloadWidget::addFavorite(QString name, QString path)
{

    if (m_payloadModel->getPayloads().contains({ path, name }))
        return true; // Already in favorites, return true

    QFile file(path);
    if (!file.exists())
        return false;

    if (!m_payloadModel->getPayloads().contains({ name, path }))
    {
        m_payloadModel->insertRows(0, 1, QModelIndex());
        auto index = m_payloadModel->index(0, 0, QModelIndex());
        m_payloadModel->setData(index, name, Qt::EditRole);
        index = m_payloadModel->index(0, 1, QModelIndex());
        m_payloadModel->setData(index, path, Qt::EditRole);

        return writeFavoritesToFile();
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
    auto payloads = m_payloadModel->getPayloads();
    for (auto paylaod : payloads)
    {
        QJsonObject payloadEntry;
        payloadEntry["name"] = paylaod.name;
        payloadEntry["path"] = paylaod.path;
        pArray.append(payloadEntry);
    }
    QJsonObject json;
    json["favorites"] = pArray;
    pFile.write(QJsonDocument(json).toJson());
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
    ui->payload_path->setText(m_payloadModel->getPayloads().at(index.row()).path);
    if (m_device->isReadyToReceivePayload())
        on_injectPayloadBtn_clicked();
}
