#ifndef MESSAGESTORAGE_H
#define MESSAGESTORAGE_H

#include <QObject>

#include <QVariantMap>

#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include "Utils/logger.h"
#include "Managers/cryptomanager.h"

#include "generated_protobuf/updatingChats.qpb.h"
#include "generated_protobuf/chatMessage.qpb.h"
#include "generated_protobuf/markMessage.qpb.h"
#include "QProtobufSerializer"

class MessageStorage : public QObject
{
    Q_OBJECT
public:
    explicit MessageStorage(QObject *parent = nullptr);
    void setActiveUser(const QString &userLogin, const int &userId);
    void setLogger(Logger* logger);
    void setCryptoManager(CryptoManager* cryptoManager);

    bool savePersonalMessageToFile(chats::ChatMessage &newMessage);
    bool saveGroupMessageToFile(chats::ChatMessage &newMessage);
public slots:
    void updatingLatestMessagesFromServer(const QByteArray &latestMessagesData);
    void updateMessageStatus(const QByteArray &data);
signals:
    void showPersonalChat(const QString& login, const QString& message, const int& id, const QString& out, const QString& type, const QString& timestamp, const int& unreadCount);

    void sendAvatarsUpdate();

    void removeAccountFromConfigManager();

    void setReadStatusToMessage(const quint64 &message_id, const quint64 &chat_id, const QString &chat_type);
private:
    QString activeUserLogin;
    int activeUserId;

    CryptoManager* cryptoManager;
    Logger* logger;
};

#endif // MESSAGESTORAGE_H
