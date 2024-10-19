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

void Client::showMessageList(QString login)
{
    qDebug() << "showMessageList";
    QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal" +"/message_" + login + ".json");
    if (!file.open(QIODevice::ReadWrite)) {
        qDebug() << "File not open client.cpp::showMessageList::41";
        return;
    }

    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        QJsonArray chatHistory = QJsonDocument::fromJson(fileData).array();

        if (!chatHistory.isEmpty()) {
            QJsonObject lastMessageObject = chatHistory.last().toObject();

            QString message = lastMessageObject["str"].toString();
            int id = lastMessageObject["id"].toInt();
            QString out = lastMessageObject["Out"].toString();

            showPersonalChat(login,message,id,out);
        } else {
            qDebug() << "chatHistory is Empty";
        }
    }
    file.close();
}

void Client::saveMessageFromDatabase(QJsonObject json)
{
    QJsonArray messagesArray = json["messages"].toArray();
    if(messagesArray.isEmpty()){
        return;
    }

    for (const QJsonValue &value : messagesArray) {
        QJsonObject json = value.toObject();

        QString message = json["str"].toString();
        QString time = json["time"].toString();
        QString fulldate = json["FullDate"].toString();
        int dialog_id = json["dialog_id"].toInt();
        int message_id = json["message_id"].toInt();
        QString sender_login = json["sender_login"].toString();
        int sender_id = json["sender_id"].toInt();
        qDebug() << message << time << dialog_id << message_id << sender_login;

        if(sender_login == activeUserName) {
            QString receiver_login = json["receiver_login"].toString();
            int receiver_id = json["receiver_id"].toInt();
            saveMessageToJson(receiver_login, message, "out", time, fulldate, message_id, dialog_id,receiver_id);
        }
        else{
            saveMessageToJson(sender_login, message, "", time, fulldate, message_id, dialog_id,sender_id);
        }
    }
}

void Client::updatingChats()
{
    QString folderPath = QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal";

    QDir dir(folderPath);

    QStringList fileList = dir.entryList(QStringList() << "message_*.json", QDir::Files);

    QJsonArray jsonArray;

    QRegularExpression regex("^message_(.*)\\.json$");


    for (const QString& fileName : fileList) {
        QRegularExpressionMatch match = regex.match(fileName);
        if (match.hasMatch()) {
            QString login = match.captured(1);
            if(login == "") continue;

            showMessageList(login);
            QJsonObject loginObject;

            //Parser for message ID

            QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal" +"/message_" + login + ".json");
            if (!file.open(QIODevice::ReadWrite)) {
                qDebug() << "File not open client.cpp::updatingChats::91";
                return;
            }

            QByteArray fileData = file.readAll();
            if (!fileData.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(fileData);
                QJsonArray chatHistory = doc.array();

                if (!chatHistory.isEmpty()) {

                    QJsonObject lastMessageObject = chatHistory.last().toObject();
                    loginObject["message_id"] = lastMessageObject["message_id"];
                    loginObject["login"] = login;
                    loginObject["dialog_id"] = lastMessageObject["dialog_id"];

                } else {
                    qDebug() << "chatHistory is Empty.";
                }
            }
            file.close();
            //end of parser for message ID

            if (!loginObject.isEmpty()) {
                jsonArray.append(loginObject);
            }
            else {
                qDebug() << "loginObject is empty";
            }
        }
    }

    QJsonObject mainObject;
    mainObject["flag"] = "updating_chats";
    if(!fileList.isEmpty()) {
        mainObject["chats"] = jsonArray;
    }
    else {
        mainObject["userlogin"] = activeUserName;
    }

    QJsonDocument doc(mainObject);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
}

void Client::loadMessageToQml(const QString username, const QString message, const QString out, const QString time)
{
    if(out == "out")
    {
        emit newOutMessage(username,message,time);
    }
    else
    {
        emit newInMessage(username,message,time);
    }
}

void Client::loadMessageFromJson(const QString &filepath)
{
    QFile file(filepath);

    if (!file.open(QIODevice::ReadWrite)) {
        qDebug() << "File not open client.cpp::loadMessageFromJson::41";
        return;
    }
    emit clearMainListView();
    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        QJsonArray chatHistory = doc.array();

        for (const QJsonValue &value : chatHistory) {
            QJsonObject messageObject = value.toObject();
            QString user = messageObject["login"].toString();
            QString message = messageObject["str"].toString();
            QString out = messageObject["Out"].toString();
            QString time = messageObject["time"].toString();

            qDebug() << time <<" "<< user << ": " << message;
            loadMessageToQml(user,message,out,time);
        }
    }
    file.close();

}

void Client::saveMessageToJson(const QString userlogin, const QString message, const QString out, const QString time,const QString FullDate, const int message_id,const int dialog_id, const int id)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal" +"/message_" + userlogin + ".json");

    if (!file.exists()) {
        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray emptyArray;
            QJsonDocument doc(emptyArray);
            file.write(doc.toJson());
            file.close();
        } else {
            qDebug() << "File dont create " + userlogin ;
        }
    } else {
        qDebug() << "File exist " + userlogin;
    }

    if (!file.open(QIODevice::ReadWrite)) {
        qDebug() << "File not open client.cpp::saveMessageToJson::99";
        return;
    }

    QJsonArray chatHistory;
    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        chatHistory = doc.array();
    }

    QJsonObject messageObject;
    if(out == "out"){
        messageObject["login"] = activeUserName;
    }
    else {
        messageObject["login"] = userlogin;
    }
    messageObject["id"] = id;
    messageObject["message_id"] = message_id;
    messageObject["dialog_id"] = dialog_id;
    messageObject["str"] = message;
    messageObject["Out"] = out;
    messageObject["FullDate"] = FullDate;
    messageObject["time"] = time;

    chatHistory.append(messageObject);

    file.resize(0);
    QJsonDocument updatedDoc(chatHistory);
    file.write(updatedDoc.toJson());
    file.close();

    emit showPersonalChat(userlogin, message, id, out);
}

void Client::connectToServer()
{
    qDebug() << "connectToServer";
    socket->connectToHost("127.0.0.1",2020);
}

void Client::createConfigFile(QString userLogin,QString userPassword)
{
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";
    QSettings settings(configFilePath,QSettings::IniFormat);
    int total = settings.value("total",0).toInt();
    bool already_exists = false;
    for( int i = 1; i<=total; i++ )
    {
        if (userLogin == settings.value("login"+ QString::number(i)))
        {
            already_exists = true;
            break;
        }
    }
    if (!already_exists)
    {
        if(settings.value("total",0).toInt() == 0)
        {
            settings.setValue("total", 1);
            settings.setValue("active_account",1);
            settings.setValue("success1","ok");
            settings.setValue("login1", userLogin);
            settings.setValue("password1", userPassword);
            qDebug()<<"Config file created! 1 user";
        }
        else if(settings.value("total",0).toInt() == 1)
        {
            settings.setValue("total", 2);
            settings.setValue("active_account",2);
            settings.setValue("success2","ok");
            settings.setValue("login2", userLogin);
            settings.setValue("password2", userPassword);
            qDebug()<<"Config file created! 2 users";
        }
        else if(settings.value("total",0).toInt() == 2)
        {
            settings.setValue("total", 3);
            settings.setValue("active_account",3);
            settings.setValue("success3","ok");
            settings.setValue("login3", userLogin);
            settings.setValue("password3", userPassword);
            qDebug()<<"Config file created! 3 users";
        }
    }
    else qDebug() << "Already exists: " << already_exists;
    total = settings.value("total",0).toInt();
    for( int i = 1; i<=total;i++)
    {
        emit newUser(settings.value("login"+QString::number(i), "").toString());
        qDebug()<< settings.value("login"+QString::number(i), "").toString();
    }
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

void Client::readPersonalJson(const QString userlogin)
{
    if(activeUserName == userlogin){
        return;
    }
    QDir dir(QCoreApplication::applicationDirPath() + "/resources/"+ activeUserName + "/personal");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal" +"/message_" + userlogin + ".json");

    if (!file.exists()) {
        qDebug() << "File not exist, creating new file";

        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray emptyArray;
            QJsonDocument doc(emptyArray);
            file.write(doc.toJson());
            file.close();
        } else {
            qDebug() << "File dont create";
        }
    }

    if (!file.open(QIODevice::ReadWrite)) {
        qDebug() << "File not open client.cpp::readPersonalJson::211";
        return;
    }

    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        QJsonArray chatHistory = doc.array();

        emit clearMainListView();
        for (const QJsonValue &value : chatHistory) {
            QJsonObject messageObject = value.toObject();
            QString user = messageObject["login"].toString();
            QString message = messageObject["str"].toString();
            QString out = messageObject["Out"].toString();
            QString time = messageObject["time"].toString();

            qDebug() << time <<" "<< user << ": " << message;
            loadMessageToQml(user,message,out,time);
        }
    }
    file.close();

}

void Client::sendPersonalMessage(const QString &message, const QString &receiver_login, const int &receiver_id)
{
    QJsonObject json;

    json["flag"] = "personal_message";
    json["message"] = message;

    json["sender_login"] = activeUserName;
    json["sender_id"] = user_id;

    json["receiver_login"] = receiver_login;
    json["receiver_id"] = receiver_id;

    qDebug() << "JSON to send:" << json;

    QJsonDocument doc(json);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
}

void Client::checkMessage(const QString &userlogin)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/resources/"+ activeUserName + "/personal");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString pathtomessages = QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal" +"/message_" + userlogin + ".json";
    QFile file(pathtomessages);
    if (!file.exists()) {
        qDebug() << "File not exist, creating new file";

        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray emptyArray;
            QJsonDocument doc(emptyArray);
            file.write(doc.toJson());
            file.close();
        } else {
            qDebug() << "File dont create";
        }
    } else {
        qDebug() << "File exist";
        loadMessageFromJson(pathtomessages);
    }
}

void Client::changeActiveAccount(const QString username)
{
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";
    QSettings configFile(configFilePath, QSettings::IniFormat);

    int total = configFile.value("total", 0).toInt();

    for (int i = 1; i <= total; ++i) {
        QString currentLogin = configFile.value("login"+ QString::number(i), "").toString();

        if (currentLogin == username) {
            configFile.setValue("active_account", i);
            qDebug() << "Active account changed to:" << i;
            QString password = configFile.value("password"+ QString::number(i),"").toString();
            emit changeAccount(username,password);

            return;
        }
    }

    qDebug() << "User not found:" << username;
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

void Client::sendSearchToServer(const QString &searchable)
{
    QJsonObject json;
    json["flag"] = "search";
    json["searchable"] = searchable;
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
    socket->flush();
    qDebug() << "Emitting clientLogout";
    emit clientLogout();
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";
    QSettings configFile(configFilePath, QSettings::IniFormat);
    int active_account = configFile.value("active_account",0).toInt();
    configFile.remove("success"+QString::number(active_account));
    configFile.remove("login"+QString::number(active_account));
    configFile.remove("password"+QString::number(active_account));

    for (int i = active_account + 1; i <= configFile.value("total",0).toInt(); ++i) {
        QString success = configFile.value("success" + QString::number(i), "").toString();
        QString login = configFile.value("login" + QString::number(i), "").toString();
        QString password = configFile.value("password" + QString::number(i), "").toString();

        configFile.setValue("success" + QString::number(i - 1), success);
        configFile.setValue("login" + QString::number(i - 1), login);
        configFile.setValue("password" + QString::number(i - 1), password);

        configFile.remove("success" + QString::number(i));
        configFile.remove("login" + QString::number(i));
        configFile.remove("password" + QString::number(i));
    }



    if(configFile.value("total",0).toInt()>=2)
    {
        configFile.setValue("total",configFile.value("total",0).toInt()-1);
        configFile.setValue("active_account",1);
        active_account = configFile.value("active_account",0).toInt();

        QString success = configFile.value("success"+QString::number(active_account),"").toString();
        qDebug() << success;
        if(success == "ok")
        {
            QString login = configFile.value("login"+QString::number(active_account), "").toString();
            QString password = configFile.value("password"+QString::number(active_account), "").toString();

            this->login(login,password);
        }
    }
    else
    {
        QFile confFile(configFilePath);
        if (confFile.exists()) {
            if (confFile.remove()) {
                qDebug() << "Config file removed successfully";
            } else {
                qDebug() << "Failed to remove config file";
            }
        } else {
            qDebug() << "Config file does not exist";
        }
    }
}

void Client::clientChangeAccount()
{
    QJsonObject json;
    json["flag"]= "logout";
    QJsonDocument doc(json);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
    socket->flush();
}

void Client::slotReadyRead()
{
    QDataStream in(socket);
    qInfo() << "data reads";
    in.setVersion(QDataStream::Qt_6_7);

    static quint32 blockSize = 0;

    if(in.status() == QDataStream::Ok)
    {
        qInfo() << "status data stream ok";

        if (blockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint32))
            {
                qDebug() << "Ожидание данных для размера блока...";
                return;
            }
            in >> blockSize;
            qDebug() << "Размер блока данных:" << blockSize;
        }

        if (socket->bytesAvailable() < blockSize)
        {
            qDebug() << "Ожидание оставшихся данных...";
            return;
        }

        //QByteArray jsonData = socket->readAll();

        QByteArray jsonData;
        jsonData.resize(blockSize);
        in.readRawData(jsonData.data(), blockSize);

        //qDebug() << "Принятые данные в байтах:" << jsonData;

        //in >> jsonData;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);

        if (doc.isNull()) {
            qDebug() << "Ошибка при разборе JSON, получен пустой JSON.";
            blockSize = 0;
            return;
        }

        QJsonObject jsonToRead = doc.object();
        jsonToRead.remove("profileImage");
        QJsonDocument toReadDoc(jsonToRead);
        qDebug() << "(without profileImage)JSON to read:" << toReadDoc.toJson(QJsonDocument::Indented);

        QJsonObject json = doc.object();
        QString flag = json["flag"].toString();
        qInfo() << "next check flags";

        if(flag =="message")
        {
            qInfo() << "flag message";
            QString str1 = json["str"].toString();
            QString userLogin = json["login"].toString();
            QString out = json["Out"].toString();
            QString time = QDateTime::currentDateTime().toString("HH:mm");
            //saveMessageToJson(userLogin,str1,out);

            if(out == "out")
            {
                emit newOutMessage(userLogin,str1,time);
            }
            else
            {
                emit newInMessage(userLogin,str1,time);
            }
        }
        else if(flag == "login")
        {
            qInfo() << "flag login";
            QString success = json["success"].toString();
            QString name = json["name"].toString();
            QString password = json["password"].toString();
            user_id = json["user_id"].toInt();

            QString imageString = json["profileImage"].toString();
            QByteArray imageData = QByteArray::fromBase64(imageString.toLatin1());

            if(success == "ok")
            {
                QImage image;
                image.loadFromData(imageData);
                QString filePath(QCoreApplication::applicationDirPath() + "/resources/images/avatar.png");
                qDebug() <<"File path: " <<filePath;
                if (!image.save(filePath)) {
                    qDebug() << "Ошибка: не удалось сохранить изображение";
                }

                QDir dir(QCoreApplication::applicationDirPath() + "/resources/messages");
                if (!dir.exists()) {
                    dir.mkpath(".");
                }
                activeUserName = name;

                QString pathtomessages = QCoreApplication::applicationDirPath() + "/resources/messages/message_" + activeUserName + ".json";
                QFile file(pathtomessages);

                if (!file.exists()) {
                    qDebug() << "File not exist, creating new file";

                    if (file.open(QIODevice::WriteOnly)) {
                        QJsonArray emptyArray;
                        QJsonDocument doc(emptyArray);
                        file.write(doc.toJson());
                        file.close();
                    } else {
                        qDebug() << "File dont create";
                    }
                } else {
                    qDebug() << "File exist";
                }


                emit loginSuccess(name);
                loadMessageFromJson(pathtomessages);
                createConfigFile(name,password);
                updatingChats();
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
        else if (flag == "search")
        {
            QJsonArray resultsArray = json.value("results").toArray();
            for (const QJsonValue &value : resultsArray) {
                QJsonObject userObj = value.toObject();
                int id = userObj.value("id").toInt();
                QString userlogin = userObj.value("userlogin").toString();
                emit newSearchUser(userlogin,id);
                qDebug() << "ID:" << id << "Userlogin:" << userlogin;
            }
        }
        else if (flag == "personal_message")
        {
            qDebug() << json["message"].toString();
            qDebug() << json["time"].toString();
            qDebug() << json["message_id"].toInt();
            QString message = json["message"].toString();
            QString time = json["time"].toString();
            int message_id = json["message_id"].toInt();
            int dialog_id = json["dialog_id"].toInt();
            QString login;
            int id;

            if(json.contains("receiver_login"))
            {
                login = json["receiver_login"].toString();
                id = json["receiver_id"].toInt();
                qDebug() << "Message from me to " + login + " id: " << id;
                saveMessageToJson(login, message, "out", time,"not:done(personal_message(activesocket))", message_id,dialog_id,id);
            }
            else
            {
                login = json["sender_login"].toString();
                id = json["sender_id"].toInt();
                qDebug() << "Message from " + login + " id: " << id;
                saveMessageToJson(login, message, "", time, "not:done(personal_message(activesocket))", message_id,dialog_id,id);
            }
            emit checkActiveDialog(login);
        }
        else if( flag == "updating_chats")
        {
            qDebug() << "Flag == updating_chats ";
            saveMessageFromDatabase(json);
        }

        blockSize = 0;
        qDebug() << "Leave read message from server";
    }
    else
    {
        qDebug() << "Error DataStream status (client.cpp::26)";
    }
}
