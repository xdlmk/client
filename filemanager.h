#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QCoreApplication>

#include <QObject>
#include <QByteArray>
#include <QDebug>
#include <QUuid>

#include <QFile>
#include <QFileDialog>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QCryptographicHash>

#include "Logger/logger.h"

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = nullptr);
    void setActiveUser(const QString &userName, const int &userId);

    Q_INVOKABLE QString openFile();
signals:

public slots:
    void setLogger(Logger* logger);
    void uploadFiles(const QJsonObject &fileDataJson);

private:
    QString replaceAfterUnderscore(const QString &url, const QString &newString);
    QString generateUniqueFileName(const QString &baseName, const QString &directoryPath);

    void checkingForFileChecker();
    QString calculateDataHash(const QByteArray& data);
    bool checkJsonForMatches(QJsonArray &checkerArray, const QByteArray &fileData, QString &fileUrl);

    QJsonArray loadJsonArrayFromFile(QFile &fileChecker);
    QString extractFileName(const QString &input);

    QString activeUserName;
    int activeUserId;
    Logger* logger;
};

#endif // FILEMANAGER_H
