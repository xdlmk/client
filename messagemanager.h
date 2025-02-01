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

class MessageManager : public QObject
{
    Q_OBJECT
public:
    explicit MessageManager(QObject *parent = nullptr);


    void setActiveUser(const QString &userName, const int &userId);
    void checkingChatAvailability(QString &login);

public slots:
    void saveMessageToJson(QString &userlogin, QString &message, QString &out, QString &time,
                           QString &fullDate, int message_id, int dialog_id, int id);
    void loadMessageToQml(const QString &username, const QString &message, const QString &out,
                          const QString &date);
    void loadMessagesFromJson(const QString &filepath);
    void saveMessageFromDatabase(QJsonObject &json);

    void loadingPersonalChat(const QString userlogin);

    void sendPersonalMessage(const QString &message, const QString &receiver_login, const int &receiver_id);
signals:
    void newOutMessage(QString name,QString message,QString time);
    void newInMessage(QString name,QString message,QString time);
    void clearMainListView();
    void showPersonalChat(QString login,QString message,int id,QString out);

    void sendMessageJson(const QJsonObject &messageJson);

private:
    QString activeUserName;
    int activeUserId;
};

#endif // MESSAGEMANAGER_H
