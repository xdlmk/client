#include "client.h"

Client::Client(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket(this);
    QObject::connect(socket, &QTcpSocket::connected, [&]() {
        qInfo() <<  "Success connect to server 172.20.10.2 on port 2020";
        emit connectionSuccess();
    });
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), [&](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
        qInfo() << "Failed to connect to server:";
        connectToServer("172.20.10.2",2020);
        emit errorWithConnect();
    });
    connectToServer("172.20.10.2",2020);

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

void Client::reg(QString login, QString password)
{
    qDebug() << "reg function open";
    QJsonObject json;
    json["flag"] = "reg";
    json["login"] = login;
    json["password"] = password;
    QJsonDocument doc(json);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
    qDebug() << "reg function close";
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

void Client::sendToServer(QString str,QString name)
{
    QJsonObject json;
    json["flag"] = "message";
    json["str"] = str;
    json["name"] = name;
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
        qDebug() << "JSON to read:" << doc.toJson(QJsonDocument::Indented);
        QJsonObject json = doc.object();
        QString flag = json["flag"].toString();
        qInfo() << "next check flags";
        if(flag == "message")
        {
            qInfo() << "flag message";
            QString str1 = json["str"].toString();
            QString name = json["name"].toString();
            mesFrom = str1;
            emit newInMessage(name);
        }
        else if(flag == "login")
        {
            qInfo() << "flag login";
            QString success = json["success"].toString();
            QString name = json["name"].toString();
            if(success == "ok")
            {
                emit loginSuccess(name);
            }
            else if(success == "poor")
            {
                emit loginFail();
            }
        }
        else if(flag == "reg")
        {
            QString success = json["success"].toString();
            QString name = json["name"].toString();
            if(success == "ok")
            {
                emit regSuccess(name) ;
            }
            else if (success == "poor")
            {
                QString error = json["errorMes"].toString();
                emit regFail(error);
            }
        }
        qDebug() << "Leave read message from server";
    }
    else
    {
        qDebug() << "Error DataStream status (client.cpp::26)";
    }

}
