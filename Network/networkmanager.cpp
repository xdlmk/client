#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}, fileNetwork(new FileNetworkManager(this)),
    messageNetwork(new MessageNetworkManager(this)) {
    connect(fileNetwork, &FileNetworkManager::onDisconnected, this, &NetworkManager::onDisconnected);
    connect(messageNetwork, &MessageNetworkManager::onDisconnected, this, &NetworkManager::onDisconnected);
    connect(messageNetwork, &MessageNetworkManager::connectionSuccess, this, &NetworkManager::connectionSuccess);

    connect(&reconnectTimer, &QTimer::timeout, this, &NetworkManager::attemptReconnect);
}

FileNetworkManager *NetworkManager::getFileNetwork() {
    return this->fileNetwork;
}

MessageNetworkManager *NetworkManager::getMessageNetwork() {
    return this->messageNetwork;
}

void NetworkManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserId = userId;
    activeUserLogin = userName;
    fileNetwork->setActiveUser(userName,userId);
    messageNetwork->setActiveUser(userName,userId);
}

void NetworkManager::setLogger(Logger *logger)
{
    this->logger = logger;
    fileNetwork->setLogger(logger);
    messageNetwork->setLogger(logger);
}

void NetworkManager::onDisconnected()
{
    if (!reconnectTimer.isActive()) {
        logger->log(Logger::INFO,"networkmanager.cpp::onDisconnected","reconnectTimer starting");
        reconnectTimer.start(2000);
    }
}

void NetworkManager::attemptReconnect()
{
    emit connectionError();
    if(messageNetwork->getSocketState() == QAbstractSocket::UnconnectedState || (activeUserId != 0 ? fileNetwork->getSocketState() == QAbstractSocket::UnconnectedState : true))
    {
        logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","Trying to connect to the server");
        if(activeUserId != 0){
            logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","activeUserId != 0");
            if(messageNetwork->getSocketState() == QAbstractSocket::UnconnectedState) {
                logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","socket UnconnectedState");
                messageNetwork->connectToServer();
            }
            if(fileNetwork->getSocketState() == QAbstractSocket::UnconnectedState) {
                logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","fileSocket UnconnectedState");
                fileNetwork->connectToFileServer();
            }
        } else {
            logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","activeUserId == 0");
            messageNetwork->connectToServer();
        }
    } else if(messageNetwork->getSocketState() == QAbstractSocket::ConnectedState && (activeUserId != 0 ? fileNetwork->getSocketState() == QAbstractSocket::ConnectedState : true)) {
        logger->log(Logger::INFO,"networkmanager.cpp::attemptReconnect","Connected");
        emit connectionSuccess();
        reconnectTimer.stop();
    }
}
