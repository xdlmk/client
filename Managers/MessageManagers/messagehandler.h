#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>

#include <QJsonObject>
#include <QVariantMap>

#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include "Utils/logger.h"
#include "Managers/cryptomanager.h"
#include "Managers/MessageManagers/messagestorage.h"
#include "Managers/MessageManagers/messagesender.h"

#include "generated_protobuf/chatMessage.qpb.h"
#include "QProtobufSerializer"

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(QObject *parent = nullptr);

    MessageSender *getMessageSender();
    MessageStorage *getMessageStorage();

    void setCryptoManager(CryptoManager* cryptoManager);

public slots:
    void setActiveUser(const QString &userLogin, const int &userId);
    void setLogger(Logger* logger);

    void processingPersonalMessage(const QByteArray &receivedMessageData);
    void processingGroupMessage(const QByteArray &receivedMessageData);

    void loadingChat(const quint64& id, const QString &flag);

    void loadingNextMessages(const QByteArray &messagesData);

signals:
    void newMessage(QVariant message);
    void clearMainListView();

    void checkAndSendAvatarUpdate(const QString &avatar_url,const int &user_id,const QString& type);
    void checkActiveDialog(QVariant message, const QString& type);

    void getChatsInfo();

    void sendMessageData(const QString &flag, const QByteArray &data);
    void sendMessageFileData(const QString &flag, const QByteArray &data);

    void insertMessage(QVariant message, bool isOutgoing);
    void returnChatToPosition();

    void updatingLatestMessagesFromServer(const QByteArray &latestMessages);

    void sendMessage(const QString &message, const quint64 &receiver_id, const QString &flag);
    void sendMessageWithFile(const QString &message, const int &receiver_id,const QString& filePath, const QString &flag);
    void sendVoiceMessage(const int &receiver_id, const QString &flag);

    void sendRequestMessagesLoading(const int &chat_id, const QString &chat_name, const QString& flag, const int& offset);

private:
    qint64 getAudioDuration(const QString &fullFileName);
    QString encryptContentFromMessage(const chats::ChatMessage &protoMsg);

    CryptoManager *cryptoManager;
    MessageStorage *messageStorage;
    MessageSender *messageSender;

    QString activeUserLogin;
    int activeUserId;
    Logger* logger;
};

#endif // MESSAGEHANDLER_H
