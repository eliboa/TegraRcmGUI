#include "qsettings.h"
#include "ui_qsettings.h"
#include "qutils.h"
#include <QProcess>

qSettings::qSettings(TegraRcmGUI *parent) : QWidget(parent),
    ui(new Ui::qSettings), parent(parent)
{
    ui->setupUi(this);
    m_kourou = parent->m_kourou;
    m_device = &parent->m_device;

    /// Stylesheets
    // Apply stylesheet to all buttons
    auto btnSs = GetStyleSheetFromResFile(":/res/QPushButton.qss");
    auto buttons = this->findChildren<QPushButton*>();
    for (int i = 0; i < buttons.count(); i++)
    {
        buttons.at(i)->setStyleSheet(btnSs);
        buttons.at(i)->setCursor(Qt::PointingHandCursor);
    }

    for (auto it : {ui->minTrayFrame, ui->driverFrame, ui->PkgmFrame})
        it->setStyleSheet(GetStyleSheetFromResFile(":/res/QFrame_box02.qss"));

    for (auto it : { ui->minTrayLbl, ui->driverLbl, ui->PkgmLbl})
        it->setStyleSheet(GetStyleSheetFromResFile(":/res/QLabel_title02.qss"));

    // Switch
    minTraySwitch = new Switch(parent->userSettings->contains("minToTray") && parent->userSettings->value("minToTray").toBool() ? true : false, 50);
    ui->minTrayLayout->addWidget(minTraySwitch);
    connect(minTraySwitch, SIGNAL(clicked()), this, SLOT(on_minToTraySwitch_toggled()));
}

qSettings::~qSettings()
{
    delete ui;
}

void qSettings::on_driverMissing()
{
    QString message(tr("The required APX device driver is missing.\nDo you wan to install it now ?"));
    if(QMessageBox::question(this, "Warning", message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        on_installDriverButton_clicked();
    }
}

void qSettings::on_installDriverButton_clicked()
{
    QString q_path = QDir(".").absolutePath() + "/apx_driver/InstallDriver.exe";

    QFile file(q_path);
    if (!file.exists())
    {
        QMessageBox::critical(this, "Error", tr("InstallDriver.exe not found!\nExpected location: ") + q_path);
        return;
    }

    std::wstring w_path = q_path.toStdWString();
    LPCWSTR path = (const wchar_t*) w_path.c_str();
    SHELLEXECUTEINFO shExInfo = { 0 };
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = nullptr;
    shExInfo.lpVerb = _T("runas");
    shExInfo.lpFile = path;
    shExInfo.lpDirectory = nullptr;
    shExInfo.nShow = SW_SHOW;
    shExInfo.hInstApp = nullptr;

    if (ShellExecuteEx(&shExInfo))
    {
        CloseHandle(shExInfo.hProcess);
    }
}

void qSettings::on_minToTraySwitch_toggled()
{
    parent->userSettings->setValue("minToTray", minTraySwitch->getState());
}

void qSettings::on_packagesButton_clicked()
{
    auto dialog = new PackageManager(&parent->m_pkgs, parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
    //delete dialog;
}
