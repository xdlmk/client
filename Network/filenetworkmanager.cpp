#include "filenetworkmanager.h"

FileNetworkManager::FileNetworkManager(QObject *parent)
    : QObject{parent}
{
    fileSocket = new QTcpSocket(this);

    activeUserId = 0;
    activeUserLogin = "";

    QObject::connect(fileSocket, &QTcpSocket::connected, [this]() {
        logger->log(Logger::INFO,"filenetworkmanager.cpp::constructor","Connection to the FileServer established");
        QJsonObject setIdentifiers;
        setIdentifiers["flag"] = "identifiers";
        setIdentifiers["userlogin"] = activeUserLogin;
        setIdentifiers["user_id"] = activeUserId;
        sendToFileServer(QJsonDocument(setIdentifiers));

        {
            QMutexLocker lock(&fileMutex);
            if (!sendFileQueue.isEmpty()) {
                QMetaObject::invokeMethod(this, "processSendFileQueue", Qt::QueuedConnection);
            }
        }
    });
    connect(fileSocket,&QTcpSocket::readyRead,this,&FileNetworkManager::onFileServerReceived);
    connect(fileSocket,&QTcpSocket::disconnected,this,&FileNetworkManager::onDisconnected);
    connect(fileSocket,&QTcpSocket::bytesWritten,this,&FileNetworkManager::handleFileBytesWritten);
    connect(fileSocket, &QAbstractSocket::errorOccurred, [this](QAbstractSocket::SocketError socketError) {
        emit onDisconnected();
        logger->log(Logger::ERROR,"filenetworkmanager.cpp::constructor","Connection to the FileServer error: " + fileSocket->errorString());
    });
}

QAbstractSocket::SocketState FileNetworkManager::getSocketState() const
{
    return fileSocket->state();
}

void FileNetworkManager::connectToFileServer()
{
    QFile file("ip.txt");
    if (!file.exists() || file.size() == 0) {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write("127.0.0.1\n");
        file.close();
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString ip = QString::fromUtf8(file.readLine()).trimmed();
    file.close();
    fileSocket->connectToHost(ip,2021);
}

void FileNetworkManager::sendToFileServer(const QJsonDocument &doc)
{
    logger->log(Logger::INFO,"filenetworkmanager.cpp::sendToFileServer","sendToFileServer starts");
    QByteArray fileDataOut = doc.toJson(QJsonDocument::Compact);

    bool shouldStartProcessing = false;
    {
        QMutexLocker lock(&fileMutex);
        if (sendFileQueue.size() >= MAX_QUEUE_SIZE) {
            logger->log(Logger::DEBUG, "filenetworkmanager.cpp::sendToFileServer", "Send queue overflow! Dropping message.");
            return;
        }
        sendFileQueue.enqueue(fileDataOut);
        logger->log(Logger::INFO, "filenetworkmanager.cpp::sendToFileServer", "Message added to queue. Queue size: " + QString::number(sendFileQueue.size()));
        shouldStartProcessing = sendFileQueue.size() == 1;
    }

    if (shouldStartProcessing) {
        logger->log(Logger::INFO, "filenetworkmanager.cpp::sendToFileServer", "Starting to process the send queue.");
        processSendFileQueue();
    }
}

void FileNetworkManager::sendFile(const QString &filePath,const QString &flag)
{
    logger->log(Logger::INFO,"filenetworkmanager.cpp::sendFile","Sending file");
    logger->log(Logger::INFO,"filenetworkmanager.cpp::sendFile","filePath = " + filePath);
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"filenetworkmanager.cpp::sendFile","Failed open file");
    }

    QByteArray fileData = file.readAll();
    file.close();

    QJsonObject fileDataJson;
    fileDataJson["flag"] = flag;
    fileDataJson["fileName"] = fileInfo.baseName();
    fileDataJson["fileExtension"] = fileInfo.suffix();
    fileDataJson["fileData"] = QString(fileData.toBase64());

    QJsonDocument doc(fileDataJson);
    sendToFileServer(doc);
}

void FileNetworkManager::sendAvatar(const QString &avatarPath, const QString &type, const int& id)
{
    logger->log(Logger::INFO,"filenetworkmanager.cpp::sendAvatar","Sending avatar");
    QFile file(avatarPath);
    QFileInfo fileInfo(avatarPath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"filenetworkmanager.cpp::sendAvatar","Failed open avatar");
    }

    QByteArray fileData = file.readAll();
    file.close();

    QJsonObject fileDataJson;
    fileDataJson["flag"] = "newAvatarData";
    fileDataJson["type"] = type;
    fileDataJson["id"] = id;
    fileDataJson["fileName"] = fileInfo.baseName();
    fileDataJson["fileExtension"] = fileInfo.suffix();
    fileDataJson["fileData"] = QString(fileData.toBase64());

    QJsonDocument doc(fileDataJson);
    sendToFileServer(doc);
}

void FileNetworkManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserId = userId;
    activeUserLogin = userName;
}

void FileNetworkManager::setLogger(Logger *logger)
{
    this->logger = logger;
}

void FileNetworkManager::onFileServerReceived()
{
    QDataStream in(fileSocket);
    logger->log(Logger::INFO,"filenetworkmanager.cpp::onFileServerReceived","Data from FileServer received");
    in.setVersion(QDataStream::Qt_6_7);

    static quint32 blockSize = 0;

    if(in.status() != QDataStream::Ok) {
        logger->log(Logger::ERROR, "filenetworkmanager.cpp::onFileServerReceived", "Error reading data from socket: " + QString::number(in.status()));
        return;
    }

    while (true) {
        if (blockSize == 0) {
            if (fileSocket->bytesAvailable() < sizeof(quint32))
                return;
            in >> blockSize;
        }

        if (fileSocket->bytesAvailable() < blockSize)
            return;

        QByteArray jsonData;
        jsonData.resize(blockSize);
        in.readRawData(jsonData.data(), blockSize);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);

        if (doc.isNull()) {
            logger->log(Logger::ERROR,"filenetworkmanager.cpp::onFileServerReceived","Received JSON doc is null");
            blockSize = 0;
            return;
        }

        QJsonObject receivedFromServerJson = doc.object();
        QString flag = receivedFromServerJson["flag"].toString();
        logger->log(Logger::INFO, "filenetworkmanager.cpp::onFileServerReceived", "Readings JSON for " + flag);

        auto it = flagMap.find(flag.toStdString());
        uint flagId = (it != flagMap.end()) ? it->second : 0;

        switch (flagId) {
        case 1: {
            QString fileUrl = receivedFromServerJson["fileUrl"].toString();
            emit sendMessageWithFile(fileUrl, "personal");
            break;
        }
        case 2: {
            QString fileUrl = receivedFromServerJson["fileUrl"].toString();
            emit sendMessageWithFile(fileUrl, "group");
            break;
        }
        case 3:
            emit uploadFiles(receivedFromServerJson);
            break;
        case 4:
            emit uploadAvatar(receivedFromServerJson);
            break;
        case 5: {
            emit sendAvatarUrl(
                receivedFromServerJson["avatar_url"].toString(),
                receivedFromServerJson["id"].toInt(),
                receivedFromServerJson["type"].toString()
                );
            break;
        }
        case 6:
            emit uploadVoiceFile(receivedFromServerJson);
            break;
        default:
            logger->log(Logger::WARN, "filenetworkmanager.cpp::onFileServerReceived", "Unknown file flag received: " + flag);
            break;
        }
        blockSize = 0;
    }
    logger->log(Logger::INFO,"filenetworkmanager.cpp::onFileServerReceived","Leave onDataReceived");
}

void FileNetworkManager::handleFileBytesWritten(qint64 bytes)
{
    QMutexLocker lock(&fileMutex);

    logger->log(Logger::INFO, "filenetworkmanager.cpp::handleFileBytesWritten", "Bytes written: " + QString::number(bytes));

    if (!sendFileQueue.isEmpty()) {
        sendFileQueue.dequeue();
        logger->log(Logger::INFO, "filenetworkmanager.cpp::handleFileBytesWritten", "Message dequeued. Queue size: " + QString::number(sendFileQueue.size()));
        if (!sendFileQueue.isEmpty()) {
            logger->log(Logger::INFO, "filenetworkmanager.cpp::handleFileBytesWritten", "More messages in queue. Scheduling next processing.");
            QTimer::singleShot(10, this, &FileNetworkManager::processSendFileQueue);
        }
    }
}

void FileNetworkManager::processSendFileQueue()
{
    QMutexLocker lock(&fileMutex);
    if (sendFileQueue.isEmpty()) {
        logger->log(Logger::DEBUG, "filenetworkmanager.cpp::processSendFileQueue", "Send file queue is empty. Nothing to process.");
        return;
    }

    QByteArray jsonData = sendFileQueue.head();
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);

    out << quint32(jsonData.size());
    out.writeRawData(jsonData.data(), jsonData.size());
    logger->log(Logger::INFO, "filenetworkmanager.cpp::processSendFileQueue", "Preparing to send data. Data size: " + QString::number(jsonData.size()) + " bytes");

    if (fileSocket->state() != QAbstractSocket::ConnectedState) {
        logger->log(Logger::DEBUG, "filenetworkmanager.cpp::processSendFileQueue", "Socket is not connected. Cannot send data.");
        return;
    }

    if (fileSocket->write(bytes) == -1) {
        logger->log(Logger::DEBUG, "filenetworkmanager.cpp::processSendFileQueue", "Failed to write data: " + fileSocket->errorString());
        return;
    }
    logger->log(Logger::INFO, "filenetworkmanager.cpp::processSendFileQueue", "Data written to socket. Data size: " + QString::number(bytes.size()) + " bytes");
}

const std::unordered_map<std::string_view, uint> FileNetworkManager::flagMap = {
    {"personal_file_url", 1}, {"group_file_url", 2},
    {"fileData", 3}, {"avatarData", 4},
    {"avatarUrl", 5}, {"voiceFileData", 6}
};
