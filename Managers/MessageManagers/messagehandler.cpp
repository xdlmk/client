#include "messagehandler.h"

MessageHandler::MessageHandler(QObject *parent)
    : QObject{parent}
{
    messageStorage = new MessageStorage(this);
    messageSender = new MessageSender(this);

    connect(this,&MessageHandler::updatingLatestMessagesFromServer,messageStorage,&MessageStorage::updatingLatestMessagesFromServer);

    connect(messageStorage,&MessageStorage::sendAvatarsUpdate,this,&MessageHandler::sendAvatarsUpdate);
    connect(messageStorage,&MessageStorage::showPersonalChat,this,&MessageHandler::showPersonalChat);
    connect(messageStorage,&MessageStorage::removeAccountFromConfigManager,this,&MessageHandler::removeAccountFromConfigManager);

    connect(this,&MessageHandler::sendMessage,messageSender,&MessageSender::sendMessage);
    connect(this,&MessageHandler::sendMessageWithFile,messageSender,&MessageSender::sendMessageWithFile);
    connect(this,&MessageHandler::sendVoiceMessage,messageSender,&MessageSender::sendVoiceMessage);
    connect(this,&MessageHandler::sendRequestMessagesLoading,messageSender,&MessageSender::sendRequestMessagesLoading);

    connect(messageSender,&MessageSender::sendMessageData,this,&MessageHandler::sendMessageData);

    connect(messageSender,&MessageSender::sendMessageFileData,this,&MessageHandler::sendMessageFileData);
}

void MessageHandler::setActiveUser(const QString &userLogin, const int &userId)
{
    this->activeUserLogin = userLogin;
    this->activeUserId = userId;
    messageStorage->setActiveUser(userLogin,userId);
    messageSender->setActiveUser(userLogin,userId);
}

void MessageHandler::setLogger(Logger *logger)
{
    this->logger = logger;
    messageStorage->setLogger(logger);
    messageSender->setLogger(logger);
}

void MessageHandler::loadMessageToQml(QVariantMap& messageToDisplay)
{
    QString fileUrl = messageToDisplay["fileUrl"].toString();
    if(fileUrl != "") {
        int underscoreIndex = fileUrl.indexOf('_');
        if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
            messageToDisplay["fileName"] = fileUrl.mid(underscoreIndex + 1);
        }
    } else messageToDisplay["fileName"] = "";
    emit newMessage(messageToDisplay);
}

void MessageHandler::processingPersonalMessage(const QByteArray &receivedMessageData)
{
    QProtobufSerializer serializer;
    chats::ChatMessage protoMsg;
    if (!protoMsg.deserialize(&serializer, receivedMessageData)) {
        logger->log(Logger::ERROR, "messagehandler.cpp::processingPersonalMessage", "Failed to deserialize personal message");
        return;
    }
    QVariantMap messageToLoad;

    messageToLoad["message"] = protoMsg.content();

    QString timestamp = protoMsg.timestamp();
    QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
    QString timeStr = dt.isValid() ? dt.toString("hh:mm") : "";
    messageToLoad["time"] = timeStr;

    messageToLoad["message_id"] = protoMsg.messageId();
    messageToLoad["fileUrl"] = protoMsg.mediaUrl();
    QString fileUrl = messageToLoad["fileUrl"].toString();
    QString fileName;
    if (!fileUrl.isEmpty()) {
        int underscoreIndex = fileUrl.indexOf('_');
        if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
            fileName = fileUrl.mid(underscoreIndex + 1);
        }
    }
    messageToLoad["fileName"] = fileName;

    messageToLoad["FullDate"] = timestamp;

    if(!protoMsg.specialType().isEmpty()){
        messageToLoad["special_type"] = protoMsg.specialType();
    }

    logger->log(Logger::INFO,"messagehandler.cpp::processingPersonalMessage","Personal message received");

    int conversationId = 0;

    if(protoMsg.senderId() == activeUserId) {
        messageToLoad["login"] = protoMsg.receiverLogin();
        messageToLoad["id"] = protoMsg.receiverId();
        messageToLoad["Out"] = "out";
        emit checkAndSendAvatarUpdate(protoMsg.receiverAvatarUrl(), protoMsg.receiverId(), "personal");
    } else if (protoMsg.receiverId() == activeUserId) {
        messageToLoad["login"] = protoMsg.senderLogin();
        messageToLoad["id"] = protoMsg.senderId();
        messageToLoad["Out"] = "";
        emit checkAndSendAvatarUpdate(protoMsg.senderAvatarUrl(), protoMsg.senderId(), "personal");
    }
    messageStorage->savePersonalMessageToFile(protoMsg);
    logger->log(Logger::INFO,"messagehandler.cpp::processingPersonalMessage","Message: " + messageToLoad["message"].toString() +" from: " + messageToLoad["login"].toString());

    emit checkActiveDialog(messageToLoad,"personal");
}

void MessageHandler::processingGroupMessage(const QByteArray &receivedMessageData)
{
    QProtobufSerializer serializer;
    chats::ChatMessage protoMsg;
    if (!protoMsg.deserialize(&serializer, receivedMessageData)) {
        logger->log(Logger::ERROR, "messagehandler.cpp::processingGroupMessage", "Failed to deserialize group message");
        return;
    }

    QVariantMap messageToLoad;
    messageToLoad["message"] = protoMsg.content();

    QString timestamp = protoMsg.timestamp();
    QDateTime dt = QDateTime::fromString(timestamp, Qt::ISODate);
    QString timeStr = dt.isValid() ? dt.toString("hh:mm") : "";
    messageToLoad["time"] = timeStr;

    messageToLoad["message_id"] = protoMsg.messageId();
    messageToLoad["fileUrl"] = protoMsg.mediaUrl();
    QString fileUrl = messageToLoad["fileUrl"].toString();

    QString fileName;
    if (!fileUrl.isEmpty()) {
        int underscoreIndex = fileUrl.indexOf('_');
        if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
            fileName = fileUrl.mid(underscoreIndex + 1);
        }
    }
    messageToLoad["fileName"] = fileName;

    messageToLoad["FullDate"] = timestamp;

    if(!protoMsg.specialType().isEmpty()){
        messageToLoad["special_type"] = protoMsg.specialType();
    }

    messageToLoad["group_name"] = protoMsg.groupName();
    messageToLoad["group_id"] = protoMsg.groupId();

    emit checkAndSendAvatarUpdate(protoMsg.groupAvatarUrl(), protoMsg.groupId(), "group");

    if(protoMsg.senderId() == activeUserId) {
        messageToLoad["Out"] = "out";
    } else {
        messageToLoad["Out"] = "";
        emit checkAndSendAvatarUpdate(protoMsg.senderAvatarUrl(), protoMsg.senderId(), "personal");
    }
    messageToLoad["login"] = protoMsg.senderLogin();
    messageToLoad["id"] = protoMsg.senderId();

    messageStorage->saveGroupMessageToFile(protoMsg);

    emit checkActiveDialog(messageToLoad, "group");
}

void MessageHandler::loadingChat(const quint64& id, const QString &flag)
{
    QString baseDir = QCoreApplication::applicationDirPath() + "/.data/" +
                      QString::number(activeUserId) + "/messages/" + flag;

    QDir dir(baseDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(baseDir + "/message_" + QString::number(id) + ".pb");

    if (!file.exists()) {
        logger->log(Logger::INFO,"messagehandler.cpp::loadingChat","File not exist, creating new file");

        if (file.open(QIODevice::WriteOnly)) {
            chats::MessageHistory emptyHistory;
            QProtobufSerializer serializer;
            QByteArray emptyData = emptyHistory.serialize(&serializer);
            file.write(emptyData);
            file.close();
        } else {
            logger->log(Logger::INFO,"messagehandler.cpp::loadingChat","File not create");
        }
    }

    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR,"messagehandler.cpp::loadingChat","File did not open with error: " + file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        chats::MessageHistory history;
        QProtobufSerializer serializer;
        if (!history.deserialize(&serializer, fileData)) {
            logger->log(Logger::ERROR, "messagehandler.cpp::loadingChat", "Failed to deserialize MessageHistory");
            file.close();
            return;
        }
        QList<chats::ChatMessage> messages = history.messages();

        emit clearMainListView();

        logger->log(Logger::INFO,"messagehandler.cpp::loadingChat","Loading personal chat from json");
        for (const auto &msg : messages) {
            QVariantMap messageToDisplay;

            messageToDisplay["login"] = msg.senderLogin();
            if (msg.senderId() == activeUserId) {
                messageToDisplay["Out"] = "out";
            } else {
                messageToDisplay["Out"] = "";
            }

            messageToDisplay["message"] = msg.content();
            //messageToDisplay["message_id"] = msg.messageId();
            messageToDisplay["fileUrl"] = msg.mediaUrl();

            QString fullDate = msg.timestamp();
            QDateTime dt = QDateTime::fromString(fullDate, Qt::ISODate);
            QString time = dt.isValid() ? dt.toString("hh:mm") : "";
            messageToDisplay["time"] = time;
            messageToDisplay["timestamp"] = fullDate;
            messageToDisplay["special_type"] = msg.specialType();

            loadMessageToQml(messageToDisplay);
        }
    }
    file.close();
}

void MessageHandler::loadingNextMessages(const QByteArray &messagesData)
{
    QProtobufSerializer serializer;
    chats::LoadMessagesResponse response;
    if (!response.deserialize(&serializer, messagesData)) {
        logger->log(Logger::ERROR, "messagehandler.cpp::loadingNextMessages", "Failed deserialize response");
        return;
    }

    QString chatName = response.chatName();
    int type = static_cast<int>(response.type());
    logger->log(Logger::INFO, "messagehandler.cpp::loadingNextMessages", "Start processing messages");

    int count = response.messages().size();

    for (int i = response.messages().size() - 1; i >= 0; --i) {
        const chats::ChatMessage &protoMsg = response.messages().at(i);
        QVariantMap messageToLoad;

        messageToLoad["message"] = protoMsg.content();

        QString fullDate = protoMsg.timestamp();
        QDateTime dt = QDateTime::fromString(fullDate, Qt::ISODate);
        QString time = dt.isValid() ? dt.toString("hh:mm") : "";
        messageToLoad["time"] = time;
        messageToLoad["FullDate"] = fullDate;

        messageToLoad["message_id"] = protoMsg.messageId();
        messageToLoad["login"] = protoMsg.senderLogin();
        messageToLoad["id"] = protoMsg.senderId();
        messageToLoad["fileUrl"] = protoMsg.mediaUrl();

        QString fileUrl = messageToLoad["fileUrl"].toString();
        QString fileName;
        if(fileUrl != "") {
            int underscoreIndex = fileUrl.indexOf('_');
            if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
                fileName = fileUrl.mid(underscoreIndex + 1);
            }
        }
        messageToLoad["fileName"] = fileName;

        if (protoMsg.groupId() != 0) {
            messageToLoad["group_id"] = protoMsg.groupId();
            messageToLoad["group_name"] = protoMsg.groupName();
            QVariant messageVariant(messageToLoad);
            bool out = (messageToLoad["id"].toInt() == activeUserId);
            emit insertMessage(messageVariant, out);
            continue;
        }

        if (messageToLoad["id"].toInt() == activeUserId) {
            messageToLoad["login"] = protoMsg.receiverLogin();
            messageToLoad["id"] = protoMsg.receiverId();
            QVariant messageVariant(messageToLoad);
            emit insertMessage(messageVariant, true);
        } else {
            QVariant messageVariant(messageToLoad);
            emit insertMessage(messageVariant, false);
        }
    }
    emit returnChatToPosition();
}
