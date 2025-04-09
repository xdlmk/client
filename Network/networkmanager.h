#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QByteArray>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQueue>

#include <QMutex>

#include <QTimer>

#include "messagenetworkmanager.h"
#include "filenetworkmanager.h"

#include "Utils/logger.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    FileNetworkManager *getFileNetwork();
    MessageNetworkManager *getMessageNetwork();

public slots:
    void setActiveUser(const QString &userName,const int &userId);
    void setLogger(Logger *logger);

signals:
    void connectionError();
    void connectionSuccess();

private slots:
    void onDisconnected();
    void attemptReconnect();

private:
    FileNetworkManager *fileNetwork;
    MessageNetworkManager *messageNetwork;

    QString activeUserLogin;
    int activeUserId;

    QTimer reconnectTimer;
    Logger* logger;
};

#endif // NETWORKMANAGER_H
