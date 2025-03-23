#include "messagesender.h"

MessageSender::MessageSender(QObject *parent)
    : QObject{parent}
{}

void MessageSender::setActiveUser(const QString &userLogin, const int &userId)
{
    this->activeUserLogin = userLogin;
    this->activeUserId = userId;
}

void MessageSender::setLogger(Logger *logger)
{
    this->logger = logger;
}

void MessageSender::sendMessage(const QString &message, const QString &receiver_login, const int &receiver_id, const QString &flag)
{
    QJsonObject personalMessageJson;

    personalMessageJson["flag"] = flag + "_message";
    personalMessageJson["message"] = message;

    personalMessageJson["sender_login"] = activeUserLogin;
    personalMessageJson["sender_id"] = activeUserId;

    if(flag == "personal") {
        personalMessageJson["receiver_login"] = receiver_login;
        personalMessageJson["receiver_id"] = receiver_id;
    } else if(flag == "group") {
        personalMessageJson["group_name"] = receiver_login;
        personalMessageJson["group_id"] = receiver_id;
    }

    emit sendMessageJson(personalMessageJson);
}

void MessageSender::saveMessageAndSendFile(const QString &message, const QString &receiver_login, const int &receiver_id, const QString &filePath, const QString &flag)
{
    QJsonObject jsonMessage;
    jsonMessage["message"] = message;
    if(flag == "personal") {
        jsonMessage["receiver_login"] = receiver_login;
        jsonMessage["receiver_id"] = receiver_id;
    } else if(flag == "group") {
        jsonMessage["group_name"] = receiver_login;
        jsonMessage["group_id"] = receiver_id;
    }

    QJsonDocument jsonDocument(jsonMessage);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Compact);


    QDir dir(QCoreApplication::applicationDirPath() + "/.tempData/" + activeUserLogin + "/" + flag + "_messages");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(QCoreApplication::applicationDirPath() + "/.tempData/" + activeUserLogin + "/" + flag + "_messages/" +"data.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(jsonData);
        file.close();
        emit sendFile(filePath,flag + "_file");
    } else {
        logger->log(Logger::ERROR,"messagesender.cpp::saveMessageAndSendFile", "File with message do not save");
    }
}

void MessageSender::sendMessageWithFile(const QString &fileUrl, const QString &flag)
{
    QDir dir(QCoreApplication::applicationDirPath() + "/.tempData/" + activeUserLogin + "/" + flag + "_messages");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(QCoreApplication::applicationDirPath() + "/.tempData/" + activeUserLogin + "/" + flag + "_messages/" +"data.json");
    QByteArray jsonData;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        jsonData = file.readAll();
        file.close();
    }
    QJsonObject jsonObject = QJsonDocument::fromJson(jsonData).object();

    QJsonObject messageJson;

    logger->log(Logger::DEBUG,"messagesender.cpp::sendMessageWithFile","FileUrl = " + fileUrl);
    messageJson["flag"] = flag + "_message";
    messageJson["message"] = jsonObject["message"].toString();
    messageJson["fileUrl"] = fileUrl;
    messageJson["sender_login"] = activeUserLogin;
    messageJson["sender_id"] = activeUserId;

    if(flag == "personal") {
        messageJson["receiver_login"] = jsonObject["receiver_login"].toString();;
        messageJson["receiver_id"] = jsonObject["receiver_id"].toInt();
    } else if(flag == "group") {
        messageJson["group_name"] = jsonObject["group_name"].toString();;
        messageJson["group_id"] = jsonObject["group_id"].toInt();
    }

    emit sendMessageJson(messageJson);
}

void MessageSender::sendVoiceMessage(const QString &receiver_login, const int &receiver_id, const QString &flag)
{
    logger->log(Logger::DEBUG,"messagesender.cpp::sendVoiceMessage", "sendVoiceMessage starts");

    QJsonObject voiceMessageJson;
    voiceMessageJson["flag"] = flag + "_voice_message";
    QString voicePath = QCoreApplication::applicationDirPath() + "/.tempData/" + activeUserLogin + "/voice_messages" + "/voiceMessage.wav";
    QFile file(voicePath);
    QFileInfo fileInfo(voicePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"messagesender.cpp::sendVoiceMessage","Failed open file");
    }
    QByteArray voiceData = file.readAll();
    file.close();

    voiceMessageJson["fileName"] = fileInfo.baseName();
    voiceMessageJson["fileExtension"] = fileInfo.suffix();
    voiceMessageJson["fileData"] = QString(voiceData.toBase64());

    voiceMessageJson["sender_login"] = activeUserLogin;
    voiceMessageJson["sender_id"] = activeUserId;

    if(flag == "personal") {
        voiceMessageJson["receiver_login"] = receiver_login;
        voiceMessageJson["receiver_id"] = receiver_id;
    } else if(flag == "group") {
        voiceMessageJson["group_name"] = receiver_login;
        voiceMessageJson["group_id"] = receiver_id;
    }

    QJsonDocument doc(voiceMessageJson);

    emit sendToFileServer(doc);
}

void MessageSender::sendRequestMessagesLoading(const int &chat_id, const QString &chat_name, const QString &flag, const int &offset)
{
    QJsonObject request;
    request["flag"] = "load_messages";
    request["chat_id"] = chat_id;
    request["user_id"] = activeUserId;
    request["chat_name"] = chat_name;
    request["offset"] = offset;
    request["type"] = flag;

    emit sendMessageJson(request);
}
