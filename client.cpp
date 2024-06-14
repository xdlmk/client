#include "client.h"

Client::Client(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket(this);
    QObject::connect(socket, &QTcpSocket::connected, [&]() {
        mesFrom = "Success connect to server 37.45.6.169 on port 2020";
        emit newInMessage();
    });
    socket->connectToHost("37.45.6.169",2020);

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

void Client::login(QString login, QString password)
{
    if(login == "admin" and password == "admin")
    {
        emit loginSuccess();
    }
    else
    {
        emit loginFail();
    }
}

void Client::sendToServer(QString str)
{
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << str;
    socket->write(data);
}

void Client::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_7);
    if(in.status() == QDataStream::Ok)
    {
        QString str;
        in >> str;
        mesFrom = str;
    }
    else
    {
        qDebug() << "Error DataStream status (client.cpp::26)";
    }
    emit newInMessage();
}
