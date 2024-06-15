#include "client.h"

Client::Client(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket(this);
     qInfo() <<  "Start connected";
    QObject::connect(socket, &QTcpSocket::connected, [&]() {
        qInfo() <<  "Success connect to server 37.45.6.169 on port 2020";
    });
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
        qInfo() << "Failed to connect to server:";
    });
     qInfo() <<  "end connected";
    connectToServer("192.168.100.21",2020);

    connect(socket,&QTcpSocket::readyRead,this,&Client::slotReadyRead);
    connect(socket,&QTcpSocket::disconnected,socket,&QTcpSocket::deleteLater);
}

QString Client::messageFrom()
{
    return mesFrom;
}

void Client::setMessageFrom(QString value)
{
    mesFrom = value;
}

void Client::connectToServer(QString host, quint16 port)
{
    socket->connectToHost(host,port);
}

void Client::login(QString login, QString password)
{
    QJsonObject json;
    json["flag"] = "login";
    json["login"] = login;
    json["password"] = password;
    QJsonDocument doc(json);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
}

void Client::sendToServer(QString str)
{
    QJsonObject json;
    json["flag"] = "message";
    json["str"] = str;
    QJsonDocument doc(json);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
}

void Client::slotReadyRead()
{
    QDataStream in(socket);
    qInfo() << "data reads";
    in.setVersion(QDataStream::Qt_6_7);
    if(in.status() == QDataStream::Ok)
    {
        qInfo() << "status data stream ok";
        QByteArray str;
        in >> str;
        QJsonDocument doc = QJsonDocument::fromJson(str);
        qDebug() << "JSON to read:" << doc.toJson(QJsonDocument::Indented); // Вывод JSON с отступами для читаемости
        QJsonObject json = doc.object();
        QString flag = json["flag"].toString();
        qInfo() << "next check flags";
        if(flag == "message")
        {
            qInfo() << "flag message";
            QString str1 = json["str"].toString();
            mesFrom = str1;
            emit newInMessage();
        }
        else if(flag == "login")
        {
            qInfo() << "flag login";
            QString success = json["success"].toString();
            if(success == "ok")
            {
                emit loginSuccess();
            }
            else if(success == "poor")
            {
                emit loginFail();
            }
        }
        qDebug() << "Leave read message from server";
    }
    else
    {
        qDebug() << "Error DataStream status (client.cpp::26)";
    }

}
