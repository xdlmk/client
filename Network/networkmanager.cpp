#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket(this);
    fileSocket = new QTcpSocket(this);

    activeUserId = 0;
    activeUserLogin = "";

    QObject::connect(socket, &QTcpSocket::connected, [this]() {
        logger->log(Logger::INFO,"networkmanager.cpp::constructor","Connection to the MessageServer established");
        emit connectionSuccess();

        if(activeUserId != 0) {
            QJsonObject setIdentifiers;
            setIdentifiers["flag"] = "identifiers";
            setIdentifiers["userlogin"] = activeUserLogin;
            setIdentifiers["user_id"] = activeUserId;
            sendData(setIdentifiers);
        }
        {
            QMutexLocker lock(&messageMutex);
            if (!sendMessageQueue.isEmpty()) {
                QMetaObject::invokeMethod(this, "processSendMessageQueue", Qt::QueuedConnection);
            }
        }
    });
    connect(socket,&QTcpSocket::readyRead,this,&NetworkManager::onDataReceived);
    connect(socket,&QTcpSocket::disconnected,this,&NetworkManager::onDisconnected);
    connect(socket,&QTcpSocket::bytesWritten,this,&NetworkManager::handleMessageBytesWritten);

    QObject::connect(fileSocket, &QTcpSocket::connected, [this]() {
        logger->log(Logger::INFO,"networkmanager.cpp::constructor","Connection to the FileServer established");
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
    connect(fileSocket,&QTcpSocket::readyRead,this,&NetworkManager::onFileServerReceived);
    connect(fileSocket,&QTcpSocket::disconnected,this,&NetworkManager::onDisconnected);
    connect(fileSocket,&QTcpSocket::bytesWritten,this,&NetworkManager::handleFileBytesWritten);

    connect(&reconnectTimer, &QTimer::timeout, this, &NetworkManager::attemptReconnect);

    QObject::connect(socket, &QAbstractSocket::errorOccurred, [this](QAbstractSocket::SocketError socketError) {
        if (!reconnectTimer.isActive()) {
            reconnectTimer.start(2000);
        }
        logger->log(Logger::ERROR,"networkmanager.cpp::constructor","Connection to the MessageServer error: " + socket->errorString());
    });
    QObject::connect(fileSocket, &QAbstractSocket::errorOccurred, [this](QAbstractSocket::SocketError socketError) {
        if (!reconnectTimer.isActive()) {
            reconnectTimer.start(2000);
        }
        logger->log(Logger::ERROR,"networkmanager.cpp::constructor","Connection to the FileServer error: " + fileSocket->errorString());
    });
    connectToServer();
}

void NetworkManager::connectToServer()
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
    socket->connectToHost(ip,2020);
}

void NetworkManager::connectToFileServer()
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

void NetworkManager::sendData(const QJsonObject &jsonToSend)
{
    QJsonDocument doc(jsonToSend);
    logger->log(Logger::INFO,"networkmanager.cpp::sendData","Sending json for " + jsonToSend["flag"].toString());
    QByteArray jsonDataOut = doc.toJson(QJsonDocument::Compact);

    bool shouldStartProcessing = false;
    {
        QMutexLocker lock(&messageMutex);
        if (sendMessageQueue.size() >= MAX_QUEUE_SIZE) {
            logger->log(Logger::DEBUG, "networkmanager.cpp::sendData", "Send queue overflow! Dropping message.");
            return;
        }
        sendMessageQueue.enqueue(jsonDataOut);
        logger->log(Logger::INFO, "networkmanager.cpp::sendData", "Message added to queue. Queue size: " + QString::number(sendMessageQueue.size()));
        shouldStartProcessing = sendMessageQueue.size() == 1;
    }

    if (shouldStartProcessing) {
        logger->log(Logger::INFO, "networkmanager.cpp::sendData", "Starting to process the send queue.");
        processSendMessageQueue();
    }
}

void NetworkManager::sendToFileServer(const QJsonDocument &doc)
{
    logger->log(Logger::INFO,"networkmanager.cpp::sendToFileServer","sendToFileServer starts");
    QByteArray fileDataOut = doc.toJson(QJsonDocument::Compact);

    bool shouldStartProcessing = false;
    {
        QMutexLocker lock(&fileMutex);
        if (sendFileQueue.size() >= MAX_QUEUE_SIZE) {
            logger->log(Logger::DEBUG, "networkmanager.cpp::sendToFileServer", "Send queue overflow! Dropping message.");
            return;
        }
        sendFileQueue.enqueue(fileDataOut);
        logger->log(Logger::INFO, "networkmanager.cpp::sendToFileServer", "Message added to queue. Queue size: " + QString::number(sendFileQueue.size()));
        shouldStartProcessing = sendFileQueue.size() == 1;
    }

    if (shouldStartProcessing) {
        logger->log(Logger::INFO, "networkmanager.cpp::sendToFileServer", "Starting to process the send queue.");
        processSendFileQueue();
    }
}

void NetworkManager::sendFile(const QString &filePath,const QString &flag)
{
    logger->log(Logger::INFO,"networkmanager.cpp::sendFile","Sending file");
    logger->log(Logger::INFO,"networkmanager.cpp::sendFile","filePath = " + filePath);
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"networkmanager.cpp::sendFile","Failed open file");
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

void NetworkManager::sendAvatar(const QString &avatarPath, const QString &type, const int& id)
{
    logger->log(Logger::INFO,"networkmanager.cpp::sendAvatar","Sending avatar");
    QFile file(avatarPath);
    QFileInfo fileInfo(avatarPath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"networkmanager.cpp::sendAvatar","Failed open avatar");
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

void NetworkManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserId = userId;
    activeUserLogin = userName;
}

void NetworkManager::setLogger(Logger *logger)
{
    this->logger = logger;
}

void NetworkManager::onDisconnected()
{
    if (!reconnectTimer.isActive()) {
        logger->log(Logger::INFO,"networkmanager.cpp::onDisconnected","reconnectTimer starting");
        reconnectTimer.start(2000);
    }
}

void NetworkManager::attemptReconnect()
{
    emit connectionError();
    if(socket->state() == QAbstractSocket::UnconnectedState || (activeUserId != 0 ? fileSocket->state() == QAbstractSocket::UnconnectedState : true))
    {
        logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","Trying to connect to the server");
        if(activeUserId != 0){
            logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","activeUserId != 0");
            if(socket->state() == QAbstractSocket::UnconnectedState) {
                logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","socket UnconnectedState");
                socket->abort();
                connectToServer();
            }
            if(fileSocket->state() == QAbstractSocket::UnconnectedState) {
                logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","fileSocket UnconnectedState");
                fileSocket->abort();
                connectToFileServer();
            }
        } else {
            logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","activeUserId == 0");
            socket->abort();
            connectToServer();
        }
    } else if(socket->state() == QAbstractSocket::ConnectedState && (activeUserId != 0 ? fileSocket->state() == QAbstractSocket::ConnectedState : true)) {
        logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","Connected");
        emit connectionSuccess();
        reconnectTimer.stop();
    }
}

void NetworkManager::onDataReceived()
{
    QDataStream in(socket);
    logger->log(Logger::INFO,"networkmanager.cpp::onDataReceived","Data packets received");
    in.setVersion(QDataStream::Qt_6_7);

    static quint32 blockSize = 0;

    if(in.status() != QDataStream::Ok)
    {
        logger->log(Logger::WARN,"networkmanager.cpp::onDataReceived","QDataStream status error");
        return;
    }

    while (true) {
        if (blockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint32))
                return;
            in >> blockSize;
        }

        if (socket->bytesAvailable() < blockSize) return;

        QByteArray jsonData;
        jsonData.resize(blockSize);
        in.readRawData(jsonData.data(), blockSize);

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);

        if (doc.isNull()) {
            logger->log(Logger::ERROR,"networkmanager.cpp::onDataReceived","Received JSON doc is null");
            blockSize = 0;
            return;
        }

        QJsonObject receivedFromServerJson = doc.object();

        QString flag = receivedFromServerJson["flag"].toString();
        logger->log(Logger::INFO,"networkmanager.cpp::onDataReceived","Readings JSON for " + flag);

        if(flag == "login") emit loginResultsReceived(receivedFromServerJson);
        else if(flag == "reg") emit registrationResultsReceived(receivedFromServerJson);
        else if(flag == "personal_message") emit messageReceived(receivedFromServerJson);
        else if(flag == "group_message") emit groupMessageReceived(receivedFromServerJson);
        else if(flag == "delete_member") emit deleteGroupMemberReceived(receivedFromServerJson);
        else if(flag == "add_group_members") emit addGroupMemberReceived(receivedFromServerJson);
        else if(flag == "chats_info") {
            if(receivedFromServerJson.contains("dialogs_info") && receivedFromServerJson.contains("groups_info")){
                emit dialogsInfoReceived(receivedFromServerJson["dialogs_info"].toObject());
                emit groupInfoReceived(receivedFromServerJson["groups_info"].toObject());
            } else {
                emit removeAccountFromConfigManager();
                QCoreApplication::quit();
            }
        }
        else if(flag == "search")  emit searchDataReceived(receivedFromServerJson);
        else if(flag == "updating_chats") emit chatsUpdateDataReceived(receivedFromServerJson);
        else if(flag == "load_messages") emit loadMeassgesReceived(receivedFromServerJson);
        else if(flag == "edit") emit editResultsReceived(receivedFromServerJson);
        else if(flag == "avatars_update") emit avatarsUpdateReceived(receivedFromServerJson);
        else if(flag == "avatarUrl") emit sendAvatarUrl(receivedFromServerJson["avatar_url"].toString(),receivedFromServerJson["id"].toInt(),receivedFromServerJson["type"].toString());

        blockSize = 0;
    }
    logger->log(Logger::INFO,"networkmanager.cpp::onDataReceived","Leave onDataReceived");

}

void NetworkManager::onFileServerReceived()
{
    QDataStream in(fileSocket);
    logger->log(Logger::INFO,"networkmanager.cpp::onFileServerReceived","Data from FileServer received");
    in.setVersion(QDataStream::Qt_6_7);

    static quint32 blockSize = 0;

    if(in.status() == QDataStream::Ok) {
        logger->log(Logger::ERROR, "networkmanager.cpp::onFileServerReceived", "Error reading data from socket: " + QString::number(in.status()));
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
            logger->log(Logger::ERROR,"networkmanager.cpp::onFileServerReceived","Received JSON doc is null");
            blockSize = 0;
            return;
        }

        QJsonObject receivedFromServerJson = doc.object();
        if(receivedFromServerJson["flag"].toString() == "personal_file_url") {
            QString fileUrl = receivedFromServerJson["fileUrl"].toString();
            emit sendMessageWithFile(fileUrl,"personal");
        } else if(receivedFromServerJson["flag"].toString() == "group_file_url") {
            QString fileUrl = receivedFromServerJson["fileUrl"].toString();
            emit sendMessageWithFile(fileUrl,"group");
        } else if (receivedFromServerJson["flag"].toString() == "fileData") {
            emit uploadFiles(receivedFromServerJson);
        } else if (receivedFromServerJson["flag"].toString() == "avatarData") {
            emit uploadAvatar(receivedFromServerJson);
        } else if (receivedFromServerJson["flag"].toString() == "avatarUrl") {
            emit sendAvatarUrl(receivedFromServerJson["avatar_url"].toString(),receivedFromServerJson["id"].toInt(),receivedFromServerJson["type"].toString());
        } else if (receivedFromServerJson["flag"].toString() == "voiceFileData") {
            emit uploadVoiceFile(receivedFromServerJson);
        }

        blockSize = 0;
    }
}

void NetworkManager::handleMessageBytesWritten(qint64 bytes)
{
    QMutexLocker lock(&messageMutex);

    logger->log(Logger::INFO, "networkmanager.cpp::handleMessageBytesWritten", "Bytes written: " + QString::number(bytes));

    if (!sendMessageQueue.isEmpty()) {
        sendMessageQueue.dequeue();
        logger->log(Logger::INFO, "networkmanager.cpp::handleMessageBytesWritten", "Message dequeued. Queue size: " + QString::number(sendMessageQueue.size()));
        if (!sendMessageQueue.isEmpty()) {
            logger->log(Logger::INFO, "networkmanager.cpp::handleMessageBytesWritten", "More messages in queue. Scheduling next processing.");
            QTimer::singleShot(10, this, &NetworkManager::processSendMessageQueue);
        }
    }
}

void NetworkManager::handleFileBytesWritten(qint64 bytes)
{
    QMutexLocker lock(&fileMutex);

    logger->log(Logger::INFO, "networkmanager.cpp::handleFileBytesWritten", "Bytes written: " + QString::number(bytes));

    if (!sendFileQueue.isEmpty()) {
        sendFileQueue.dequeue();
        logger->log(Logger::INFO, "networkmanager.cpp::handleFileBytesWritten", "Message dequeued. Queue size: " + QString::number(sendFileQueue.size()));
        if (!sendFileQueue.isEmpty()) {
            logger->log(Logger::INFO, "networkmanager.cpp::handleFileBytesWritten", "More messages in queue. Scheduling next processing.");
            QTimer::singleShot(10, this, &NetworkManager::processSendFileQueue);
        }
    }
}

void NetworkManager::processSendFileQueue()
{
    QMutexLocker lock(&fileMutex);
    if (sendFileQueue.isEmpty()) {
        logger->log(Logger::DEBUG, "networkmanager.cpp::processSendFileQueue", "Send file queue is empty. Nothing to process.");
        return;
    }

    QByteArray jsonData = sendFileQueue.head();
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);

    out << quint32(jsonData.size());
    out.writeRawData(jsonData.data(), jsonData.size());
    logger->log(Logger::INFO, "networkmanager.cpp::processSendFileQueue", "Preparing to send data. Data size: " + QString::number(jsonData.size()) + " bytes");

    if (fileSocket->state() != QAbstractSocket::ConnectedState) {
        logger->log(Logger::DEBUG, "networkmanager.cpp::processSendFileQueue", "Socket is not connected. Cannot send data.");
        return;
    }

    if (fileSocket->write(bytes) == -1) {
        logger->log(Logger::DEBUG, "networkmanager.cpp::processSendFileQueue", "Failed to write data: " + fileSocket->errorString());
        return;
    }
    logger->log(Logger::INFO, "networkmanager.cpp::processSendFileQueue", "Data written to socket. Data size: " + QString::number(bytes.size()) + " bytes");
}

void NetworkManager::processSendMessageQueue()
{
    QMutexLocker lock(&messageMutex);
    if (sendMessageQueue.isEmpty()) {
        logger->log(Logger::DEBUG, "networkmanager.cpp::processSendMessageQueue", "Send queue is empty. Nothing to process.");
        return;
    }

    QByteArray jsonData = sendMessageQueue.head();
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);

    out << quint32(jsonData.size());
    out.writeRawData(jsonData.data(), jsonData.size());
    logger->log(Logger::INFO, "networkmanager.cpp::processSendMessageQueue", "Preparing to send data. Data size: " + QString::number(jsonData.size()) + " bytes");

    if (socket->state() != QAbstractSocket::ConnectedState) {
        logger->log(Logger::DEBUG, "networkmanager.cpp::processSendMessageQueue", "Socket is not connected. Cannot send data.");
        return;
    }
    if (socket->write(bytes) == -1) {
        logger->log(Logger::DEBUG, "networkmanager.cpp::processSendMessageQueue", "Failed to write data: " + socket->errorString());
        return;
    }
    logger->log(Logger::INFO, "networkmanager.cpp::processSendMessageQueue", "Data written to socket. Data size: " + QString::number(bytes.size()) + " bytes");
}
