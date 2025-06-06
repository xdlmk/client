#ifndef FILENETWORKMANAGER_H
#define FILENETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QFile>
#include <QFileInfo>

#include <QMutex>
#include <QTimer>

#include "Utils/logger.h"

#include "generated_protobuf/envelope.qpb.h"
#include "generated_protobuf/identifiers.qpb.h"
#include "generated_protobuf/getAvatar.qpb.h"
#include "QProtobufSerializer"

class FileNetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit FileNetworkManager(QObject *parent = nullptr);

    QAbstractSocket::SocketState getSocketState() const;

public slots:
    void processSendFileQueue();

    void connectToFileServer();
    void sendData(const QString &flag, const QByteArray &data);
    void sendAvatar(const QString &avatarPath, const QString &type, const int& id);

    void setActiveUser(const QString &userName, const int &userId);
    void setLogger(Logger *logger);

signals:
    void uploadFiles(const QByteArray &fileData);
    void uploadVoiceFile(const QByteArray &fileData);
    void uploadAvatar(const QByteArray &data);
    void sendAvatarUrl(const QString &avatar_url, const int& user_id, const QString& type);

    void onDisconnected();

private slots:
    void onFileServerReceived();
    void handleFileBytesWritten(qint64 bytes);

private:

    QTcpSocket* fileSocket;
    QByteArray writeBuffer;
    QQueue<QByteArray> sendFileQueue;
    QMutex fileMutex;

    QString activeUserLogin;
    quint64 activeUserId;

    Logger* logger;

    static const std::unordered_map<std::string_view, uint> flagMap;
    const int MAX_QUEUE_SIZE = 1000;
};

#endif // FILENETWORKMANAGER_H
