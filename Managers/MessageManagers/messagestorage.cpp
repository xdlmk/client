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

bool MessageStorage::savePersonalMessageToFile(const chats::ChatMessage &newMessage)
{
    QString basePath = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/messages/personal";
    QDir dir(basePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath;
    if(newMessage.senderId() == activeUserId) {
        filePath = basePath + "/message_" + QString::number(newMessage.receiverId()) + ".pb";
    } else if (newMessage.receiverId() == activeUserId) {
        filePath = basePath + "/message_" + QString::number(newMessage.senderId()) + ".pb";
    }

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

    QString out = "";
    if (newMessage.senderId() == activeUserId) {
        out = "out";
        emit showPersonalChat(newMessage.receiverLogin(), newMessage.content(), newMessage.receiverId(), out, "personal");
    } else if (newMessage.receiverId() == activeUserId) {
        emit showPersonalChat(newMessage.senderLogin(), newMessage.content(), newMessage.senderId(), out, "personal");
    }

    return true;
}

bool MessageStorage::saveGroupMessageToFile(const chats::ChatMessage &newMessage)
{
    QString basePath = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/messages/group";
    QDir dir(basePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath = basePath + "/message_" + QString::number(newMessage.groupId()) + ".pb";

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

    QString out = "";
    if (newMessage.senderId() == activeUserId) out = "out";
    emit showPersonalChat(newMessage.groupName(), newMessage.content(), newMessage.groupId(), out, "group");

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
            saveGroupMessageToFile(msg);
        } else if(msg.receiverId() != 0){
            savePersonalMessageToFile(msg);
        }
    }

    emit sendAvatarsUpdate();
}
