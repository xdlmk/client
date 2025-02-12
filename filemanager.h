#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QCoreApplication>

#include <QObject>
#include <QByteArray>
#include <QDebug>
#include <QUuid>

#include <QFile>
#include <QFileDialog>
#include <QRegularExpression>

#include <QJsonObject>
#include <QJsonDocument>

#include "Logger/logger.h"

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = nullptr);
    void setActiveUser(const QString &userName, const int &userId);

    Q_INVOKABLE QString openFile();
signals:
    void getFile(const QString &fileUrl);

public slots:
    void setLogger(Logger* logger);
    void uploadFiles(const QJsonObject &fileDataJson);

    void checkFileExists(const QString &fileUrl);

private:
    QString extractFileName(const QString &input);

    QString activeUserName;
    int activeUserId;
    Logger* logger;
};

#endif // FILEMANAGER_H
