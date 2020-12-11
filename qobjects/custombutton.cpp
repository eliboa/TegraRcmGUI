#include "custombutton.h"
#include "../qutils.h"

CustomButton::CustomButton(TegraRcmGUI *parent, const QString &text, ButtonStyle style) : QPushButton(parent), m_gui(parent)
{
    setText(text);

    QString stylesheet_file = ":/res/QPushButton.qss";
    if (style == LIGHT)
        stylesheet_file = ":/res/QPushButton_light.qss";

    setStyleSheet(GetStyleSheetFromResFile(stylesheet_file));

    m_enabled = getFailedCondition() == C_NONE ? true : false;
    setCursor(m_enabled ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
}

void CustomButton::addEnableCondition(BtnCondition condition)
{
    for (BtnCondition m_condition : m_enable_conditions) if (m_condition == condition)
        return;

    m_enable_conditions.push_back(condition);
}

void CustomButton::paintEvent(QPaintEvent *e)
{
    if (!m_enable_conditions.size())
    {
        QPushButton::paintEvent(e);
        return;
    }

    auto failed = getFailedCondition();
    bool b_enabled = failed == C_NONE ? true : false;
    if (b_enabled != m_enabled)
    {
        setCursor(b_enabled ? Qt::PointingHandCursor : Qt::ForbiddenCursor);
        QPushButton::setStatusTip(getStatusTip(&failed));
        m_enabled = b_enabled;
    }

    QPushButton::paintEvent(e);
}

void CustomButton::setStatusTip(const QString &statusTip)
{
    m_statusTip = statusTip;
    QPushButton::setStatusTip(getStatusTip());
}

BtnCondition CustomButton::getFailedCondition()
{
    for (auto condition : m_enable_conditions)
    {
        if ((condition == C_RCM_READY && !m_gui->m_device.rcmIsReady())
         || (condition == C_ARIANE_READY && !m_gui->m_device.arianeIsReady())
         || (condition == C_READY_FOR_PAYLOAD && !m_gui->m_device.isReadyToReceivePayload()))
            return condition;
    }
    return C_NONE;
}

QString CustomButton::getStatusTip(BtnCondition *failed_condition)
{
    auto failed = failed_condition ? *failed_condition : getFailedCondition();
    if (failed == C_RCM_READY)
        return tr("No RCM device found");

    if (failed == C_ARIANE_READY)
        return tr("Ariane is not loaded/ready");

    if (failed == C_READY_FOR_PAYLOAD)
        return tr("No device found");

    return m_statusTip;
}

void CustomButton::mousePressEvent(QMouseEvent *e)
{
    if (!m_enabled)
        return;

    QPushButton::mousePressEvent(e);
}
