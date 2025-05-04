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
signals:
    void showPersonalChat(const QString& login, const QString& message, const int& id, const QString& out, const QString& type);

    void sendAvatarsUpdate();

    void removeAccountFromConfigManager();
private:
    QString activeUserLogin;
    int activeUserId;

    CryptoManager* cryptoManager;
    Logger* logger;
};

#endif // MESSAGESTORAGE_H
