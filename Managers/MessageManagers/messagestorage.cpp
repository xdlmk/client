#include "messagestorage.h"

MessageStorage::MessageStorage(QObject *parent)
    : QObject{parent}
{}

void MessageStorage::setActiveUser(const QString &userLogin, const int &userId)
{
    this->activeUserLogin = userLogin;
    this->activeUserId = userId;
}

void MessageStorage::setLogger(Logger *logger)
{
    this->logger = logger;
}

void MessageStorage::saveMessageToJson(const QJsonObject &messageToSave)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/resources/" + activeUserLogin + "/personal");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserLogin + "/personal" +"/message_" + messageToSave["login"].toString() + ".json");

    if (!file.exists()) {
        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray emptyArray;
            QJsonDocument doc(emptyArray);
            file.write(doc.toJson());
            file.close();
        } else {
            logger->log(Logger::ERROR,"messagestorage.cpp::saveMessageToJson","File dont create " + messageToSave["login"].toString());
        }
    }
    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR,"messagestorage.cpp::saveMessageToJson","File did not open with error: " + file.errorString());
        return;
    }

    QJsonArray chatHistory;
    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        chatHistory = doc.array();
    }

    QJsonObject messageObject;
    if(messageToSave["Out"].toString() == "out") messageObject["login"] = activeUserLogin;
    else messageObject["login"] = messageToSave["login"].toString();

    messageObject["id"] = messageToSave["id"].toInt();
    messageObject["message_id"] = messageToSave["message_id"].toInt();
    messageObject["dialog_id"] = messageToSave["dialog_id"].toInt();
    messageObject["str"] = messageToSave["message"].toString();
    messageObject["Out"] = messageToSave["Out"].toString();
    messageObject["FullDate"] = messageToSave["FullDate"].toString();
    messageObject["time"] = messageToSave["time"].toString();
    messageObject["fileUrl"] = messageToSave["fileUrl"].toString();

    chatHistory.append(messageObject);

    file.resize(0);
    QJsonDocument updatedDoc(chatHistory);
    file.write(updatedDoc.toJson());
    file.close();

    emit showPersonalChat(messageToSave["login"].toString(), messageObject["str"].toString(), messageObject["id"].toInt(), messageObject["Out"].toString(), "personal");
}

void MessageStorage::saveGroupMessageToJson(const QJsonObject &messageToSave)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/resources/" + activeUserLogin + "/group");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserLogin + "/group" +"/message_" + messageToSave["group_name"].toString() + ".json");

    if (!file.exists()) {
        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray emptyArray;
            QJsonDocument doc(emptyArray);
            file.write(doc.toJson());
            file.close();
        } else {
            logger->log(Logger::ERROR,"messagestorage.cpp::saveGroupMessageToJson","File dont create " + messageToSave["login"].toString());
        }
    }

    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR,"messagestorage.cpp::saveGroupMessageToJson","File did not open with error: " + file.errorString());
        return;
    }

    QJsonArray chatHistory;
    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        chatHistory = doc.array();
    }

    QJsonObject messageObject;
    messageObject["login"] = messageToSave["login"].toString();;
    messageObject["id"] = messageToSave["id"].toInt();
    messageObject["message_id"] = messageToSave["message_id"].toInt();
    messageObject["group_name"] = messageToSave["group_name"].toString();
    messageObject["group_id"] = messageToSave["group_id"].toInt();
    messageObject["str"] = messageToSave["message"].toString();
    messageObject["Out"] = messageToSave["Out"].toString();
    messageObject["FullDate"] = messageToSave["FullDate"].toString();
    messageObject["time"] = messageToSave["time"].toString();
    messageObject["fileUrl"] = messageToSave["fileUrl"].toString();

    chatHistory.append(messageObject);

    file.resize(0);
    QJsonDocument updatedDoc(chatHistory);
    file.write(updatedDoc.toJson());
    file.close();

    emit showPersonalChat(messageObject["group_name"].toString(), messageObject["str"].toString(), messageObject["group_id"].toInt(), messageObject["Out"].toString(), "group");
}

void MessageStorage::updatingLatestMessagesFromServer(QJsonObject &latestMessages)
{
    logger->log(Logger::INFO,"messagestorage.cpp::saveMessageFromDatabase","saveMessageFromDatabase starts");
    QJsonArray messagesArray = latestMessages["messages"].toArray();

    QDir mesDir(QCoreApplication::applicationDirPath() + "/resources/" + activeUserLogin);
    mesDir.removeRecursively();

    for (const QJsonValue &value : messagesArray) {
        QJsonObject json = value.toObject();
        QJsonObject messageToSave;
        messageToSave["login"] = json["sender_login"].toString();
        messageToSave["id"] = json["sender_id"].toInt();
        messageToSave["message_id"] = json["message_id"].toInt();
        messageToSave["message"] = json["str"].toString();
        messageToSave["time"] = json["time"].toString();
        messageToSave["FullDate"] = json["FullDate"].toString();
        messageToSave["fileUrl"] = json["fileUrl"].toString();
        messageToSave["Out"] = "";

        if(json.contains("group_id")){
            messageToSave["group_id"] = json["group_id"].toInt();
            messageToSave["group_name"] = json["group_name"].toString();
            if( messageToSave["id"].toInt() == activeUserId) messageToSave["Out"] = "out";
            saveGroupMessageToJson(messageToSave);
            continue;
        }

        messageToSave["dialog_id"] = json["dialog_id"].toInt();

        if(messageToSave["login"].toString() == activeUserLogin) {
            messageToSave["login"] = json["receiver_login"].toString();
            messageToSave["id"] = json["receiver_id"].toInt();
            messageToSave["Out"] = "out";
            saveMessageToJson(messageToSave);
        } else saveMessageToJson(messageToSave);
    }
    emit sendAvatarsUpdate();
    emit getContactList();
}
