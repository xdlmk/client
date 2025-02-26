#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>

#include "Logger/logger.h"

class MessageManager : public QObject
{
    Q_OBJECT
public:
    explicit MessageManager(QObject *parent = nullptr);
    void setActiveUser(const QString &userName, const int &userId);

    void checkingChatAvailability(QString &login);

public slots:
    void saveMessageToJson(QString &userlogin, QString &message, QString &out, QString &time,
                           QString &fullDate, int message_id, int dialog_id, int id, QString &fileUrl);
    void loadMessageToQml(const QString &username, const QString &message, const QString &out,
                          const QString &date, const QString &fileUrl = "");
    void loadMessagesFromJson(const QString &filepath);
    void saveMessageFromDatabase(QJsonObject &json);

    void loadingPersonalChat(const QString userlogin);

    void sendPersonalMessage(const QString &message, const QString &receiver_login, const int &receiver_id);
    void saveMessageAndSendFile(const QString &message, const QString &receiver_login, const int &receiver_id,const QString& filePath);
    void sendPersonalMessageWithFile(const QString &fileUrl);
    void sendVoiceMessage(const QString &receiver_login, const int &receiver_id);

    void setLogger(Logger* logger);
signals:
    void newMessage(QString username,QString message,QString time,
                    QString fileUrl,QString fileName, bool isOutgoing);
    void clearMainListView();
    void showPersonalChat(QString login,QString message,int id,QString out);

    void sendAvatarsUpdate();
    void sendMessageJson(const QJsonObject &messageJson);
    void sendFile(const QString& filePath);
    void sendToFileServer(const QJsonDocument &doc);

private:
    QString activeUserName;
    int activeUserId;
    Logger* logger;
};

#endif // MESSAGEMANAGER_H
