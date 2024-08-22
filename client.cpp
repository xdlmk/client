#include "client.h"

Client::Client(QObject *parent)
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
    qDebug() << "Client constructor";
    connectToServer();
    qDebug() << "Client constructor1";
    connect(socket,&QTcpSocket::readyRead,this,&Client::slotReadyRead);
    connect(socket,&QTcpSocket::disconnected,this,&Client::onDisconnected);
    connect(&reconnectTimer, &QTimer::timeout, this, &Client::attemptReconnect);
}

QString Client::messageFrom()
{
    return mesFrom;
}

void Client::setMessageFrom(QString value)
{
    mesFrom = value;
}

void Client::connectToServer()
{
    qDebug() << "connectToServer";
    socket->connectToHost("192.168.100.242",2020);
}

void Client::createConfigFile(QString userLogin,QString userPassword)
{
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";
    QSettings settings(configFilePath,QSettings::IniFormat);
        settings.setValue("success","ok");
        settings.setValue("login", userLogin);
        settings.setValue("password", userPassword);
        qDebug()<<"Config file created!";
}

void Client::onDisconnected()
{
    qDebug() << "Disconnected from server.";
    if (!reconnectTimer.isActive()) {
        reconnectTimer.start(2000);
    }
}

void Client::attemptReconnect()
{
    emit errorWithConnect();
    qDebug() << socket->state();
    if(socket->state() == QAbstractSocket::UnconnectedState)
    {
        qDebug() << "Attempting to reconnect...";
        socket->abort();
        connectToServer();
    }
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

void Client::sendToServer(QString str,QString userLogin)
{
    QJsonObject json;
    json["flag"] = "message";
    json["str"] = str;
    json["login"] = userLogin;
    QJsonDocument doc(json);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
}

void Client::logout()
{
    QJsonObject json;
    json["flag"]= "logout";
    QJsonDocument doc(json);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
    emit clientLogout();
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

        if(flag =="message")
        {
            qInfo() << "flag message";
            QString str1 = json["str"].toString();
            QString userLogin = json["login"].toString();
            mesFrom = str1;
            QString out = json["Out"].toString();
            if(out == "out")
            {
                emit newOutMessage(userLogin);
            }
            else
            {
                emit newInMessage(userLogin);
            }
        }
        else if(flag == "login")
        {
            qInfo() << "flag login";
            QString success = json["success"].toString();
            QString name = json["name"].toString();
            QString password = json["password"].toString();
            if(success == "ok")
            {
                emit loginSuccess(name);
                createConfigFile(name,password);
            }
            else if(success == "poor")
            {
                emit loginFail();
            }
        }
        else if (flag == "reg")
        {
            qDebug() << "flag reg";
            QString success = json["success"].toString();
            if(success == "ok")
            {
                emit regSuccess() ;
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
