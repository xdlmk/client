#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QByteArray>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QTimer>

#include "Utils/logger.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void connectToServer();

public slots:
    void sendData(const QJsonObject &jsonToSend);
    void sendToFileServer(const QJsonDocument &doc);
    void sendFile(const QString &filePath,const QString &flag);
    void sendAvatar(const QString &avatarPath, const QString &type, const int& id);

    void setActiveUser(const QString &userName,const int &userId);
    void setLogger(Logger *logger);
signals:
    void dataReceived(const QJsonDocument &doc);
    void uploadFiles(const QJsonObject &fileDataJson);
    void uploadVoiceFile(const QJsonObject &fileDataJson);
    void uploadAvatar(const QJsonObject &avatarDataJson);
    void sendAvatarUrl(const QString &avatar_url,const int& user_id, const QString& type);

    void messageReceived(const QJsonObject &receivedMessageJson);
    void groupMessageReceived(const QJsonObject &receivedMessageJson);
    void groupInfoReceived(const QJsonObject &receivedGroupInfoJson);
    void deleteGroupMemberReceived(const QJsonObject &receivedDeleteMemberFromGroup);
    void addGroupMemberReceived(const QJsonObject &receivedAddMemberFromGroup);
    void dialogsInfoReceived(const QJsonObject &receivedDialogInfoJson);
    void loginResultsReceived(const QJsonObject &loginResultsJson);
    void registrationResultsReceived(const QJsonObject &registrationResultsJson);
    void searchDataReceived(const QJsonObject &searchDataJson);
    void chatsUpdateDataReceived(QJsonObject &chatsUpdateDataJson);
    void loadMeassgesReceived(QJsonObject &messagesJson);
    void editResultsReceived(const QJsonObject &editResultsJson);
    void avatarsUpdateReceived(const QJsonObject &avatarsUpdateJson);

    void sendMessageWithFile(const QString& fileUrl,const QString &flag);

    void connectionError();
    void connectionSuccess();

    void removeAccountFromConfigManager();


private slots:
    void onDisconnected();
    void attemptReconnect();

    void onDataReceived();
    void onFileServerReceived();

private:
    QString activeUserLogin;
    int activeUserId;

    QTcpSocket* fileSocket;
    QTcpSocket* socket;
    QTimer reconnectTimer;
    Logger* logger;
};

#endif // NETWORKMANAGER_H
