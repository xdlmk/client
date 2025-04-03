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
    });
    connect(socket,&QTcpSocket::readyRead,this,&NetworkManager::onDataReceived);
    connect(socket,&QTcpSocket::disconnected,this,&NetworkManager::onDisconnected);

    QObject::connect(fileSocket, &QTcpSocket::connected, [this]() {
        logger->log(Logger::INFO,"networkmanager.cpp::constructor","Connection to the FileServer established");
    });
    connect(fileSocket,&QTcpSocket::readyRead,this,&NetworkManager::onFileServerReceived);
    connect(fileSocket,&QTcpSocket::disconnected,this,&NetworkManager::onDisconnected);

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

void NetworkManager::connectToFileServer(QString &userlogin, int &user_id)
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
    QJsonObject setIdentifiers;
    setIdentifiers["flag"] = "identifiers";
    setIdentifiers["userlogin"] = userlogin;
    setIdentifiers["user_id"] = user_id;
    sendData(setIdentifiers);
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
            socket->abort();
            connectToServer(activeUserLogin,activeUserId);
            if(fileSocket->state() == QAbstractSocket::UnconnectedState) {
                logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","fileSocket UnconnectedState");
                fileSocket->abort();
                connectToFileServer(activeUserLogin,activeUserId);
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
            emit sendAvatarUrl(receivedFromServerJson["avatar_url"].toString(),receivedFromServerJson["id"].toInt(),receivedFromServerJson["type"].toString());
        } else if (receivedFromServerJson["flag"].toString() == "voiceFileData") {
            emit uploadVoiceFile(receivedFromServerJson);
        }

    } else {
        logger->log(Logger::ERROR, "networkmanager.cpp::onFileServerReceived", "Error reading data from socket: " + QString::number(in.status()));
    }
    blockSize = 0;
}

void NetworkManager::connectToServer(const QString &userlogin, const int &user_id)
{
    logger->log(Logger::INFO,"networkmanager.cpp::connectToServer(with ident)","Trying to connect to the MessageServer");
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

    QJsonObject setIdentifiers;
    setIdentifiers["flag"] = "identifiers";
    setIdentifiers["userlogin"] = userlogin;
    setIdentifiers["user_id"] = user_id;
    sendData(setIdentifiers);
}
