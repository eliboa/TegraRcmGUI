#ifndef QSETTINGSU_H
#define QSETTINGSU_H

#include <QWidget>
#include "tegrarcmgui.h"

class TegraRcmGUI;
class Kourou;
class QKourou;

QT_BEGIN_NAMESPACE
namespace Ui {
class qSettings;
}
QT_BEGIN_NAMESPACE

class qSettings : public QWidget
{
    Q_OBJECT

public:
    explicit qSettings(TegraRcmGUI *parent = nullptr);
    ~qSettings();

public slots:
    void on_driverMissing();
private slots:
    void on_installDriverButton_clicked();

private:
    Ui::qSettings *ui;
    TegraRcmGUI *parent;
    QKourou *m_kourou;
    Kourou *m_device;

};

#endif // QSETTINGSU_H
