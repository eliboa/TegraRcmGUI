#ifndef CUSTOMBUTTON_H
#define CUSTOMBUTTON_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include "../tegrarcmgui.h"

class TegraRcmGUI;

typedef enum _ButtonStyle : int {
    DEFAULT,
    LIGHT
} ButtonStyle;

typedef enum _BtnCondition : int {
    C_NONE,
    C_RCM_READY,
    C_ARIANE_READY,
    C_READY_FOR_PAYLOAD,
} BtnCondition;

class CustomButton : public QPushButton
{
    Q_OBJECT
public:
    CustomButton(TegraRcmGUI *parent, const QString &text, ButtonStyle style = DEFAULT);
    void addEnableCondition(BtnCondition condition);
    void setStatusTip(const QString &statusTip);

private:
   TegraRcmGUI *m_gui;
   bool m_enabled = true;
   QVector<BtnCondition> m_enable_conditions;
   QString m_statusTip;

   QString getStatusTip(BtnCondition *failed_condition = nullptr);
   BtnCondition getFailedCondition();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent *e);
};


#endif // CUSTOMBUTTON_H
