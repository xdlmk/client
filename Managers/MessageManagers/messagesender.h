#ifndef MESSAGESENDER_H
#define MESSAGESENDER_H

#include <QObject>

#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

#include <QDir>
#include <QFile>
#include <QCoreApplication>

#include "Utils/logger.h"

#include "generated_protobuf/loadMessages.qpb.h"
#include <QProtobufSerializer>
class MessageSender : public QObject
{
    Q_OBJECT
public:
    explicit MessageSender(QObject *parent = nullptr);
    void setActiveUser(const QString &userLogin, const int &userId);
    void setLogger(Logger* logger);

public slots:
    void sendMessage(const QString &message, const int &receiver_id, const QString &flag);
    void sendMessageWithFile(const QString &message, const int &receiver_id,const QString& filePath, const QString &flag);
    void sendVoiceMessage(const int &receiver_id, const QString &flag);

    void sendRequestMessagesLoading(const int &chat_id, const QString &chat_name, const QString& flag, const int& offset);
signals:
    void sendMessageData(const QString &flag, const QByteArray &data);
    void sendMessageFileData(const QString &flag, const QByteArray &data);
private:
    QString activeUserLogin;
    int activeUserId;
    Logger* logger;
};

#endif // MESSAGESENDER_H
