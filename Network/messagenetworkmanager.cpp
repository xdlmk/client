#include "messagenetworkmanager.h"

MessageNetworkManager::MessageNetworkManager(QObject *parent)
    : QObject{parent}
{
    socket = new QTcpSocket(this);

    activeUserId = 0;
    activeUserLogin = "";

    QObject::connect(socket, &QTcpSocket::connected, [this]() {
        logger->log(Logger::INFO,"messagenetworkmanager.cpp::constructor","Connection to the MessageServer established");
        emit connectionSuccess();

        if(activeUserId != 0) {
            common::Identifiers ident;
            ident.setUserId(activeUserId);
            QProtobufSerializer serializer;
            sendData("identifiers", ident.serialize(&serializer));
        }
        {
            QMutexLocker lock(&messageMutex);
            if (!sendMessageQueue.isEmpty()) {
                QMetaObject::invokeMethod(this, "processSendMessageQueue", Qt::QueuedConnection);
            }
        }
    });

    connect(socket,&QTcpSocket::readyRead,this,&MessageNetworkManager::onDataReceived);
    connect(socket,&QTcpSocket::disconnected,this,&MessageNetworkManager::onDisconnected);
    connect(socket,&QTcpSocket::bytesWritten,this,&MessageNetworkManager::handleMessageBytesWritten);
    connect(socket, &QAbstractSocket::errorOccurred, [this](QAbstractSocket::SocketError socketError) {
        emit onDisconnected();
        logger->log(Logger::ERROR,"messagenetworkmanager.cpp::constructor","Connection to the FileServer error: " + socket->errorString());
    });

    connectToServer();
}

QAbstractSocket::SocketState MessageNetworkManager::getSocketState() const
{
    return socket->state();
}

void MessageNetworkManager::connectToServer()
{
    QFile file("ip.txt");
    if (!file.exists() || file.size() == 0) {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write("127.0.0.1\n");
        file.close();
    }
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString ip = QString::fromUtf8(file.readLine()).trimmed();
    file.close();
    socket->connectToHost(ip,2020);
}

void MessageNetworkManager::sendData(const QString &flag, const QByteArray &data)
{
    messages::Envelope envelope;
    envelope.setFlag(flag);
    envelope.setPayload(data);
    QProtobufSerializer serializer;
    QByteArray envelopeData = envelope.serialize(&serializer);

    logger->log(
        Logger::INFO,
        "messagenetworkmanager.cpp::sendData",
        "Sending envelope for flag: " + flag
        );

    bool shouldStartProcessing = false;
    {
        QMutexLocker lock(&messageMutex);
        if (sendMessageQueue.size() >= MAX_QUEUE_SIZE) {
            logger->log(
                Logger::DEBUG,
                "messagenetworkmanager.cpp::sendData",
                "Send queue overflow! Dropping message."
                );
            return;
        }
        sendMessageQueue.enqueue(envelopeData);
        logger->log(
            Logger::INFO,
            "messagenetworkmanager.cpp::sendData",
            "Message added to queue. Queue size: " + QString::number(sendMessageQueue.size())
            );
        shouldStartProcessing = sendMessageQueue.size() == 1;
    }

    if (shouldStartProcessing) {
        logger->log(
            Logger::INFO,
            "messagenetworkmanager.cpp::sendData",
            "Starting to process the send queue."
            );
        processSendMessageQueue();
    }
}

void MessageNetworkManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserId = userId;
    activeUserLogin = userName;
}

void MessageNetworkManager::setLogger(Logger *logger)
{
    this->logger = logger;
}

void MessageNetworkManager::onDataReceived()
{
    QDataStream in(socket);
    logger->log(Logger::INFO,"messagenetworkmanager.cpp::onDataReceived","Data packets received");
    in.setVersion(QDataStream::Qt_6_7);

    static quint32 blockSize = 0;

    if(in.status() != QDataStream::Ok)
    {
        logger->log(Logger::WARN,"messagenetworkmanager.cpp::onDataReceived","QDataStream status error");
        return;
    }

    while (true) {
        if (blockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint32))
                return;
            in >> blockSize;
        }

        if (socket->bytesAvailable() < blockSize) return;

        QByteArray envelopeData;
        envelopeData.resize(blockSize);
        in.readRawData(envelopeData.data(), blockSize);

        QProtobufSerializer serializer;
        messages::Envelope envelope;
        if (!envelope.deserialize(&serializer, envelopeData)) {
            logger->log(Logger::WARN, "clienthandler.cpp::readClient", "Failed to deserialize protobuf");
            blockSize = 0;
            return;
        }

        QString flag = envelope.flag();
        QByteArray payload = envelope.payload();

        logger->log(Logger::INFO,"messagenetworkmanager.cpp::onDataReceived","Readings PROTO for " + flag);

        auto it = flagMap.find(flag.toStdString());
        uint flagId = (it != flagMap.end()) ? it->second : 0;
        switch (flagId) {
        case 1:
            emit loginResultsReceived(payload);
            break;
        case 2:
            emit registrationResultsReceived(payload);
            break;
        case 3:
            emit messageReceived(payload);
            break;
        case 4:
            emit groupMessageReceived(payload);
            break;
        case 5:
            emit deleteGroupMemberReceived(payload);
            break;
        case 6:
            emit addGroupMemberReceived(payload);
            break;
        case 7:{
            chats::ChatsInfoResponse response;
            QProtobufSerializer serializer;
            response.deserialize(&serializer,payload);

            emit dialogsInfoReceived(response.dialogsInfo());
            emit groupInfoReceived(response.groupsInfo());
            break;
        }
        case 8:
            emit searchDataReceived(payload);
            break;
        case 9:
            emit chatsUpdateDataReceived(payload);
            break;
        case 10:
            emit loadMeassgesReceived(payload);
            break;
        case 11:
            emit editResultsReceived(payload);
            break;
        case 12:
            emit avatarsUpdateReceived(payload);
            break;
        default:
            logger->log(Logger::WARN, "messagenetworkmanager.cpp::onDataReceived", "Unknown flag received: " + flag);
            break;
        }
        blockSize = 0;
    }
    logger->log(Logger::INFO,"messagenetworkmanager.cpp::onDataReceived","Leave onDataReceived");
}

void MessageNetworkManager::handleMessageBytesWritten(qint64 bytes)
{
    QMutexLocker lock(&messageMutex);

    logger->log(Logger::INFO, "messagenetworkmanager.cpp::handleMessageBytesWritten", "Bytes written: " + QString::number(bytes));

    if (!sendMessageQueue.isEmpty()) {
        sendMessageQueue.dequeue();
        logger->log(Logger::INFO, "messagenetworkmanager.cpp::handleMessageBytesWritten", "Message dequeued. Queue size: " + QString::number(sendMessageQueue.size()));
        if (!sendMessageQueue.isEmpty()) {
            logger->log(Logger::INFO, "messagenetworkmanager.cpp::handleMessageBytesWritten", "More messages in queue. Scheduling next processing.");
            QTimer::singleShot(10, this, &MessageNetworkManager::processSendMessageQueue);
        }
    }
}

void MessageNetworkManager::processSendMessageQueue()
{
    QMutexLocker lock(&messageMutex);
    if (sendMessageQueue.isEmpty()) {
        logger->log(Logger::DEBUG, "messagenetworkmanager.cpp::processSendMessageQueue", "Send queue is empty. Nothing to process.");
        return;
    }

    QByteArray jsonData = sendMessageQueue.head();
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);

    out << quint32(jsonData.size());
    out.writeRawData(jsonData.data(), jsonData.size());
    logger->log(Logger::INFO, "messagenetworkmanager.cpp::processSendMessageQueue", "Preparing to send data. Data size: " + QString::number(jsonData.size()) + " bytes");

    if (socket->state() != QAbstractSocket::ConnectedState) {
        logger->log(Logger::DEBUG, "messagenetworkmanager.cpp::processSendMessageQueue", "Socket is not connected. Cannot send data.");
        return;
    }
    if (socket->write(bytes) == -1) {
        logger->log(Logger::DEBUG, "messagenetworkmanager.cpp::processSendMessageQueue", "Failed to write data: " + socket->errorString());
        return;
    }
    logger->log(Logger::INFO, "messagenetworkmanager.cpp::processSendMessageQueue", "Data written to socket. Data size: " + QString::number(bytes.size()) + " bytes");
}

const std::unordered_map<std::string_view, uint> MessageNetworkManager::flagMap = {
    {"login", 1}, {"reg", 2}, {"personal_message", 3},
    {"group_message", 4}, {"delete_member", 5},
    {"add_group_members", 6}, {"chats_info", 7},
    {"search", 8}, {"updating_chats", 9},
    {"load_messages", 10}, {"edit", 11},
    {"avatars_update", 12}
};
