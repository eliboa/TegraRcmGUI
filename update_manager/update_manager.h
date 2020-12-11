#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = nullptr);

signals:

public slots:
};

#endif // UPDATEMANAGER_H
