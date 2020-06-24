#include "qutils.h"

QString FileDialog(QWidget *parent, fdMode mode, const QString& defaultName)
{
    QFileDialog fd(parent);
    QString filePath;
    if (mode == open_file)
    {
        filePath = QFileDialog::getOpenFileName(parent, "Open file", "default_dir\\");
    }
    else
    {
        fd.setAcceptMode(QFileDialog::AcceptSave); // Ask overwrite
        filePath = fd.getSaveFileName(parent, "Save as", "default_dir\\" + defaultName);
    }
    if (!filePath.isEmpty())
    {
        QSettings appSettings;
        QDir CurrentDir;
        appSettings.setValue("default_dir", CurrentDir.absoluteFilePath(filePath));
    }
    return filePath;
}
