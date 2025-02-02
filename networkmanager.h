#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QByteArray>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QTimer>

#include "Logger/logger.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void connectToServer();

public slots:
    void sendData(const QJsonObject &jsonToSend);

    void setLogger(Logger *logger);
signals:
    void dataReceived(const QJsonDocument &doc);

    void messageReceived(const QJsonObject &receivedMessageJson);
    void loginResultsReceived(const QJsonObject &loginResultsJson);
    void registrationResultsReceived(const QJsonObject &registrationResultsJson);
    void searchDataReceived(const QJsonObject &searchDataJson);
    void chatsUpdateDataReceived(QJsonObject &chatsUpdateDataJson);

    void connectionError();
    void connectionSuccess();


private slots:
    void onDisconnected();
    void attemptReconnect();

    void onDataReceived();

private:
    QTcpSocket* socket;
    QTimer reconnectTimer;
    Logger* logger;
};

#endif // NETWORKMANAGER_H
