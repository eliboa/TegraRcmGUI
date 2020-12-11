#include "github_api.h"

GitHubAPI::GitHubAPI(QObject *parent)
    : QObject(parent)
{

}

bool GitHubAPI::getLatestRelease(const QString &owner, const QString &repo, QString *latest_release)
{
    if (!GET_sync(QString("/repos/%1/%2/releases/latest").arg(owner).arg(repo)))
        return false;

    QJsonDocument jsonResponse = QJsonDocument::fromJson(m_pReply->readAll());
    QJsonObject jsonObj = jsonResponse.object();

    QString lr = jsonObj["tag_name"].toString();

    if (!lr.size())
        return false;

    *latest_release = lr;
    return true;
}

// API GET synchronous
bool GitHubAPI::GET_sync(const QString &endpoint)
{
    QUrl url(QString(GITH_API_URL) + endpoint);

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/vnd.github.v3+json");

    m_pReply = manager.get(request);

    if (m_pReply == nullptr)
    {
        qDebug() << "GitHubAPI::api_GET_sync(): network error";
        return false;
    }

    if (!waitForConnect(10000))
    {
        qDebug() << "GitHubAPI::api_GET_sync(): timeout";
        m_NetworkError = QNetworkReply::TimeoutError;
        return false;
    }

    if (m_pReply == nullptr)
    {
        qDebug() << "GitHubAPI::api_GET_sync(): cancelled";
        m_NetworkError = QNetworkReply::OperationCanceledError;
        return false;
    }

    if (m_pReply->error() != QNetworkReply::NoError)
    {
        qDebug() << "GitHubAPI::api_GET_sync(): error" << m_pReply->errorString();
        m_NetworkError = m_pReply->error();
        return false;
    }

    return true;
}

// QNetworkManager is designed to work asynchronously
// We'll use a QEventLoop to wait until we receive the response and a QTimer to set a timeout
bool GitHubAPI::waitForConnect(int nTimeOutms)
{
    QTimer *timer = nullptr;
    QEventLoop eventLoop;
    bool bReadTimeOut = false;

    if (nTimeOutms > 0)
    {
        timer = new QTimer(this);

        connect(timer, SIGNAL(timeout()), this, SLOT(slotWaitTimeout()));
        timer->setSingleShot(true);
        timer->start(nTimeOutms);

        connect(this, SIGNAL(signalReadTimeout()), &eventLoop, SLOT(quit()));
    }

    // Wait on QNetworkManager reply here
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));

    if (m_pReply != nullptr)
    {
       // We wait for the first reply which comes faster than the finished signal
       connect(m_pReply, SIGNAL(readyRead()), &eventLoop, SLOT(quit()));
    }
    eventLoop.exec();

    if (timer != nullptr)
    {
        timer->stop();
        delete timer;
        timer = nullptr;
    }

    bReadTimeOut = m_bReadTimeOut;
    m_bReadTimeOut = false;

    return !bReadTimeOut;
}

void GitHubAPI::slotWaitTimeout()
{
    m_bReadTimeOut = true;  // Report timeout
    emit signalReadTimeout();
}
