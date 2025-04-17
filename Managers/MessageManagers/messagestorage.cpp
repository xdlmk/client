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
    QDir dir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/messages/personal");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QFile file(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/messages/personal" +"/message_" + messageToSave["login"].toString() + ".json");

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

bool MessageStorage::savePersonalMessageToFile(const quint64 &receiver_id, const chats::ChatMessage &newMessage)
{
    QString basePath = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/messages/personal";
    QDir dir(basePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath = basePath + "/message_" + QString::number(receiver_id) + ".pb";

    if (!QFile::exists(filePath)) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            chats::MessageHistory emptyHistory;
            QProtobufSerializer serializer;
            QByteArray emptyData = emptyHistory.serialize(&serializer);
            file.write(emptyData);
            file.close();
        } else {
            logger->log(Logger::ERROR, "messagestorage.cpp::savePersonalMessageToFile",
                        "Failed to create file: " + filePath);
            return false;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR, "messagestorage.cpp::savePersonalMessageToFile",
                    "Failed to open file: " + file.errorString());
        return false;
    }

    QByteArray fileData = file.readAll();
    chats::MessageHistory history;
    QProtobufSerializer serializer;
    if (!fileData.isEmpty()) {
        history.deserialize(&serializer, fileData);
    }

    QList<chats::ChatMessage> messages = history.messages();
    messages.append(newMessage);
    history.setMessages(messages);

    file.resize(0);
    QByteArray outData = history.serialize(&serializer);
    file.write(outData);
    file.close();

    return true;
}

void MessageStorage::saveGroupMessageToJson(const QJsonObject &messageToSave)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/messages/group");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QFile file(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/messages/group" +"/message_" + messageToSave["group_name"].toString() + ".json");

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

bool MessageStorage::saveGroupMessageToFile(const quint64 &group_id, const chats::ChatMessage &newMessage)
{
    QString basePath = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/messages/group";
    QDir dir(basePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath = basePath + "/group_" + QString::number(group_id) + ".pb";

    if (!QFile::exists(filePath)) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            chats::MessageHistory emptyHistory;
            QProtobufSerializer serializer;
            QByteArray emptyData = emptyHistory.serialize(&serializer);
            file.write(emptyData);
            file.close();
        } else {
            logger->log(Logger::ERROR, "messagestorage.cpp::saveGroupMessageToFile",
                        "Failed to create file: " + filePath);
            return false;
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR, "messagestorage.cpp::saveGroupMessageToFile",
                    "Failed to open file: " + file.errorString());
        return false;
    }

    QByteArray fileData = file.readAll();
    chats::MessageHistory history;
    QProtobufSerializer serializer;
    if (!fileData.isEmpty()) {
        history.deserialize(&serializer, fileData);
    }

    QList<chats::ChatMessage> messages = history.messages();
    messages.append(newMessage);
    history.setMessages(messages);

    file.resize(0);
    QByteArray outData = history.serialize(&serializer);
    file.write(outData);
    file.close();

    return true;
}

void MessageStorage::updatingLatestMessagesFromServer(const QByteArray &latestMessagesData)
{
    logger->log(Logger::INFO,"messagestorage.cpp::saveMessageFromDatabase","saveMessageFromDatabase starts");
    QProtobufSerializer serializer;
    chats::UpdatingChatsResponse latestMessages;

    if (!latestMessages.deserialize(&serializer, latestMessagesData)) {
        logger->log(Logger::WARN, "messagestorage.cpp::updatingLatestMessagesFromServer", "Error deserialize response from server");
        return;
    }

    if (latestMessages.status() == "error") {
        logger->log(Logger::WARN, "messagestorage.cpp::updatingLatestMessagesFromServer", "Error processing login");
        return;
    }

    QList<chats::ChatMessage> messagesList = latestMessages.messages();

    QDir mesDir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/messages");
    mesDir.removeRecursively();

    chats::UpdatingChatsResponse messageStorage;

    for (const auto &msg : messagesList) {
        if (msg.groupId() != 0) {
            saveGroupMessageToFile(msg.groupId(), msg);
        } else if(msg.receiverId() != 0){
            savePersonalMessageToFile(msg.receiverId(), msg);
        }
    }

    emit sendAvatarsUpdate();
    emit getContactList();
}
