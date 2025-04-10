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
class MessageSender : public QObject
{
    Q_OBJECT
public:
    explicit MessageSender(QObject *parent = nullptr);
    void setActiveUser(const QString &userLogin, const int &userId);
    void setLogger(Logger* logger);

public slots:
    void sendMessage(const QString &message, const int &receiver_id, const QString &flag);
    void saveMessageAndSendFile(const QString &message, const int &receiver_id,const QString& filePath, const QString &flag);
    void sendMessageWithFile(const QString &fileUrl,const QString &flag);
    void sendMessageWithFile(const QString &message, const QString &receiver_login, const int &receiver_id,const QString& filePath, const QString &flag);
    void sendVoiceMessage(const QString &receiver_login, const int &receiver_id, const QString &flag);

    void sendRequestMessagesLoading(const int &chat_id, const QString &chat_name, const QString& flag, const int& offset);
signals:
    void sendMessageJson(const QJsonObject &messageJson);
    void sendFile(const QString& filePath,const QString &flag);
    void sendToFileServer(const QJsonDocument &doc);
private:
    QString activeUserLogin;
    int activeUserId;
    Logger* logger;
};

#endif // MESSAGESENDER_H
