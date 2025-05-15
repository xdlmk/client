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

void MessageStorage::setCryptoManager(CryptoManager *cryptoManager)
{
    this->cryptoManager = cryptoManager;
}

bool MessageStorage::savePersonalMessageToFile(chats::ChatMessage &newMessage)
{
    QString basePath = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/messages/personal";
    QDir dir(basePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath;
    QByteArray encryptedSessionKey;
    if(newMessage.senderId() == activeUserId) {
        encryptedSessionKey = newMessage.senderEncryptedSessionKey();
        filePath = basePath + "/message_" + QString::number(newMessage.receiverId()) + ".pb";
    } else if (newMessage.receiverId() == activeUserId) {
        encryptedSessionKey = newMessage.receiverEncryptedSessionKey();
        filePath = basePath + "/message_" + QString::number(newMessage.senderId()) + ".pb";
    }

    QByteArray sessionKey;
    try {
        sessionKey = cryptoManager->unsealData(encryptedSessionKey);
    } catch (const std::exception &e) {
        logger->log(Logger::ERROR, "messagestorage.cpp::savePersonalMessageToFile", QString("Session key unsealing error: %1").arg(e.what()));
        return false;
    }
    QString content;
    if(newMessage.content() != ""){
        QByteArray encryptedMessageData = QByteArray::fromBase64(newMessage.content().toUtf8());
        QByteArray decryptedMessageData;
        try {
            decryptedMessageData = cryptoManager->symmetricDecrypt(encryptedMessageData, sessionKey);
        } catch (const std::exception &e) {
            logger->log(Logger::ERROR, "messagestorage.cpp::savePersonalMessageToFile", QString("Message decryption error: %1").arg(e.what()));
            return false;
        }
        content = QString::fromUtf8(decryptedMessageData);
    } else content = "";


    newMessage.setContent(content);

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
    QString message;
    if(newMessage.specialType() == "voice_message") {
        message = "Voice message";
    } else if(newMessage.specialType() == "file_message" && newMessage.content() == "") {
        message = newMessage.file().fileName();
    } else {
        message = newMessage.content();
    }

    if (newMessage.senderId() == activeUserId) {
        out = "out";
        emit showPersonalChat(newMessage.receiverLogin(), message, newMessage.receiverId(), out, "personal");
    } else if (newMessage.receiverId() == activeUserId) {
        emit showPersonalChat(newMessage.senderLogin(), message, newMessage.senderId(), out, "personal");
    }

    return true;
}

bool MessageStorage::saveGroupMessageToFile(chats::ChatMessage &newMessage)
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

    QString message;
    if(newMessage.specialType() == "voice_message") {
        message = "Voice message";

    } else if(newMessage.specialType() == "file_message" && newMessage.content() == "") {
        message = newMessage.file().fileName();
    } else {
        message = newMessage.content();
    }
    emit showPersonalChat(newMessage.groupName(), message, newMessage.groupId(), out, "group");

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

    for (auto &msg : messagesList) {
        if (msg.groupId() != 0) {
            saveGroupMessageToFile(msg);
        } else if(msg.receiverId() != 0){
            savePersonalMessageToFile(msg);
        }
    }

    emit sendAvatarsUpdate();
}

void MessageStorage::updateMessageStatus(const QByteArray &data)
{
    chats::MarkMessageResponse response;
    QProtobufSerializer serializer;
    response.deserialize(&serializer, data);
    quint64 message_id = response.messageId();
    quint64 reader_id = response.readerId();
    quint64 sender_id = response.chatId();

    QString basePath = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/messages/personal";
    QDir dir(basePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    quint64 chatId;
    if (activeUserId == reader_id) chatId = sender_id;
    else chatId = reader_id;

    QString filePath = basePath + "/message_" + QString::number(chatId) + ".pb";
    if (!QFile::exists(filePath)) {
        logger->log(Logger::WARN, "messagestorage.cpp::updateMessageStatus", "Message file not exists");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::WARN, "messagestorage.cpp::updateMessageStatus", "Failed to open file: " + file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    chats::MessageHistory history;
    if (!fileData.isEmpty()) {
        history.deserialize(&serializer, fileData);
    }

    QList<chats::ChatMessage> messages = history.messages();
    bool updated = false;
    for (int i = 0; i < messages.size(); i++) {
        if (messages[i].messageId() == message_id) {
            messages[i].setIsRead(true);
            updated = true;
            break;
        }
    }

    if (updated) {
        history.setMessages(messages);
        file.resize(0);
        QByteArray outData = history.serialize(&serializer);
        file.write(outData);
        emit setReadStatusToMessage(message_id, chatId, "personal");
    } else {
        logger->log(Logger::WARN, "messagestorage.cpp::updateMessageStatus", "Message with id " + QString::number(message_id) + " not found in history file");
    }
    file.close();
}
