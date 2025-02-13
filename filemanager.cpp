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
    logger->log(Logger::INFO,"filemanager.cpp::uploadFiles", "uploadFiles start");
    QString fileDataString = fileDataJson["fileData"].toString();
    QByteArray fileData = QByteArray::fromBase64(fileDataString.toUtf8());
    QString fileUrl = extractFileName(fileDataJson["fileName"].toString());

    checkingForFileChecker();
    QFile fileChecker(QCoreApplication::applicationDirPath() + "/.fileChecker/" + activeUserName + "/checker.json");
    QJsonArray checkerArray = loadJsonArrayFromFile(fileChecker);

    if(!checkJsonForMatches(checkerArray,fileData,fileDataJson["fileName"].toString())) {
        logger->log(Logger::INFO,"filemanager.cpp::uploadFiles", "checkJsonForMatches returning true");
        QDir dir(QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QFile file(QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName + "/" + fileUrl);
        if (!file.open(QIODevice::WriteOnly)) {
            logger->log(Logger::WARN,"filemanager.cpp::uploadFiles", "Failed to save file");
            return;
        }
        if (!fileChecker.open(QIODevice::WriteOnly)) {
            logger->log(Logger::WARN,"filemanager.cpp::uploadFiles", "Failed to save fileChecker");
            return;
        }
        QJsonDocument jsonDoc(checkerArray);
        fileChecker.write(jsonDoc.toJson());
        fileChecker.close();

        file.write(fileData);
        file.close();
    }
}

void FileManager::checkingForFileChecker()
{
    logger->log(Logger::INFO,"filemanager.cpp::checkingForFileChecker", "checkingForFileChecker starts");
    QFile fileChecker(QCoreApplication::applicationDirPath() + "/.fileChecker/" + activeUserName + "/checker.json");
    QDir dirFileChecker(QCoreApplication::applicationDirPath() + "/.fileChecker/" + activeUserName);
    QDir dirUploadFiles(QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName);
    if (dirFileChecker.exists()) {
        if (!fileChecker.exists())
        {
            dirUploadFiles.removeRecursively();
            if(fileChecker.open(QIODevice::WriteOnly)){
                fileChecker.close();
            } else {
                logger->log(Logger::WARN,"filemanager.cpp::checkingForFileChecker", "Failed to create fileChecker");
            }
        }
    } else {
        dirUploadFiles.removeRecursively();
        dirFileChecker.mkpath(".");
    }
}

QString FileManager::calculateDataHash(const QByteArray &data)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return QString(hash.result().toHex());
}

bool FileManager::checkJsonForMatches(QJsonArray &checkerArray, const QByteArray &fileData, const QString &fileUrl)
{
    logger->log(Logger::INFO,"filemanager.cpp::checkJsonForMatches", "checkJsonForMatches starts");
    QFile file(QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName + "/" + extractFileName(fileUrl));
    if(file.exists()){
        for (const auto &item : checkerArray) {
            if (!item.isObject()) {
                logger->log(Logger::WARN,"filemanager.cpp::checkJsonForMatches", "Element is not a JSON object. Skipping");
                continue;
            }

            QJsonObject jsonObject = item.toObject();

            QString jsonFileUrl = jsonObject["fileUrl"].toString();

            if (jsonFileUrl == fileUrl) {
                logger->log(Logger::DEBUG,"filemanager.cpp::checkJsonForMatches", "fileUrl matches");
                QString jsonFileHash = jsonObject["fileHash"].toString();
                if (jsonFileHash == calculateDataHash(fileData)) {
                    logger->log(Logger::DEBUG,"filemanager.cpp::checkJsonForMatches", "fileUrl and dataHash match");
                    return true;
                }
            }
        }
    }

    QJsonObject newFileObject;
    newFileObject["fileUrl"] = fileUrl;
    newFileObject["fileHash"] = calculateDataHash(fileData);
    checkerArray.append(newFileObject);

    logger->log(Logger::INFO,"filemanager.cpp::checkJsonForMatches", "returning false");
    return false;
}

QJsonArray FileManager::loadJsonArrayFromFile(QFile &fileChecker)
{
    logger->log(Logger::INFO,"filemanager.cpp::loadJsonArrayFromFile", "loadJsonArrayFromFile starts");
    if(!fileChecker.open(QIODevice::ReadWrite)) {
        logger->log(Logger::WARN,"filemanager.cpp::loadJsonArrayFromFile", "Failed to open filechecker");
    }
    QByteArray fileCheckerData = fileChecker.readAll();
    fileChecker.close();
    QJsonDocument fileCheckerDoc = QJsonDocument::fromJson(fileCheckerData);

    return fileCheckerDoc.array();
}

QString FileManager::extractFileName(const QString &input)
{
    QRegularExpression regex("_([^_]*$)");
    QRegularExpressionMatch match = regex.match(input);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return QString();
}
