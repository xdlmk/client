#include "filemanager.h"

FileManager::FileManager(QObject *parent)
    : QObject{parent}
{}

void FileManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserName = userName;
    activeUserId = userId;
}

QString FileManager::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        "Select file",
        "~",
        "All files (*)"
        );
    return filePath;
}

void FileManager::setLogger(Logger *logger)
{
    this->logger = logger;
}

void FileManager::uploadFiles(const QJsonObject &fileDataJson)
{
    logger->log(Logger::INFO,"filemanager.cpp::uploadFile", "uploadFiles start");
    QString fileDataString = fileDataJson["fileData"].toString();
    QByteArray fileData = QByteArray::fromBase64(fileDataString.toUtf8());
    QString fileUrl = extractFileName(fileDataJson["fileName"].toString());

    QDir dir(QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QFile file(QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName + "/" + fileUrl);
    if (!file.open(QIODevice::WriteOnly)) {
        logger->log(Logger::WARN,"filemanager.cpp::uploadFile", "Failed to save file");
        return;
    }
    file.write(fileData);
    file.close();
}

void FileManager::checkFileExists(const QString &fileUrl)
{
    QString fileName = extractFileName(fileUrl);
    if(QFile::exists(QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName + "/" + fileUrl)) {
        logger->log(Logger::INFO,"filemanager.cpp::checkFileExists", "Checking file exists");
    } else {
        emit getFile(fileUrl);
    }
}

QString FileManager::extractFileName(const QString &input)
{
    QRegularExpression regex("_([^_]+$)");
    QRegularExpressionMatch match = regex.match(input);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return QString();
}
