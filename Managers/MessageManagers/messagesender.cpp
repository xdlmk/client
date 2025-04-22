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

void MessageSender::sendMessage(const QString &message, const int &receiver_id, const QString &flag)
{
    chats::ChatMessage msg;
    msg.setContent(message);
    msg.setSenderId(activeUserId);

    if(flag == "personal") {
        msg.setReceiverId(receiver_id);
    } else if(flag == "group"){
        msg.setGroupId(receiver_id);
    }
    QProtobufSerializer serializer;
    emit sendMessageData(flag + "_message",msg.serialize(&serializer)); // personal_message group_message
}

void MessageSender::sendMessageWithFile(const QString &message, const QString &receiver_login, const int &receiver_id, const QString &filePath, const QString &flag)
{
    logger->log(Logger::DEBUG,"messagesender.cpp::sendMessageWithFile", "sendMessageWithFile starts");

    QJsonObject fileMessageJson;
    fileMessageJson["flag"] = flag + "_file_message"; // personal_file_message group_file_message
    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"messagesender.cpp::sendMessageWithFile","Failed open file");
    }
    QByteArray fileData = file.readAll();
    file.close();

    fileMessageJson["fileName"] = fileInfo.baseName();
    fileMessageJson["fileExtension"] = fileInfo.suffix();
    fileMessageJson["fileData"] = QString(fileData.toBase64());

    fileMessageJson["sender_login"] = activeUserLogin;
    fileMessageJson["sender_id"] = activeUserId;
    fileMessageJson["message"] = message;

    if(flag == "personal") {
        fileMessageJson["receiver_login"] = receiver_login;
        fileMessageJson["receiver_id"] = receiver_id;
    } else if(flag == "group") {
        fileMessageJson["group_name"] = receiver_login;
        fileMessageJson["group_id"] = receiver_id;
    }

    QJsonDocument doc(fileMessageJson);

    emit sendToFileServer(doc);
}

void MessageSender::sendVoiceMessage(const QString &receiver_login, const int &receiver_id, const QString &flag)
{
    logger->log(Logger::DEBUG,"messagesender.cpp::sendVoiceMessage", "sendVoiceMessage starts");

    QJsonObject voiceMessageJson;
    voiceMessageJson["flag"] = flag + "_voice_message"; //personal_voice_message group_voice_message
    QString voicePath = QCoreApplication::applicationDirPath() + "/.tempData/" + QString::number(activeUserId) + "/voice_messages" + "/voiceMessage.wav";
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
    chats::LoadMessagesRequest request;
    request.setChatId(chat_id);
    request.setUserId(activeUserId);
    request.setOffset(offset);
    if(flag == "personal") {
        request.setType(chats::ChatTypeGadget::ChatType::PERSONAL);
    } else if(flag == "group") {
        request.setType(chats::ChatTypeGadget::ChatType::GROUP);
    }
    QProtobufSerializer serializer;
    emit sendMessageData("load_messages", request.serialize(&serializer));
    /*
    QJsonObject request;
    request["flag"] = "load_messages";
    request["chat_id"] = chat_id;
    request["user_id"] = activeUserId;
    request["chat_name"] = chat_name;
    request["offset"] = offset;
    request["type"] = flag;
    emit sendMessageJson(request);*/

}
