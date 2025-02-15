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

void FileManager::sendAvatarUrl(const QString &avatar_url,const int& user_id)
{
    QJsonObject avatarUrlJson;
    avatarUrlJson["flag"] = "avatar";
    avatarUrlJson["avatar_url"] = avatar_url;
    avatarUrlJson["user_id"] = user_id;
    QJsonDocument doc(avatarUrlJson);
    emit sendToFileServer(doc);
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
    QString fileUrl = fileDataJson["fileName"].toString();

    checkingForFileChecker();
    QFile fileChecker(QCoreApplication::applicationDirPath() + "/.fileChecker/" + activeUserName + "/checker.json");
    QJsonArray checkerArray = loadJsonArrayFromFile(fileChecker);

    if(!checkJsonForMatches(checkerArray,fileData,fileUrl)) {
        QDir dir(QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        fileUrl = extractFileName(fileUrl);
        qDebug() << "File name from upload: "<<fileUrl;
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

void FileManager::uploadAvatar(const QJsonObject &avatarDataJson)
{
    logger->log(Logger::INFO,"filemanager.cpp::uploadAvatar", "uploadAvatar start");
    QString avatarDataString = avatarDataJson["avatarData"].toString();
    QByteArray avatarData = QByteArray::fromBase64(avatarDataString.toUtf8());

    checkingForFileChecker();
    QFile avatarChecker(QCoreApplication::applicationDirPath() + "/.fileChecker/" + activeUserName + "/avatarChecker.json");
    QJsonArray avatarCheckerArray = loadJsonArrayFromFile(avatarChecker);

    QJsonObject newAvatarDataJson;
    //qDebug()<< "uploadAvatar::user_id: " << QString::number(avatarDataJson["user_id"].toInt());
    //qDebug()<< "uploadAvatar::avatar_url: " << avatarDataJson["avatar_url"].toString();
    newAvatarDataJson["user_id"] = avatarDataJson["user_id"].toInt();
    newAvatarDataJson["avatar_url"] = avatarDataJson["avatar_url"].toString();
    avatarCheckerArray.append(newAvatarDataJson);

    if (!avatarChecker.open(QIODevice::WriteOnly)) {
        logger->log(Logger::WARN,"filemanager.cpp::uploadAvatar", "Failed to save avatarChecker");
        return;
    }
    QJsonDocument jsonDoc(avatarCheckerArray);
    avatarChecker.write(jsonDoc.toJson());
    avatarChecker.close();
    QDir avatarDir(QCoreApplication::applicationDirPath() + "/avatars/" + activeUserName);
    if (!avatarDir.exists()) {
        avatarDir.mkpath(".");
    }
    QFile avatar(QCoreApplication::applicationDirPath() + "/avatars/" + activeUserName + "/" + QString::number(avatarDataJson["user_id"].toInt()) + ".png");

    if (!avatar.open(QIODevice::WriteOnly)) {
        logger->log(Logger::WARN,"filemanager.cpp::uploadAvatar", "Failed to save avatar");
        return;
    }
    avatar.write(avatarData);
    avatar.close();
}

QString FileManager::replaceAfterUnderscore(const QString &url, const QString &newString)
{
    int underscoreIndex = url.indexOf('_');

    if (underscoreIndex != -1) {
        return url.left(underscoreIndex + 1) + newString;
    }

    return url + "_" + newString;
}

QString FileManager::generateUniqueFileName(const QString &baseName, const QString &directoryPath)
{
    QString fileName = baseName;
    QString fileExtension;

    int dotIndex = baseName.lastIndexOf('.');
    if (dotIndex != -1) {
        fileExtension = baseName.mid(dotIndex);
        fileName = baseName.left(dotIndex);
    }

    QString uniqueFileName = baseName;
    int suffix = 1;

    QDir directory(directoryPath);

    while (directory.exists(uniqueFileName)) {
        uniqueFileName = QString("%1 (%2)%3").arg(fileName).arg(suffix).arg(fileExtension);
        suffix++;
    }

    return uniqueFileName;
}

void FileManager::checkingForFileChecker()
{
    logger->log(Logger::INFO,"filemanager.cpp::checkingForFileChecker", "checkingForFileChecker starts");
    QFile fileChecker(QCoreApplication::applicationDirPath() + "/.fileChecker/" + activeUserName + "/checker.json");
    QFile avatarChecker(QCoreApplication::applicationDirPath() + "/.fileChecker/" + activeUserName + "/avatarChecker.json");
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
            if(avatarChecker.open(QIODevice::WriteOnly)){
                avatarChecker.close();
            } else {
                logger->log(Logger::WARN,"filemanager.cpp::checkingForFileChecker", "Failed to create avatarChecker");
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

bool FileManager::checkJsonForMatches(QJsonArray &checkerArray, const QByteArray &fileData, QString &fileUrl)
{
    logger->log(Logger::INFO,"filemanager.cpp::checkJsonForMatches", "checkJsonForMatches starts");
    QString filesDir = QCoreApplication::applicationDirPath() + "/uploads/" + activeUserName;
    for (int i = 0; i < checkerArray.size(); ++i) {
        QJsonValue item = checkerArray.at(i);
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
                QString localFileName = jsonObject["fileName"].toString();
                QFile file(filesDir + "/" + localFileName);
                if(file.exists()) {
                    return true;
                } else {
                    checkerArray.removeAt(i);
                }
            }
        }

    }

    QJsonObject newFileObject;
    newFileObject["fileUrl"] = fileUrl;
    QString uniqName = generateUniqueFileName(extractFileName(fileUrl),filesDir);
    newFileObject["fileName"] = uniqName;
    qDebug() << "1 File url from checkJson: " << fileUrl;
    fileUrl = replaceAfterUnderscore(fileUrl,uniqName);
    qDebug() << "2 File url from checkJson: " << fileUrl;
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
    int underscoreIndex = input.indexOf('_');
    if (underscoreIndex != -1 && underscoreIndex + 1 < input.length()) {
        return input.mid(underscoreIndex + 1);
    }
    return QString();
}
