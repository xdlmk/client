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

#include <QImage>
#include <QColor>
#include <QFont>
#include <QPainter>

#include "Core/logger.h"

class MessageManager : public QObject
{
    Q_OBJECT
public:
    explicit MessageManager(QObject *parent = nullptr);
    void setActiveUser(const QString &userName, const int &userId);

    void checkingChatAvailability(QString &login, const QString &flag);

public slots:
    void saveMessageToJson(QString &userlogin, QString &message, QString &out, QString &time,
                           QString &fullDate, int message_id, int dialog_id, int id, QString &fileUrl);
    void saveGroupMessageToJson(QString &userlogin, QString &message, QString &out, QString &time,
                           QString &fullDate, int message_id, int id,QString &groupName,int group_id,
                           QString &fileUrl);
    void loadMessageToQml(const QString &username, const QString &message, const QString &out,
                          const QString &date, const QString &fileUrl = "");
    void saveMessageFromDatabase(QJsonObject &json);
    void savePersonalMessage(const QJsonObject &personalMessageJson);
    void saveGroupMessage(const QJsonObject &groupMessageJson);

    void loadingChat(const QString userlogin, const QString &flag);

    void loadingNextMessages(QJsonObject &messagesJson);

    void sendMessage(const QString &message, const QString &receiver_login, const int &receiver_id, const QString &flag);
    void saveMessageAndSendFile(const QString &message, const QString &receiver_login, const int &receiver_id,const QString& filePath, const QString &flag);
    void sendMessageWithFile(const QString &fileUrl,const QString &flag);
    void sendVoiceMessage(const QString &receiver_login, const int &receiver_id, const QString &flag);

    void requestMessageDownload(const int &chat_id, const QString &chat_name, const QString& flag, const int& offset);

    void setLogger(Logger* logger);
signals:
    void checkAndSendAvatarUpdate(const QString &avatar_url,const int &user_id,const QString& type);
    void checkActiveDialog(int user_id, QString login,QString message, QString out,
                           QString time,QString fileName,QString fileUrl, QString type);

    void newMessage(QString username,QString message,QString time,
                    QString fileUrl,QString fileName, bool isOutgoing);
    void clearMainListView();
    void showPersonalChat(QString login,QString message,int id,QString out, QString type);

    void sendAvatarsUpdate();
    void sendMessageJson(const QJsonObject &messageJson);
    void sendFile(const QString& filePath,const QString &flag);
    void sendToFileServer(const QJsonDocument &doc);

    void getContactList();
    void getChatsInfo();

    void insertMessage(QString username,QString message,QString time,
                       QString fileUrl,QString fileName, bool isOutgoing);
    void returnChatToPosition();

private:
    void generateAvatarImage(const QString& text, const int& id,const QString& type);

    QString activeUserName;
    int activeUserId;
    Logger* logger;
};

#endif // MESSAGEMANAGER_H
