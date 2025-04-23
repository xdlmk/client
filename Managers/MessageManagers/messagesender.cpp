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

void MessageSender::sendMessageWithFile(const QString &message, const int &receiver_id, const QString &filePath, const QString &flag)
{
    logger->log(Logger::DEBUG,"messagesender.cpp::sendMessageWithFile", "sendMessageWithFile starts");

    chats::ChatMessage chatMsg;
    chatMsg.setContent(message);
    chatMsg.setSenderId(activeUserId);

    if(flag == "personal") {
        chatMsg.setReceiverId(receiver_id);
    } else if(flag == "group"){
        chatMsg.setGroupId(receiver_id);
    }

    QFile file(filePath);
    QFileInfo fileInfo(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"messagesender.cpp::sendMessageWithFile","Failed open file");
    }
    QByteArray fileData = file.readAll();
    file.close();

    chats::FileData fileDataMsg;
    fileDataMsg.setFileName(fileInfo.baseName());
    fileDataMsg.setFileExtension(fileInfo.suffix());
    fileDataMsg.setFileData(fileData);

    chatMsg.setFile(fileDataMsg);

    QProtobufSerializer serializer;
    emit sendMessageFileData(flag + "_file_message", chatMsg.serialize(&serializer)); // personal_file_message group_file_message
}

void MessageSender::sendVoiceMessage(const int &receiver_id, const QString &flag)
{
    logger->log(Logger::DEBUG,"messagesender.cpp::sendVoiceMessage", "sendVoiceMessage starts");

    chats::ChatMessage chatMsg;
    chatMsg.setSenderId(activeUserId);

    if(flag == "personal") {
        chatMsg.setReceiverId(receiver_id);
    } else if(flag == "group"){
        chatMsg.setGroupId(receiver_id);
    }

    QString voicePath = QCoreApplication::applicationDirPath() + "/.tempData/" + QString::number(activeUserId) + "/voice_messages" + "/voiceMessage.wav";
    QFile file(voicePath);
    QFileInfo fileInfo(voicePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"messagesender.cpp::sendVoiceMessage","Failed open file");
    }
    QByteArray voiceData = file.readAll();
    file.close();

    chats::FileData fileDataMsg;
    fileDataMsg.setFileName(fileInfo.baseName());
    fileDataMsg.setFileExtension(fileInfo.suffix());
    fileDataMsg.setFileData(voiceData);

    chatMsg.setFile(fileDataMsg);

    QProtobufSerializer serializer;
    emit sendMessageFileData(flag + "_voice_message", chatMsg.serialize(&serializer)); // personal_voice_message group_voice_message
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
