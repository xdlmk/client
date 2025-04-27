#include "filemanager.h"

FileManager::FileManager(QObject *parent)
    : QObject{parent}
{}

void FileManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserName = userName;
    activeUserId = userId;
}

QString FileManager::openFile(QString type)
{
    if ( type == "Image") {
        QString filePath = QFileDialog::getOpenFileName(
            nullptr,
            "Select an image file",
            "~",
            "Image files (*.png *.jpeg *.jpg)"
            );
        return filePath;
    } else {
        QString filePath = QFileDialog::getOpenFileName(
            nullptr,
            "Select file",
            "~",
            "All files (*)"
            );
        return filePath;
    }
    return "";
}

void FileManager::sendAvatarUrl(const QString &avatar_url,const int& user_id, const QString& type)
{
    avatars::AvatarRequest request;
    request.setType(type);
    request.setAvatarUrl(avatar_url);
    request.setUserId(user_id);

    QProtobufSerializer serializer;
    QByteArray data = request.serialize(&serializer);

    emit sendDataFile("avatarUrl", data);
}

void FileManager::setLogger(Logger *logger)
{
    this->logger = logger;
}

void FileManager::uploadFiles(const QByteArray &fileData)
{
    logger->log(Logger::INFO,"filemanager.cpp::uploadFiles", "uploadFiles start");

    files::FileData response;
    QProtobufSerializer serializer;
    if(!response.deserialize(&serializer, fileData)) {
        logger->log(Logger::WARN,"filemanager.cpp::uploadFiles", "Failed to deserialize FileRequest");
        return;
    }
    QByteArray newFileData = response.fileData();
    QString fileUrl = response.fileName();

    files::FileChecker fileChecker = loadFileChecker();

    if (!checkProtoForMatches(fileChecker, newFileData, fileUrl)) {
        QDir dir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/uploads/");
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QString localFilePath = dir.path() + "/" + fileUrl;
        QFile file(localFilePath);
        if (!file.open(QIODevice::WriteOnly)) {
            logger->log(Logger::WARN, "filemanager.cpp::uploadFiles", "Failed to save file");
            return;
        }
        file.write(newFileData);
        file.close();

        saveFileChecker(fileChecker);
    } else {
        QString localFilePath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/uploads/" + fileUrl;
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(localFilePath))) {
            logger->log(Logger::WARN, "filemanager.cpp::uploadFiles", "Failed to open file");
        }
    }
}

void FileManager::uploadVoiceFile(const QByteArray &fileData)
{
    logger->log(Logger::INFO,"filemanager.cpp::uploadVoiceFile", "uploadVoiceFile start");
    files::FileData response;
    QProtobufSerializer serializer;
    if(!response.deserialize(&serializer, fileData)) {
        logger->log(Logger::WARN,"filemanager.cpp::uploadFiles", "Failed to deserialize FileRequest");
        return;
    }
    QByteArray newFileData = response.fileData();
    QString fileUrl = response.fileName();

    files::FileChecker fileChecker = loadFileChecker();

    if (!checkProtoForMatches(fileChecker, newFileData, fileUrl)) {
        QDir dir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/.voiceFiles/");
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        QFile file(dir.path() + "/" + fileUrl);
        if (!file.open(QIODevice::WriteOnly)) {
            logger->log(Logger::WARN, "filemanager.cpp::uploadVoiceFile", "Failed to save voice file");
            return;
        }
        file.write(newFileData);
        file.close();

        saveFileChecker(fileChecker);
    }
}

void FileManager::uploadAvatar(const QByteArray &data)
{
    logger->log(Logger::INFO,"filemanager.cpp::uploadAvatar", "uploadAvatar start");

    QProtobufSerializer serializer;
    avatars::AvatarData avatarProto;
    if (!avatarProto.deserialize(&serializer, data)) {
        logger->log(Logger::WARN, "filemanager.cpp::uploadAvatar", "Failed to deserialize AvatarData");
        return;
    }
    QString type = avatarProto.type();
    quint64 userId = avatarProto.userId();
    QByteArray avatarData = avatarProto.avatarData();
    QString avatarUrl = avatarProto.avatarUrl();

    QString chatType = (type == "personal") ? "/dialogsInfo/" : "/groupsInfo/";
    QString pathToInfo = QCoreApplication::applicationDirPath() + "/.data/" +
                         QString::number(activeUserId) + chatType +
                         QString::number(userId) + ".pb";

    QFileInfo fileInfo(pathToInfo);
    QDir infoDir(fileInfo.path());
    if (!infoDir.exists()) {
        infoDir.mkpath(".");
    }

    QFile infoFile(pathToInfo);

    if (!infoFile.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"filemanager.cpp::uploadAvatar", "Failed to read info file: " + pathToInfo);
    } else {
        QByteArray fileData = infoFile.readAll();
        infoFile.close();

        if (type == "personal") {
            chats::DialogInfoItem dialogItem;
            if (!dialogItem.deserialize(&serializer, fileData)) {
                logger->log(Logger::WARN, "filemanager.cpp::uploadAvatar",
                            "Failed to deserialize DialogInfoItem from: " + pathToInfo);
            } else {
                dialogItem.setAvatarUrl(avatarUrl);
                QByteArray updatedData = dialogItem.serialize(&serializer);

                if (!infoFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    logger->log(Logger::WARN, "filemanager.cpp::uploadAvatar",
                                "Failed to open info pb file for writing: " + pathToInfo);
                } else {
                    infoFile.write(updatedData);
                    infoFile.close();
                }
            }
        } else if (type == "group"){
            chats::GroupInfoItem groupItem;
            if (!groupItem.deserialize(&serializer, fileData)) {
                logger->log(Logger::WARN, "filemanager.cpp::uploadAvatar",
                            "Failed to deserialize GroupInfoItem from: " + pathToInfo);
            } else {
                groupItem.setAvatarUrl(avatarUrl);
                QByteArray updatedData = groupItem.serialize(&serializer);

                if (!infoFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    logger->log(Logger::WARN, "filemanager.cpp::uploadAvatar",
                                "Failed to open info pb file for writing: " + pathToInfo);
                } else {
                    infoFile.write(updatedData);
                    infoFile.close();
                }
            }
        }
    }


    QString pathToSave = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/avatars/" + type;
    QDir avatarDir(pathToSave);
    if (!avatarDir.exists()) {
        avatarDir.mkpath(".");
    }
    QFile avatar(pathToSave + "/" + QString::number(userId) + ".png");

    if (!avatar.open(QIODevice::WriteOnly)) {
        logger->log(Logger::WARN,"filemanager.cpp::uploadAvatar", "Failed to save avatar");
        return;
    }
    avatar.write(avatarData);
    avatar.close();
}

void FileManager::getFile(const QString &fileUrl, const QString &flag)
{
    logger->log(Logger::INFO,"filemanager.cpp::getFile","getFile starts for flag: " + flag);
    QString filePath = "";
    QString filesDir;
    if(flag == "fileUrl") {
        filesDir = QCoreApplication::applicationDirPath() + "/.data/" +
                   QString::number(activeUserId) + "/uploads/";
    } else if (flag == "voiceFileUrl") {
        filesDir = QCoreApplication::applicationDirPath() + "/.data/" +
                   QString::number(activeUserId) + "/.voiceFiles/";
    }
    if(!isFileDownloaded(fileUrl,filePath,filesDir)) {
        files::FileRequest request;
        request.setFileUrl(fileUrl);
        QProtobufSerializer serializer;
        emit sendDataFile(flag, request.serialize(&serializer));
    } else {
        logger->log(Logger::INFO,"filemanager.cpp::getFile","File is downloaded: " + filePath);
        if(flag == "fileUrl") {
            if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))) {
                logger->log(Logger::WARN,"filemanager.cpp::getFile", "Failed to open file");
            }
        } else if(flag == "voiceFileUrl") {
            emit voiceExists();
        }
    }
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
    QFile fileChecker(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/.fileChecker/checker.json");
    QDir dirFileChecker(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/.fileChecker/" );
    QDir dirUploadFiles(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/uploads/");
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

bool FileManager::isFileDownloaded(const QString &fileUrl,QString &filePath,const QString &downloadFilesDir)
{
    files::FileChecker fileChecker = loadFileChecker();

    for (auto it = fileChecker.items().begin(); it != fileChecker.items().end();) {

        QString savedFileUrl = it->fileUrl();
        QString localFileName = it->fileName();

        if (savedFileUrl == fileUrl) {
            QFile file(downloadFilesDir + localFileName);
            if(file.exists()) {
                if (!file.open(QIODevice::ReadOnly)) {
                    logger->log(Logger::WARN,"filemanager.cpp::isFileDownloaded", "Failed to open file: " + file.errorString());
                    file.remove();

                    QList<files::FileCheckItem> items = fileChecker.items();
                    items.erase(it);
                    fileChecker.setItems(items);

                    continue;
                }
                QByteArray fileData = file.readAll();
                file.close();
                if (it->fileHash() == calculateDataHash(fileData)) {
                    filePath = downloadFilesDir + localFileName;
                    logger->log(Logger::DEBUG,"filemanager.cpp::isFileDownloaded", "File downloaded");
                    return true;
                } else {
                    QList<files::FileCheckItem> items = fileChecker.items();
                    items.erase(it);
                    fileChecker.setItems(items);
                    continue;
                }
            } else {
                QList<files::FileCheckItem> items = fileChecker.items();
                items.erase(it);
                fileChecker.setItems(items);
                continue;
            }
        }
        ++it;
    }
    saveFileChecker(fileChecker);

    logger->log(Logger::DEBUG,"filemanager.cpp::isFileDownloaded", "File not downloaded");
    return false;
}

bool FileManager::checkProtoForMatches(files::FileChecker &fileChecker, const QByteArray &fileData, QString &fileUrl)
{
    logger->log(Logger::INFO, "filemanager.cpp::checkProtoForMatches", "checkProtoForMatches starts");

    QString filesDir = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/uploads/";

    for (auto it = fileChecker.items().begin(); it != fileChecker.items().end(); ) {
        if (it->fileUrl() == fileUrl) {
            QString localFileName = it->fileName();
            QFile file(filesDir + "/" + localFileName);
            if (calculateDataHash(file.readAll()) == it->fileHash()) {
                if (file.exists()) {
                    fileUrl = localFileName;
                    return true;
                } else {
                    QList<files::FileCheckItem> items = fileChecker.items();
                    items.erase(it);
                    fileChecker.setItems(items);
                    continue;
                }
            }
        }
        ++it;
    }

    files::FileCheckItem newItem;
    newItem.setFileUrl(fileUrl);
    newItem.setFileName(generateUniqueFileName(fileUrl, filesDir));
    newItem.setFileHash(calculateDataHash(fileData));

    QList<files::FileCheckItem> listItems = fileChecker.items();
    listItems.append(newItem);
    fileChecker.setItems(listItems);

    logger->log(Logger::INFO, "filemanager.cpp::checkProtoForMatches", "File not found. Added to checker.");
    return false;
}

files::FileChecker FileManager::loadFileChecker()
{
    QFile file(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/.fileChecker/checker.dat");
    files::FileChecker fileChecker;

    if (file.open(QIODevice::ReadOnly)) {
        QProtobufSerializer serializer;
        if (fileChecker.deserialize(&serializer, file.readAll())) {
            file.close();
            return fileChecker;
        }
        file.close();
    }

    logger->log(Logger::WARN, "filemanager.cpp::loadFileChecker", "Failed to load FileChecker");
    return fileChecker;
}

bool FileManager::saveFileChecker(const files::FileChecker &fileChecker)
{
    QFile file(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/.fileChecker/checker.dat");
    if (file.open(QIODevice::WriteOnly)) {
        QProtobufSerializer serializer;
        if(!file.write(fileChecker.serialize(&serializer))){
            logger->log(Logger::WARN, "filemanager.cpp::saveFileChecker", "Failed to write FileChecker");
            return false;
        }
        file.close();
        logger->log(Logger::INFO, "filemanager.cpp::saveFileChecker", "FileChecker saved successfully");
    } else {
        logger->log(Logger::WARN, "filemanager.cpp::saveFileChecker", "Failed to save FileChecker");
        return false;
    }
    return true;
}

QString FileManager::extractFileName(const QString &input)
{
    int underscoreIndex = input.indexOf('_');
    if (underscoreIndex != -1 && underscoreIndex + 1 < input.length()) {
        return input.mid(underscoreIndex + 1);
    }
    return QString();
}
