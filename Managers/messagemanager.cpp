#include "messagemanager.h"

MessageManager::MessageManager(QObject *parent)
    : QObject{parent}
{}

void MessageManager::loadMessageToQml(const QString &username, const QString &message, const QString &out, const QString &time, const QString &fileUrl)
{
    QString fileName = "";
    if(fileUrl != "") {
        int underscoreIndex = fileUrl.indexOf('_');
        if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
            fileName = fileUrl.mid(underscoreIndex + 1);
        }
    }

    if(out == "out")
    {
        emit newMessage(username, message, time, fileName, fileUrl, true);
    }
    else
    {
        emit newMessage(username, message, time, fileName, fileUrl, false);
    }
}

void MessageManager::saveMessageToJson(QString &userlogin, QString &message, QString &out, QString &time, QString &fullDate, int message_id, int dialog_id, int id, QString &fileUrl)
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
            logger->log(Logger::ERROR,"messagemanager.cpp::saveMessageToJson","File dont create " + userlogin);
        }
    } else {
        logger->log(Logger::INFO,"messagemanager.cpp::saveMessageToJson","File exist " + userlogin);
    }

    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR,"messagemanager.cpp::saveMessageToJson","File did not open with error: " + file.errorString());
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
    messageObject["FullDate"] = fullDate;
    messageObject["time"] = time;
    messageObject["fileUrl"] = fileUrl;

    chatHistory.append(messageObject);

    file.resize(0);
    QJsonDocument updatedDoc(chatHistory);
    file.write(updatedDoc.toJson());
    file.close();

    emit showPersonalChat(userlogin, message, id, out);
}

void MessageManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserName = userName;
    activeUserId = userId;
}

void MessageManager::saveMessageFromDatabase(QJsonObject &json)
{
    logger->log(Logger::INFO,"messagemanager.cpp::saveMessageFromDatabase","saveMessageFromDatabase starts");
    QJsonArray messagesArray = json["messages"].toArray();
    if(messagesArray.isEmpty()){
        emit sendAvatarsUpdate();
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
        QString fileUrl = json["fileUrl"].toString();
        QString out = "";

        if(sender_login == activeUserName) {
            QString receiver_login = json["receiver_login"].toString();
            int receiver_id = json["receiver_id"].toInt();
            out = "out";
            saveMessageToJson(receiver_login, message, out, time, fulldate, message_id, dialog_id,receiver_id,fileUrl);
        }
        else {
            saveMessageToJson(sender_login, message, out, time, fulldate, message_id, dialog_id,sender_id,fileUrl);
        }
    }
    emit sendAvatarsUpdate();
}

void MessageManager::savePersonalMessage(const QJsonObject &personalMessageJson)
{
    QString message = personalMessageJson["message"].toString();
    QString time = personalMessageJson["time"].toString();
    int message_id = personalMessageJson["message_id"].toInt();
    int dialog_id = personalMessageJson["dialog_id"].toInt();
    QString fileUrl = "";
    if(personalMessageJson.contains("fileUrl"))  fileUrl = personalMessageJson["fileUrl"].toString();
    QString login;
    QString out = "";
    QString avatar_url;
    QString fullDate = "not:done(messagemanager::savePersonalMessage)";
    int id;

    logger->log(Logger::INFO,"messagemanager.cpp::processingPersonalMessageFromServer","Personal message received");

    if(personalMessageJson.contains("receiver_login")) {
        login = personalMessageJson["receiver_login"].toString();
        id = personalMessageJson["receiver_id"].toInt();
        avatar_url = personalMessageJson["receiver_avatar_url"].toString();
        out = "out";
        saveMessageToJson(login, message, out, time,fullDate, message_id,dialog_id,id,fileUrl);
    } else {
        login = personalMessageJson["sender_login"].toString();
        id = personalMessageJson["sender_id"].toInt();
        avatar_url = personalMessageJson["sender_avatar_url"].toString();

        saveMessageToJson(login, message, out, time, fullDate, message_id,dialog_id,id,fileUrl);
    }


    logger->log(Logger::INFO,"messagemanager.cpp::processingPersonalMessageFromServer","Message: " + message +" from: " + login);
    emit checkAndSendAvatarUpdate(avatar_url,id);

    QString fileName = "";
    if(fileUrl != "") {
        int underscoreIndex = fileUrl.indexOf('_');
        if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
            fileName = fileUrl.mid(underscoreIndex + 1);
        }
    }

    emit checkActiveDialog(login,message,out,time,fileName,fileUrl);
}

void MessageManager::loadingPersonalChat(const QString userlogin)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/resources/"+ activeUserName + "/personal");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal" +"/message_" + userlogin + ".json");

    if (!file.exists()) {
        logger->log(Logger::INFO,"messagemanager.cpp::loadingPersonalChat","File not exist, creating new file");

        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray emptyArray;
            QJsonDocument doc(emptyArray);
            file.write(doc.toJson());
            file.close();
        } else {
            logger->log(Logger::INFO,"messagemanager.cpp::checkingChatAvailability","File not create");
        }
    }

    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR,"messagemanager.cpp::loadingPersonalChat","File did not open with error: " + file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        QJsonArray chatHistory = doc.array();

        emit clearMainListView();

        logger->log(Logger::INFO,"messagemanager.cpp::loadingPersonalChat","Loading personal chat from json");
        for (const QJsonValue &value : chatHistory) {
            QJsonObject messageObject = value.toObject();
            QString user = messageObject["login"].toString();
            QString message = messageObject["str"].toString();
            QString out = messageObject["Out"].toString();

            QString fileUrl;
            if(messageObject.contains("fileUrl")) fileUrl = messageObject["fileUrl"].toString();
            else fileUrl = "";

            QString time = messageObject["time"].toString();

            loadMessageToQml(user,message,out,time, fileUrl);
        }
    }
    file.close();

}

void MessageManager::sendPersonalMessage(const QString &message, const QString &receiver_login, const int &receiver_id)
{
    QJsonObject personalMessageJson;

    personalMessageJson["flag"] = "personal_message";
    personalMessageJson["message"] = message;

    personalMessageJson["sender_login"] = activeUserName;
    personalMessageJson["sender_id"] = activeUserId;

    personalMessageJson["receiver_login"] = receiver_login;
    personalMessageJson["receiver_id"] = receiver_id;

    emit sendMessageJson(personalMessageJson);
}

void MessageManager::saveMessageAndSendFile(const QString &message, const QString &receiver_login, const int &receiver_id, const QString &filePath)
{
    QJsonObject jsonMessage;
    jsonMessage["message"] = message;
    jsonMessage["receiver_login"] = receiver_login;
    jsonMessage["receiver_id"] = receiver_id;

    QJsonDocument jsonDocument(jsonMessage);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Compact);

    QFile file("data.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(jsonData);
        file.close();
        emit sendFile(filePath);
    } else {
        logger->log(Logger::ERROR,"messagemanager.cpp::saveMessageAndSendFile", "File with message do not save");
    }
}

void MessageManager::sendPersonalMessageWithFile(const QString &fileUrl)
{
    QFile file("data.json");
    QByteArray jsonData;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        jsonData = file.readAll();
        file.close();
    }
    QJsonObject jsonObject = QJsonDocument::fromJson(jsonData).object();

    QJsonObject personalMessageJson;

    logger->log(Logger::DEBUG,"messagemanager.cpp::sendPersonalMessageWithFile","FileUrl = " + fileUrl);
    personalMessageJson["flag"] = "personal_message";
    personalMessageJson["message"] = jsonObject["message"].toString();
    personalMessageJson["fileUrl"] = fileUrl;

    personalMessageJson["sender_login"] = activeUserName;
    personalMessageJson["sender_id"] = activeUserId;

    personalMessageJson["receiver_login"] = jsonObject["receiver_login"].toString();;
    personalMessageJson["receiver_id"] = jsonObject["receiver_id"].toInt();

    emit sendMessageJson(personalMessageJson);
}

void MessageManager::sendVoiceMessage(const QString &receiver_login, const int &receiver_id)
{
    logger->log(Logger::DEBUG,"messagemanager.cpp::sendVoiceMessage", "sendVoiceMessage starts");

    QJsonObject voiceMessageJson;
    voiceMessageJson["flag"] = "voice_message";
    QString voicePath = QCoreApplication::applicationDirPath() + "/.tempData/" + activeUserName + "/voice_messages" + "/voiceMessage.wav";
    QFile file(voicePath);
    QFileInfo fileInfo(voicePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"messagemanager.cpp::sendVoiceMessage","Failed open file");
    }
    QByteArray voiceData = file.readAll();
    file.close();

    voiceMessageJson["fileName"] = fileInfo.baseName();
    voiceMessageJson["fileExtension"] = fileInfo.suffix();
    voiceMessageJson["fileData"] = QString(voiceData.toBase64());

    voiceMessageJson["sender_login"] = activeUserName;
    voiceMessageJson["sender_id"] = activeUserId;
    voiceMessageJson["receiver_login"] = receiver_login;
    voiceMessageJson["receiver_id"] = receiver_id;

    QJsonDocument doc(voiceMessageJson);

    emit sendToFileServer(doc);
}

void MessageManager::setLogger(Logger *logger)
{
    this->logger = logger;
}

void MessageManager::checkingChatAvailability(QString &login)
{
    logger->log(Logger::INFO,"messagemanager.cpp::checkingChatAvailability","Checking if a chat exists in a local save");
    QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal" +"/message_" + login + ".json");
    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR,"messagemanager.cpp::checkingChatAvailability","File did not open with error: " + file.errorString());
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

            emit showPersonalChat(login,message,id,out);
        } else {
            logger->log(Logger::INFO,"messagemanager.cpp::checkingChatAvailability","Chat history is empty");
        }
    }
    file.close();
}
