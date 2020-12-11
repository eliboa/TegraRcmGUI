#ifndef GITH_API_H
#define GITH_API_H

#include <QtNetwork>
#include <QtCore>

#define GITH_API_URL "https://api.github.com"

class GitHubAPI: public QObject
{
    Q_OBJECT
public:
    explicit GitHubAPI(QObject *parent = nullptr);
    bool GET_sync(const QString &endpoint);
    bool getLatestRelease(const QString &owner, const QString &repo, QString *latest_release);

private:
    QNetworkAccessManager manager;
    bool waitForConnect(int nTimeOutms);

protected:
    QNetworkReply *m_pReply;
    QNetworkReply::NetworkError m_NetworkError;
    bool m_bReadTimeOut = false;

private slots:
    void slotWaitTimeout();

signals:
    void signalReadTimeout();
};

#endif

