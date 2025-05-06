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

void MessageSender::setCryptoManager(CryptoManager *cryptoManager)
{
    this->cryptoManager = cryptoManager;
}

void MessageSender::sendMessage(const QString &message, const quint64 &receiver_id, const QString &flag)
{
    logger->log(Logger::INFO, "messagesender.cpp::sendMessage", "Method starts");
    chats::ChatMessage msg;
    QProtobufSerializer serializer;
    msg.setSenderId(activeUserId);

    if(flag == "personal") {
        logger->log(Logger::INFO, "messagesender.cpp::sendMessage", "flag = personal");
        QString dialogInfoPath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo" + "/" + QString::number(receiver_id) + ".pb";
        if(!QFile::exists(dialogInfoPath)) {
            logger->log(Logger::INFO, "messagesender.cpp::sendMessage", "dialogInfo not exists");
            chats::CreateDialogRequest request;
            request.setSenderId(activeUserId);
            request.setReceiverId(receiver_id);

            QString uniqName = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QString baseDir = QCoreApplication::applicationDirPath()
                              + "/.tempData/"
                              + QString::number(receiver_id);

            QDir dir;
            if (!dir.exists(baseDir)) dir.mkpath(baseDir);

            QString filePath = baseDir + "/" + uniqName + ".txt";

            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream out(&file);
                out << message;
                file.close();
            } else {
                logger->log(Logger::ERROR, "messagesender.cpp::sendMessage", "File not open for save message");
                return;
            }

            request.setUniqMessageId(uniqName);

            emit sendMessageData("create_dialog", request.serialize(&serializer));
            return;
        }

        QByteArray encryptedMessage;
        try {
            encryptedMessage = cryptoManager->symmetricEncrypt(message.toUtf8(),cryptoManager->getDecryptedSessionKey(dialogInfoPath));
        } catch (const std::exception &e) {
            logger->log(Logger::ERROR, "messagesender.cpp::sendMessage", QString("Error encrypting message: ") + QString(e.what()));
            return;
        }

        QString content = QString::fromUtf8(encryptedMessage.toBase64());
        msg.setContent(content);
        msg.setReceiverId(receiver_id);
    } else if(flag == "group"){
        msg.setContent(message);
        msg.setGroupId(receiver_id);
    }

    emit sendMessageData(flag + "_message",msg.serialize(&serializer)); // personal_message group_message
    return;
}

void MessageSender::sendMessageWithFile(const QString &message, const int &receiver_id, const QString &filePath, const QString &flag)
{
    logger->log(Logger::DEBUG,"messagesender.cpp::sendMessageWithFile", "sendMessageWithFile starts");

    QProtobufSerializer serializer;
    chats::ChatMessage chatMsg;
    chatMsg.setContent(message);
    chatMsg.setSenderId(activeUserId);

    if(flag == "personal") {
        logger->log(Logger::INFO, "messagesender.cpp::sendMessageWithFile", "flag = personal");

        QString dialogInfoPath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo" + "/" + QString::number(receiver_id) + ".pb";
        if(!QFile::exists(dialogInfoPath)) {
            logger->log(Logger::INFO, "messagesender.cpp::sendMessageWithFile", "dialogInfo not exists");
            chats::CreateDialogRequest request;
            request.setSenderId(activeUserId);
            request.setReceiverId(receiver_id);

            QString uniqName = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QString baseDir = QCoreApplication::applicationDirPath()
                              + "/.tempData/"
                              + QString::number(receiver_id) + "/files/" + flag;

            QDir dir;
            if (!dir.exists(baseDir)) dir.mkpath(baseDir);

            QString filePath = baseDir + "/" + uniqName + ".txt";
            QString metaFilePath = baseDir + "/" + uniqName + ".meta";

            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream out(&file);
                out << message;
                file.close();
            } else {
                logger->log(Logger::ERROR, "messagesender.cpp::sendMessageWithFile", "File not open for save message");
                return;
            }
            QFile metaFile(metaFilePath);
            if (metaFile.open(QIODevice::WriteOnly)) {
                QTextStream outMeta(&metaFile);
                outMeta << filePath;
                metaFile.close();
            } else {
                logger->log(Logger::ERROR, "messagesender.cpp::sendMessageWithFile", "File not open for saving file path");
                return;
            }

            request.setUniqMessageId(uniqName);

            emit sendMessageData("create_dialog", request.serialize(&serializer));
            return;
        }

        QByteArray encryptedMessage;
        try {
            encryptedMessage = cryptoManager->symmetricEncrypt(message.toUtf8(),cryptoManager->getDecryptedSessionKey(dialogInfoPath));
        } catch (const std::exception &e) {
            logger->log(Logger::ERROR, "messagesender.cpp::sendMessageWithFile", QString("Error encrypting message: ") + QString(e.what()));
            return;
        }

        QString content = QString::fromUtf8(encryptedMessage.toBase64());
        chatMsg.setContent(content);
        chatMsg.setReceiverId(receiver_id);

        QFile file(filePath);
        QFileInfo fileInfo(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            logger->log(Logger::WARN,"messagesender.cpp::sendMessageWithFile","Failed open file");
        }
        QByteArray encryptedFileData;
        try {
            encryptedFileData = cryptoManager->symmetricEncrypt(file.readAll(),cryptoManager->getDecryptedSessionKey(dialogInfoPath));
        } catch (const std::exception &e) {
            logger->log(Logger::ERROR, "messagesender.cpp::sendMessageWithFile", QString("Error encrypting message: ") + QString(e.what()));
            return;
        }
        file.close();

        chats::FileData fileDataMsg;
        fileDataMsg.setFileName(fileInfo.baseName());
        fileDataMsg.setFileExtension(fileInfo.suffix());
        fileDataMsg.setFileData(encryptedFileData);

        chatMsg.setFile(fileDataMsg);

        emit sendMessageFileData(flag + "_file_message", chatMsg.serialize(&serializer)); // personal_file_message group_file_message

    } else if(flag == "group"){
        chatMsg.setContent(message);
        chatMsg.setGroupId(receiver_id);

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
}

void MessageSender::sendVoiceMessage(const int &receiver_id, const QString &flag)
{
    logger->log(Logger::DEBUG,"messagesender.cpp::sendVoiceMessage", "sendVoiceMessage starts");

    QString filePath = QCoreApplication::applicationDirPath() + "/.tempData/" + QString::number(activeUserId) + "/voice_messages/" + flag + "/" + QString::number(receiver_id) + "/voiceMessage.wav";

    QProtobufSerializer serializer;
    chats::ChatMessage chatMsg;
    chatMsg.setSenderId(activeUserId);

    if(flag == "personal") {
        QString dialogInfoPath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo" + "/" + QString::number(receiver_id) + ".pb";
        if(!QFile::exists(dialogInfoPath)) {
            logger->log(Logger::INFO, "messagesender.cpp::sendVoiceMessage", "dialogInfo not exists");
            chats::CreateDialogRequest request;
            request.setSenderId(activeUserId);
            request.setReceiverId(receiver_id);

            QString uniqName = QUuid::createUuid().toString(QUuid::WithoutBraces);
            QString baseDir = QCoreApplication::applicationDirPath()
                              + "/.tempData/"
                              + QString::number(receiver_id);

            QDir dir;
            if (!dir.exists(baseDir)) dir.mkpath(baseDir);

            QString filePath = baseDir + "/" + uniqName + ".txt";
            QString metaVoiceFilePath = baseDir + "/" + uniqName + ".meta.v";

            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                QTextStream out(&file);
                out << QString();
                file.close();
            } else {
                logger->log(Logger::ERROR, "messagesender.cpp::sendVoiceMessage", "File not open for save message");
                return;
            }
            QFile metaFile(metaVoiceFilePath);
            if (metaFile.open(QIODevice::WriteOnly)) {
                QTextStream outMeta(&metaFile);
                outMeta << filePath;
                metaFile.close();
            } else {
                logger->log(Logger::ERROR, "messagesender.cpp::sendVoiceMessage", "File not open for saving file path");
                return;
            }

            request.setUniqMessageId(uniqName);

            emit sendMessageData("create_dialog", request.serialize(&serializer));
            return;
        }

        chatMsg.setContent(QString());
        chatMsg.setReceiverId(receiver_id);

        QFile file(filePath);
        QFileInfo fileInfo(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            logger->log(Logger::WARN,"messagesender.cpp::sendVoiceMessage","Failed open file");
        }
        QByteArray encryptedFileData;
        try {
            encryptedFileData = cryptoManager->symmetricEncrypt(file.readAll(),cryptoManager->getDecryptedSessionKey(dialogInfoPath));
        } catch (const std::exception &e) {
            logger->log(Logger::ERROR, "messagesender.cpp::sendVoiceMessage", QString("Error encrypting message: ") + QString(e.what()));
            return;
        }
        file.close();

        chats::FileData fileDataMsg;
        fileDataMsg.setFileName(fileInfo.baseName());
        fileDataMsg.setFileExtension(fileInfo.suffix());
        fileDataMsg.setFileData(encryptedFileData);

        chatMsg.setFile(fileDataMsg);

        emit sendMessageFileData(flag + "_voice_message", chatMsg.serialize(&serializer)); // personal_voice_message group_voice_message
    } else if(flag == "group"){
        chatMsg.setGroupId(receiver_id);

        QFile file(filePath);
        QFileInfo fileInfo(filePath);
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

        emit sendMessageFileData(flag + "_voice_message", chatMsg.serialize(&serializer)); // personal_voice_message group_voice_message
    }

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
}
