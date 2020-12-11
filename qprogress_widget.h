#ifndef QPROGRESS_WIDGET_H
#define QPROGRESS_WIDGET_H

#include <QDialog>

namespace Ui {
class qProgressWidget;
}

class qProgressWidget : public QDialog
{
    Q_OBJECT

public:
    explicit qProgressWidget(const QString init_message = "", QWidget *parent = nullptr);
    ~qProgressWidget();

public:
    void setTimeOut(unsigned int seconds) { if (seconds) m_timeout_s = seconds; }

public slots:
    void init_ProgressWidget(const QString init_message = "", int timeout_s = 0);
    void setLabel(const QString label);
    void setLastActivity();
    void closeEvent(QCloseEvent *e) override;

private:
    Ui::qProgressWidget *ui;
    qint64 m_latest_activity;
    unsigned int m_timeout_s = 5;

private slots:
    void checkActivity();



};

#endif // QPROGRESS_WIDGET_H
