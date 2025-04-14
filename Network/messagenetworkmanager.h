#ifndef MESSAGENETWORKMANAGER_H
#define MESSAGENETWORKMANAGER_H

#include <QObject>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QQueue>

#include <QJsonObject>
#include <QJsonDocument>

#include <QMutex>
#include <QTimer>

#include "Utils/logger.h"

#include "envelope.qpb.h"
#include <QtProtobuf/qprotobufserializer.h>

class MessageNetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit MessageNetworkManager(QObject *parent = nullptr);

    QAbstractSocket::SocketState getSocketState() const;

public slots:
    void processSendMessageQueue();

    void connectToServer();
    void sendData(const QJsonObject &jsonToSend);
    void sendData(const QString &flag,const QByteArray &data);

    void setActiveUser(const QString &userName, const int &userId);
    void setLogger(Logger *logger);

signals:
    void messageReceived(const QJsonObject &receivedMessageJson);
    void groupMessageReceived(const QJsonObject &receivedMessageJson);
    void groupInfoReceived(const QJsonObject &receivedGroupInfoJson);
    void deleteGroupMemberReceived(const QJsonObject &receivedDeleteMemberFromGroup);
    void addGroupMemberReceived(const QJsonObject &receivedAddMemberFromGroup);
    void dialogsInfoReceived(const QJsonObject &receivedDialogInfoJson);
    void loginResultsReceived(const QJsonObject &loginResultsJson);
    void registrationResultsReceived(const QJsonObject &registrationResultsJson);
    void searchDataReceived(const QJsonObject &searchDataJson);
    void chatsUpdateDataReceived(QJsonObject &chatsUpdateDataJson);
    void loadMeassgesReceived(QJsonObject &messagesJson);
    void editResultsReceived(const QJsonObject &editResultsJson);
    void avatarsUpdateReceived(const QJsonObject &avatarsUpdateJson);

    void removeAccountFromConfigManager();

    void connectionSuccess();
    void onDisconnected();

private slots:
    void onDataReceived();
    void handleMessageBytesWritten(qint64 bytes);

private:

    QTcpSocket* socket;
    QQueue<QByteArray> sendMessageQueue;
    QMutex messageMutex;

    QString activeUserLogin;
    int activeUserId;

    Logger* logger;

    static const std::unordered_map<std::string_view, uint> flagMap;
    const int MAX_QUEUE_SIZE = 1000;
};

#endif // MESSAGENETWORKMANAGER_H
