#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket(this);
    fileSocket = new QTcpSocket(this);
    QObject::connect(socket, &QTcpSocket::connected, [&]() {
        logger->log(Logger::INFO,"networkmanager.cpp::constructor","Connection to the server established");
        reconnectTimer.stop();
        emit connectionSuccess();
    });
    QObject::connect(socket, &QAbstractSocket::errorOccurred, [&](QAbstractSocket::SocketError socketError) {
        if (!reconnectTimer.isActive()) {
            reconnectTimer.start(2000);
        }
        logger->log(Logger::ERROR,"networkmanager.cpp::constructor","Connection error: " + socket->errorString());
    });
    connectToServer();

    connect(socket,&QTcpSocket::readyRead,this,&NetworkManager::onDataReceived);
    connect(socket,&QTcpSocket::disconnected,this,&NetworkManager::onDisconnected);
    connect(&reconnectTimer, &QTimer::timeout, this, &NetworkManager::attemptReconnect);
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

void NetworkManager::sendData(const QJsonObject &jsonToSend)
{
    QJsonDocument doc(jsonToSend);
    logger->log(Logger::INFO,"networkmanager.cpp::sendData","Sending json for " + jsonToSend["flag"].toString());
    QByteArray jsonDataOut = doc.toJson(QJsonDocument::Compact);
    QByteArray data;
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << quint32(jsonDataOut.size());
    out.writeRawData(jsonDataOut.data(),jsonDataOut.size());
    socket->write(data);
    socket->flush();
}

void NetworkManager::sendToFileServer(const QJsonDocument &doc)
{
    logger->log(Logger::INFO,"networkmanager.cpp::sendToFileServer","sendToFileServer starts");
    QByteArray fileDataOut = doc.toJson(QJsonDocument::Compact);
    QByteArray data;
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << quint32(fileDataOut.size());
    out.writeRawData(fileDataOut.data(),fileDataOut.size());

    if(fileSocket->state() == QAbstractSocket::UnconnectedState){
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
        if (!fileSocket->waitForConnected(5000)) {
            logger->log(Logger::WARN,"networkmanager.cpp::sendToFileServer","Failed to connect to fileServer");
        }
    }

    connect(fileSocket,&QTcpSocket::readyRead,this,&NetworkManager::onFileServerReceived);
    fileSocket->write(data);
    fileSocket->flush();
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

void NetworkManager::sendAvatar(const QString &avatarPath)
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
    fileDataJson["user_id"] = activeUserId;
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
    logger->log(Logger::INFO,"networkmanager.cpp::onDisconnected","Disconnecting from the server");
    if (!reconnectTimer.isActive()) {
        reconnectTimer.start(2000);
    }
}

void NetworkManager::attemptReconnect()
{
    emit connectionError();
    if(socket->state() == QAbstractSocket::UnconnectedState)
    {
        logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","Trying to connect to the server");
        socket->abort();
        connectToServer();
    }
}

void NetworkManager::onDataReceived()
{
    QDataStream in(socket);
    logger->log(Logger::INFO,"networkmanager.cpp::onDataReceived","Data packets received");
    in.setVersion(QDataStream::Qt_6_7);

    static quint32 blockSize = 0;

    if(in.status() == QDataStream::Ok)
    {
        if (blockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint32))
            {
                return;
            }
            in >> blockSize;
        }

        if (socket->bytesAvailable() < blockSize)
        {
            return;
        }

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

        if(flag == "login")
        {
            emit loginResultsReceived(receivedFromServerJson);
        }
        else if(flag == "reg")
        {
            emit registrationResultsReceived(receivedFromServerJson);
        }
        else if(flag == "personal_message")
        {
            emit messageReceived(receivedFromServerJson);
        }
        else if(flag == "group_message")
        {
            emit groupMessageReceived(receivedFromServerJson);
        }
        else if(flag == "group_info")
        {
            emit groupInfoReceived(receivedFromServerJson);
        }
        else if(flag == "search")
        {
            emit searchDataReceived(receivedFromServerJson);
        }
        else if(flag == "updating_chats")
        {
            emit chatsUpdateDataReceived(receivedFromServerJson);
        }
        else if(flag == "load_messages")
        {
            emit loadMeassgesReceived(receivedFromServerJson);
        }
        else if(flag == "edit")
        {
            emit editResultsReceived(receivedFromServerJson);
        }
        else if(flag == "avatars_update")
        {
            emit avatarsUpdateReceived(receivedFromServerJson);
        }

        blockSize = 0;
        logger->log(Logger::INFO,"networkmanager.cpp::onDataReceived","Leave onDataReceived");
    }
    else
    {
        logger->log(Logger::WARN,"networkmanager.cpp::onDataReceived","QDataStream status error");
    }
}

void NetworkManager::onFileServerReceived()
{
    QDataStream in(fileSocket);
    logger->log(Logger::INFO,"networkmanager.cpp::onFileServerReceived","Data from FileServer received");
    in.setVersion(QDataStream::Qt_6_7);

    static quint32 blockSize = 0;

    if(in.status() == QDataStream::Ok)
    {
        if (blockSize == 0) {
            if (fileSocket->bytesAvailable() < sizeof(quint32))
            {
                return;
            }
            in >> blockSize;
        }

        if (fileSocket->bytesAvailable() < blockSize)
        {
            return;
        }

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
            emit sendAvatarUrl(receivedFromServerJson["avatar_url"].toString(),receivedFromServerJson["user_id"].toInt());
        } else if (receivedFromServerJson["flag"].toString() == "voiceFileData") {
            emit uploadVoiceFile(receivedFromServerJson);
        }

    } else {
        logger->log(Logger::ERROR, "networkmanager.cpp::onFileServerReceived", "Error reading data from socket: " + QString::number(in.status()));
    }
    blockSize = 0;
}
