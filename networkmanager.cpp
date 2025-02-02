#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket(this);
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
    socket->connectToHost("127.0.0.1",2020);
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
        logger->log(Logger::INFO,"networkmanager.cpp::onDataReceived","Readings JSON for " + receivedFromServerJson["flag"].toString());

        QString flag = receivedFromServerJson["flag"].toString();

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
        else if(flag == "search")
        {
            emit searchDataReceived(receivedFromServerJson);
        }
        else if(flag == "updating_chats")
        {
            emit chatsUpdateDataReceived(receivedFromServerJson);
        }

        blockSize = 0;
        logger->log(Logger::INFO,"networkmanager.cpp::onDataReceived","Leave onDataReceived");
    }
    else
    {
        logger->log(Logger::WARN,"networkmanager.cpp::onDataReceived","QDataStream status error");
    }
}
