#ifndef FILENETWORKMANAGER_H
#define FILENETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QFile>
#include <QFileInfo>

#include <QJsonObject>
#include <QJsonDocument>

#include <QMutex>
#include <QTimer>

#include "Utils/logger.h"

class FileNetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit FileNetworkManager(QObject *parent = nullptr);

    QAbstractSocket::SocketState getSocketState() const;

public slots:
    void processSendFileQueue();

    void connectToFileServer();
    void sendToFileServer(const QJsonDocument &doc);
    void sendFile(const QString &filePath, const QString &flag);
    void sendAvatar(const QString &avatarPath, const QString &type, const int& id);

    void setActiveUser(const QString &userName, const int &userId);
    void setLogger(Logger *logger);

signals:
    void uploadFiles(const QJsonObject &fileDataJson);
    void uploadVoiceFile(const QJsonObject &fileDataJson);
    void uploadAvatar(const QJsonObject &avatarDataJson);
    void sendAvatarUrl(const QString &avatar_url, const int& user_id, const QString& type);

    void sendMessageWithFile(const QString& fileUrl,const QString &flag);

    void onDisconnected();

private slots:
    void onFileServerReceived();
    void handleFileBytesWritten(qint64 bytes);

private:

    QTcpSocket* fileSocket;
    QQueue<QByteArray> sendFileQueue;
    QMutex fileMutex;

    QString activeUserLogin;
    int activeUserId;

    Logger* logger;

    static const std::unordered_map<std::string_view, uint> flagMap;
    const int MAX_QUEUE_SIZE = 1000;
};

#endif // FILENETWORKMANAGER_H
