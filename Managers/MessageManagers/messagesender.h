#ifndef MESSAGESENDER_H
#define MESSAGESENDER_H

#include <QObject>

#include <QByteArray>
#include <QUuid>

#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include "Managers/cryptomanager.h"

#include "Utils/logger.h"

#include "generated_protobuf/loadMessages.qpb.h"
#include "generated_protobuf/createDialog.qpb.h"
#include "generated_protobuf/chatsInfo.qpb.h"
#include "generated_protobuf/markMessage.qpb.h"
#include <QProtobufSerializer>
class MessageSender : public QObject
{
    Q_OBJECT
public:
    explicit MessageSender(QObject *parent = nullptr);
    void setActiveUser(const QString &userLogin, const int &userId);
    void setLogger(Logger* logger);
    void setCryptoManager(CryptoManager *cryptoManager);

public slots:
    void sendMessage(const QString &message, const quint64 &receiver_id, const QString &flag);
    void sendMessageWithFile(const QString &message, const int &receiver_id,const QString& filePath, const QString &flag);
    void sendVoiceMessage(const int &receiver_id, const QString &flag);

    void markMessageAsRead(const quint64& message_id);

    void sendRequestMessagesLoading(const int &chat_id, const QString &chat_name, const QString& flag, const int& offset);

signals:
    void sendMessageData(const QString &flag, const QByteArray &data);
    void sendMessageFileData(const QString &flag, const QByteArray &data);

private:
    bool writeContentToFile(const QString &filePath, const QString &content);

    QString activeUserLogin;
    int activeUserId;

    Logger* logger;

    CryptoManager *cryptoManager;
};

#endif // MESSAGESENDER_H
