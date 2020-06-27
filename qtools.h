#ifndef QTOOLS_H
#define QTOOLS_H

#include <QWidget>
#include <QObject>
#include "tegrarcmgui.h"
#include "qutils.h"

class TegraRcmGUI;
class Kourou;
class QKourou;

QT_BEGIN_NAMESPACE
namespace Ui { class qTools; }
QT_END_NAMESPACE

class qTools : public QWidget
{
    Q_OBJECT
public:
    explicit qTools(TegraRcmGUI *parent = nullptr);
    ~qTools();
    Switch *autoRCM_switch;

private:
    Ui::qTools *ui;
    TegraRcmGUI *parent;
    QKourou *m_kourou;
    Kourou *m_device;

public slots:
    void on_deviceStateChange();

private slots:
    void on_autoRcmSwitchToggled();

signals:
    void error(int);

};

#endif // QTOOLS_H
