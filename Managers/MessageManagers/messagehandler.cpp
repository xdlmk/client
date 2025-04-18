#include "messagehandler.h"

MessageHandler::MessageHandler(QObject *parent)
    : QObject{parent}
{
    messageStorage = new MessageStorage(this);
    messageSender = new MessageSender(this);
    avatarGenerator = new AvatarGenerator(this);

    connect(this,&MessageHandler::updatingLatestMessagesFromServer,messageStorage,&MessageStorage::updatingLatestMessagesFromServer);

    connect(messageStorage,&MessageStorage::getContactList,this,&MessageHandler::getContactList);
    connect(messageStorage,&MessageStorage::sendAvatarsUpdate,this,&MessageHandler::sendAvatarsUpdate);
    connect(messageStorage,&MessageStorage::showPersonalChat,this,&MessageHandler::showPersonalChat);
    connect(messageStorage,&MessageStorage::removeAccountFromConfigManager,this,&MessageHandler::removeAccountFromConfigManager);

    connect(this,&MessageHandler::sendMessage,messageSender,&MessageSender::sendMessage);
    connect(this,&MessageHandler::sendMessageWithFile,messageSender,&MessageSender::sendMessageWithFile);
    connect(this,&MessageHandler::sendVoiceMessage,messageSender,&MessageSender::sendVoiceMessage);
    connect(this,&MessageHandler::sendRequestMessagesLoading,messageSender,&MessageSender::sendRequestMessagesLoading);

    connect(messageSender,&MessageSender::sendMessageJson,this,&MessageHandler::sendMessageJson);
    connect(messageSender,&MessageSender::sendToFileServer,this,&MessageHandler::sendToFileServer);
}

void MessageHandler::setActiveUser(const QString &userLogin, const int &userId)
{
    this->activeUserLogin = userLogin;
    this->activeUserId = userId;
    messageStorage->setActiveUser(userLogin,userId);
    messageSender->setActiveUser(userLogin,userId);
    avatarGenerator->setActiveUser(userLogin,userId);
}

void MessageHandler::setLogger(Logger *logger)
{
    this->logger = logger;
    messageStorage->setLogger(logger);
    messageSender->setLogger(logger);
}

void MessageHandler::checkingChatAvailability(QString &login, const QString &flag)
{
    logger->log(Logger::INFO,"messagehandler.cpp::checkingChatAvailability","Checking if a chat exists in a local save");
    QFile file(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/messages/" + flag + "/message_" + login + ".json");
    if (!file.open(QIODevice::ReadWrite)) {
        logger->log(Logger::ERROR,"messagehandler.cpp::checkingChatAvailability","File did not open with error: " + file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();
    if (!fileData.isEmpty()) {
        QJsonArray chatHistory = QJsonDocument::fromJson(fileData).array();

        if (!chatHistory.isEmpty()) {
            QJsonObject lastMessageObject = chatHistory.last().toObject();

            QString message = lastMessageObject["str"].toString();

            int id;
            if(flag == "group") id = lastMessageObject["group_id"].toInt();
            else if (flag == "personal") id = lastMessageObject["id"].toInt();

            QString out = lastMessageObject["Out"].toString();

            emit showPersonalChat(login, message, id, out, flag);
        } else {
            logger->log(Logger::INFO,"messagehandler.cpp::checkingChatAvailability","Chat history is empty");
        }
    }
    file.close();
}

void MessageHandler::loadMessageToQml(QJsonObject& messageToDisplay)
{
    QString fileUrl = messageToDisplay["fileUrl"].toString();
    if(fileUrl != "") {
        int underscoreIndex = fileUrl.indexOf('_');
        if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
            messageToDisplay["fileName"] = fileUrl.mid(underscoreIndex + 1);
        }
    } else messageToDisplay["fileName"] = "";
    QVariant message = messageToDisplay.toVariantMap();
    emit newMessage(message);
}

void MessageHandler::processingPersonalMessage(const QJsonObject &personalMessageJson)
{
    QJsonObject messageToSave;
    messageToSave["message"] = personalMessageJson["message"].toString();
    messageToSave["time"] = personalMessageJson["time"].toString();
    messageToSave["message_id"] = personalMessageJson["message_id"].toInt();
    messageToSave["dialog_id"] = personalMessageJson["dialog_id"].toInt();
    if(personalMessageJson.contains("fileUrl"))  messageToSave["fileUrl"] = personalMessageJson["fileUrl"].toString();
    else messageToSave["fileUrl"] = "";
    QString fileUrl = messageToSave["fileUrl"].toString();
    QString avatar_url;
    int id;
    messageToSave["FullDate"] = "not:done(messagehandler::processingPersonalMessage)";

    logger->log(Logger::INFO,"messagehandler.cpp::processingPersonalMessage","Personal message received");

    if(personalMessageJson.contains("receiver_login")) {
        messageToSave["login"] = personalMessageJson["receiver_login"].toString();
        id = personalMessageJson["receiver_id"].toInt();
        messageToSave["id"] = id;
        avatar_url = personalMessageJson["receiver_avatar_url"].toString();
        messageToSave["Out"] = "out";
        messageStorage->saveMessageToJson(messageToSave);
    } else {
        messageToSave["login"] = personalMessageJson["sender_login"].toString();
        id = personalMessageJson["sender_id"].toInt();
        messageToSave["id"] = id;
        avatar_url = personalMessageJson["sender_avatar_url"].toString();
        messageToSave["Out"] = "";
        messageStorage->saveMessageToJson(messageToSave);
    }

    logger->log(Logger::INFO,"messagehandler.cpp::processingPersonalMessage","Message: " + messageToSave["message"].toString() +" from: " + messageToSave["login"].toString());
    emit checkAndSendAvatarUpdate(avatar_url,id,"personal");

    QString fileName = "";
    if(fileUrl != "") {
        int underscoreIndex = fileUrl.indexOf('_');
        if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
            fileName = fileUrl.mid(underscoreIndex + 1);
        }
    }
    messageToSave["fileName"] = fileName;

    QVariantMap message = messageToSave.toVariantMap();
    emit checkActiveDialog(message,"personal");
    emit getContactList();
}

void MessageHandler::processingGroupMessage(const QJsonObject &groupMessageJson)
{
    QJsonObject messageToSave;
    messageToSave["message"] = groupMessageJson["message"].toString();
    messageToSave["time"] = groupMessageJson["time"].toString();
    messageToSave["message_id"] = groupMessageJson["message_id"].toInt();
    if(groupMessageJson.contains("fileUrl"))   messageToSave["fileUrl"] = groupMessageJson["fileUrl"].toString();
    else messageToSave["fileUrl"] = "";
    if(groupMessageJson["special_type"].toString() == "create") {
        emit getChatsInfo();
    }

    QString fileUrl = messageToSave["fileUrl"].toString();
    messageToSave["group_name"] = groupMessageJson["group_name"].toString();
    messageToSave["group_id"] = groupMessageJson["group_id"].toInt();
    QString out = "";
    messageToSave["FullDate"] = "not:done(messagemanager::saveGroupMessage)";
    if(groupMessageJson.contains("group_avatar_url")){
        if(groupMessageJson["group_avatar_url"].toString() == ""){
            avatarGenerator->generateAvatarImage(groupMessageJson["group_name"].toString(),groupMessageJson["group_id"].toInt(),"group");
        } else {
            emit checkAndSendAvatarUpdate(groupMessageJson["group_avatar_url"].toString(),groupMessageJson["group_id"].toInt(),"group");
        }
    }

    if(groupMessageJson["sender_id"].toInt() != activeUserId) {
        messageToSave["Out"] = "";
        emit checkAndSendAvatarUpdate(groupMessageJson["sender_avatar_url"].toString(),groupMessageJson["sender_id"].toInt(), "personal");
    } else {
        messageToSave["Out"] = "out";
    }
    messageToSave["login"] = groupMessageJson["sender_login"].toString();
    messageToSave["id"] = groupMessageJson["sender_id"].toInt();

    messageStorage->saveGroupMessageToJson(messageToSave);

    QString fileName = "";
    if(fileUrl != "") {
        int underscoreIndex = fileUrl.indexOf('_');
        if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
            fileName = fileUrl.mid(underscoreIndex + 1);
        }
    }
    messageToSave["fileName"] = fileName;

    QVariant message = messageToSave.toVariantMap();
    emit checkActiveDialog(message,"group");
}

void MessageHandler::loadingChat(const QString userlogin, const QString &flag)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/.data/"+ QString::number(activeUserId) + "/messages/" + flag);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/messages/" + flag +"/message_" + userlogin + ".json");

    if (!file.exists()) {
        logger->log(Logger::INFO,"messagehandler.cpp::loadingChat","File not exist, creating new file");

        if (file.open(QIODevice::WriteOnly)) {
            QJsonArray emptyArray;
            QJsonDocument doc(emptyArray);
            file.write(doc.toJson());
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
        QJsonDocument doc = QJsonDocument::fromJson(fileData);
        QJsonArray chatHistory = doc.array();

        emit clearMainListView();

        logger->log(Logger::INFO,"messagehandler.cpp::loadingChat","Loading personal chat from json");
        for (const QJsonValue &value : chatHistory) {
            QJsonObject messageObject = value.toObject();
            QJsonObject messageToDisplay;
            messageToDisplay["login"] = messageObject["login"].toString();
            messageToDisplay["message"] = messageObject["str"].toString();
            messageToDisplay["Out"] = messageObject["Out"].toString();

            if(messageObject.contains("fileUrl")) messageToDisplay["fileUrl"] = messageObject["fileUrl"].toString();
            else messageToDisplay["fileUrl"] = "";

            messageToDisplay["time"] = messageObject["time"].toString();

            loadMessageToQml(messageToDisplay);
        }
    }
    file.close();
}

void MessageHandler::loadingNextMessages(QJsonObject &messagesJson)
{
    QString type = messagesJson["type"].toString();
    QString chatName = messagesJson["chat_name"].toString();
    QJsonArray newMessages = messagesJson["messages"].toArray();

    logger->log(Logger::INFO, "messagehandler.cpp::loadingNextMessages", "Start processing messages");

    QJsonArray messagesArray = messagesJson["messages"].toArray();

    for (int i = messagesArray.size() - 1; i >= 0; --i) {
        QJsonObject json = messagesArray[i].toObject();

        QJsonObject messageToLoad;
        messageToLoad["message"] = json["str"].toString();
        messageToLoad["time"] = json["time"].toString();
        messageToLoad["FullDate"] = json["FullDate"].toString();
        messageToLoad["message_id"] = json["message_id"].toInt();
        messageToLoad["login"] = json["sender_login"].toString();
        messageToLoad["id"] = json["sender_id"].toInt();
        messageToLoad["fileUrl"] = json["fileUrl"].toString();
        QString fileUrl = messageToLoad["fileUrl"].toString();

        QString fileName = "";
        if(fileUrl != "") {
            int underscoreIndex = fileUrl.indexOf('_');
            if (underscoreIndex != -1 && underscoreIndex + 1 < fileUrl.length()) {
                fileName = fileUrl.mid(underscoreIndex + 1);
            }
        }
        messageToLoad["fileName"] = fileName;

        if (json.contains("group_id")) {
            messageToLoad["group_id"] = json["group_id"].toInt();
            messageToLoad["group_name"] = json["group_name"].toString();
            QVariant message = messageToLoad.toVariantMap();
            if (messageToLoad["id"].toInt() == activeUserId) emit insertMessage(message, true);
            else emit insertMessage(message, false);

            continue;
        }

        messageToLoad["dialog_id"] = json["dialog_id"].toInt();

        if (messageToLoad["id"].toInt() == activeUserId) {
            messageToLoad["login"] = json["receiver_login"].toString();
            messageToLoad["id"] = json["receiver_id"].toInt();
            QVariant message = messageToLoad.toVariantMap();
            emit insertMessage(message, true);
        } else {
            QVariant message = messageToLoad.toVariantMap();
            emit insertMessage(message, false);
        }
    }
    emit returnChatToPosition();
}
