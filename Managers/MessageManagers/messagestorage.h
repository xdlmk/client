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

class MessageStorage : public QObject
{
    Q_OBJECT
public:
    explicit MessageStorage(QObject *parent = nullptr);
    void setActiveUser(const QString &userLogin, const int &userId);
    void setLogger(Logger* logger);

    void saveMessageToJson(const QJsonObject& messageToSave);
    void saveGroupMessageToJson(const QJsonObject& messageToSave);
public slots:
    void updatingLatestMessagesFromServer(QJsonObject &latestMessages);
signals:
    void showPersonalChat(const QString& login, const QString& message, const int& id, const QString& out, const QString& type);

    void sendAvatarsUpdate();
    void getContactList();
private:
    QString activeUserLogin;
    int activeUserId;
    Logger* logger;
};

#endif // MESSAGESTORAGE_H
