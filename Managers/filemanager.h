#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QCoreApplication>

#include <QObject>
#include <QByteArray>
#include <QDebug>
#include <QUuid>

#include <QFile>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QCryptographicHash>

#include "Utils/logger.h"

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = nullptr);
    void setActiveUser(const QString &userName, const int &userId);

    Q_INVOKABLE QString openFile(QString type);
signals:
    void sendToFileServer(const QJsonDocument& avatarUrlDoc);
    void voiceExists();

public slots:
    void sendAvatarUrl(const QString& avatar_url,const int& user_id, const QString& type);
    void setLogger(Logger* logger);
    void uploadFiles(const QJsonObject &fileDataJson);
    void uploadVoiceFile(const QJsonObject &fileDataJson);
    void uploadAvatar(const QJsonObject &avatarDataJson);

    void getFile(const QString &fileUrl,const QString &flag);

private:
    QString replaceAfterUnderscore(const QString &url, const QString &newString);
    QString generateUniqueFileName(const QString &baseName, const QString &directoryPath);

    void checkingForFileChecker();
    QString calculateDataHash(const QByteArray& data);
    bool isFileDownloaded(const QString &fileUrl,QString &filePath,const QString &downloadFilesDir);
    bool checkJsonForMatches(QJsonArray &checkerArray, const QByteArray &fileData, QString &fileUrl);

    QJsonArray loadJsonArrayFromFile(QFile &fileChecker);
    QString extractFileName(const QString &input);

    QString activeUserName;
    int activeUserId;
    Logger* logger;
};

#endif // FILEMANAGER_H
