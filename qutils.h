#ifndef QUTILS_H
#define QUTILS_H

#include <QWidget>
#include <QFileDialog>
#include <QFile>
#include <QSettings>

enum fdMode { open_file, save_as };

QString FileDialog(QWidget *parent, fdMode mode, const QString& defaultName = "");

#endif // QUTILS_H
