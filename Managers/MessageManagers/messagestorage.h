#ifndef MESSAGESTORAGE_H
#define MESSAGESTORAGE_H

#include <QObject>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QVariantMap>

#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include "Utils/logger.h"

#include "generated_protobuf/updatingChats.qpb.h"
#include "generated_protobuf/chatMessage.qpb.h"
#include "QProtobufSerializer"

class MessageStorage : public QObject
{
    Q_OBJECT
public:
    explicit MessageStorage(QObject *parent = nullptr);
    void setActiveUser(const QString &userLogin, const int &userId);
    void setLogger(Logger* logger);

    bool savePersonalMessageToFile(const chats::ChatMessage &newMessage);
    bool saveGroupMessageToFile(const chats::ChatMessage &newMessage);
public slots:
    void updatingLatestMessagesFromServer(const QByteArray &latestMessagesData);
signals:
    void showPersonalChat(const QString& login, const QString& message, const int& id, const QString& out, const QString& type);

    void sendAvatarsUpdate();
    void getContactList();

    void removeAccountFromConfigManager();
private:
    QString activeUserLogin;
    int activeUserId;
    Logger* logger;
};

#endif // MESSAGESTORAGE_H
