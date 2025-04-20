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

#include "generated_protobuf/envelope.qpb.h"
#include "generated_protobuf/chatsInfo.qpb.h"
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
    void sendDataJson(const QJsonObject &jsonToSend);
    void sendData(const QString &flag,const QByteArray &data);

    void setActiveUser(const QString &userName, const int &userId);
    void setLogger(Logger *logger);

signals:
    void loginResultsReceived(const QByteArray &loginResultsData);
    void registrationResultsReceived(const QByteArray &registrationResultsData);
    void messageReceived(const QByteArray &receivedMessageData);
    void groupMessageReceived(const QByteArray &receivedMessageData);
    void deleteGroupMemberReceived(const QByteArray &receivedDeleteMemberFromGroupData);
    void addGroupMemberReceived(const QByteArray &receivedAddMemberFromGroupData);
    void dialogsInfoReceived(const QList<chats::DialogInfoItem> &receivedDialogInfo);
    void groupInfoReceived(const QList<chats::GroupInfoItem> &receivedGroupInfo);

    void searchDataReceived(const QByteArray &searchData);
    void chatsUpdateDataReceived(const QByteArray &chatsUpdateData);
    void loadMeassgesReceived(const QByteArray &messagesData);
    void editResultsReceived(const QByteArray &editResultsData);

    void avatarsUpdateReceived(const QByteArray &avatarsUpdateData);


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
