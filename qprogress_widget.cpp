#include "qprogress_widget.h"
#include "ui_qprogress_widget.h"
#include "qutils.h"
#include <QDateTime>

qProgressWidget::qProgressWidget(const QString init_message, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qProgressWidget)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet(GetStyleSheetFromResFile(":/res/ProgressWidget.qss"));

    QMovie *movie = new QMovie(":/res/loader_bg646464.gif");
    ui->loadingLbl->setMovie(movie);
    ui->loadingLbl->show();
    movie->start();

    if (init_message.size())
        setLabel(init_message);

    m_latest_activity = QDateTime::currentSecsSinceEpoch();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkActivity()));
    timer->start(1000); // Every second
}

void qProgressWidget::init_ProgressWidget(const QString init_message, int timeout_s)
{
    if (timeout_s)
        setTimeOut(timeout_s);

    if (init_message.size())
        setLabel(init_message);
    else
    {
        setLastActivity();
        show();
    }
}

qProgressWidget::~qProgressWidget()
{
    delete ui;
}

void qProgressWidget::checkActivity()
{
    if (!isVisible())
        return;

    qint64 now = QDateTime::currentSecsSinceEpoch();
    if (now > m_latest_activity + m_timeout_s)
        hide();
}

void qProgressWidget::setLastActivity()
{
    m_latest_activity = QDateTime::currentSecsSinceEpoch();
}

void qProgressWidget::setLabel(const QString label)
{    
    QString s_label = label.size() > 40 ? "..." + label.mid(label.size() - 37, 37) : label;
    ui->label->setText(s_label);
    setLastActivity();
    if (!isVisible())
        show();
}

void qProgressWidget::closeEvent(QCloseEvent *e)
{
    hide();
    QDialog::closeEvent(e);
}
