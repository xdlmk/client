#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket(this);
    QObject::connect(socket, &QTcpSocket::connected, [&]() {
        qInfo() <<  "Success connect to server on port 2020";
        reconnectTimer.stop();
        emit connectionSuccess();
    });
    QObject::connect(socket, &QAbstractSocket::errorOccurred, [&](QAbstractSocket::SocketError socketError) {
        if (!reconnectTimer.isActive()) {
            reconnectTimer.start(2000);
        }
        qDebug() << "Connection error: " << socket->errorString();
    });
    connectToServer();

    connect(socket,&QTcpSocket::readyRead,this,&NetworkManager::onDataReceived);
    connect(socket,&QTcpSocket::disconnected,this,&NetworkManager::onDisconnected);
    connect(&reconnectTimer, &QTimer::timeout, this, &NetworkManager::attemptReconnect);
}

void NetworkManager::connectToServer()
{
    qDebug() << "connectToServer";
    socket->connectToHost("127.0.0.1",2020);
}

void NetworkManager::sendData(const QJsonObject &jsonToSend)
{
    QJsonDocument doc(jsonToSend);
    qDebug() << "Sending json: "<< doc.toJson(QJsonDocument::Indented);
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

void NetworkManager::onDisconnected()
{
    qDebug() << "Disconnected from server.";
    if (!reconnectTimer.isActive()) {
        reconnectTimer.start(2000);
    }
}

void NetworkManager::attemptReconnect()
{
    emit connectionError();
    qDebug() << socket->state();
    if(socket->state() == QAbstractSocket::UnconnectedState)
    {
        qDebug() << "Attempting to reconnect...";
        socket->abort();
        connectToServer();
    }
}

void NetworkManager::onDataReceived()
{
    QDataStream in(socket);
    qInfo() << "data reads";
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
            qDebug() << "Block size:" << blockSize;
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
            qDebug() << "Error with JSON doc check, doc is null";
            blockSize = 0;
            return;
        }

        QJsonObject jsonToRead = doc.object();
        jsonToRead.remove("profileImage");
        QJsonDocument toReadDoc(jsonToRead);
        qDebug() << "(without profileImage)JSON to read:" << toReadDoc.toJson(QJsonDocument::Indented);

        QJsonObject receivedFromServerJson = doc.object();
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
        //emit dataReceived(doc);

        blockSize = 0;
        qDebug() << "Leave read message from server";
    }
    else
    {
        qDebug() << "[" << QDateTime::currentDateTime().toString() << "] " << "Error DataStream status (networkmanager.cpp::onDataReceived)";
    }
}
