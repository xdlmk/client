#include "client.h"

Client::Client(QObject *parent)
    : QObject{parent}
{
    networkManager = new NetworkManager(this);
    messageManager = new MessageManager(this);
    accountManager = new AccountManager(networkManager, this);

    /* socket = new QTcpSocket(this);
    QObject::connect(socket, &QTcpSocket::connected, [&]() {
        qInfo() <<  "Success connect to server on port 2020";
        reconnectTimer.stop();
        emit connectionSuccess();
    });
    QObject::connect(socket, &QAbstractSocket::errorOccurred, [&](QAbstractSocket::SocketError socketError) {
        if (!reconnectTimer.isActive()) {
            reconnectTimer.start(2000);
        }
        qDebug() << "Connection error: " << socket->errorString();
    });
    qDebug() << "Client constructor";
    connectToServer();
    qDebug() << "Client constructor1";
    connect(socket,&QTcpSocket::readyRead,this,&Client::slotReadyRead);
    connect(socket,&QTcpSocket::disconnected,this,&Client::onDisconnected);
    connect(&reconnectTimer, &QTimer::timeout, this, &Client::attemptReconnect);
*/
    connect(networkManager,&NetworkManager::connectionSuccess,this,&Client::connectionSuccess);
    connect(networkManager,&NetworkManager::connectionError,this,&Client::connectionError);

    connect(networkManager,&NetworkManager::loginResultsReceived,accountManager,&AccountManager::processingLoginResultsFromServer);
    connect(networkManager,&NetworkManager::registrationResultsReceived,accountManager,&AccountManager::processingRegistrationResultsFromServer);
    connect(networkManager,&NetworkManager::messageReceived,accountManager,&AccountManager::processingPersonalMessageFromServer);
    connect(networkManager,&NetworkManager::searchDataReceived,accountManager,&AccountManager::processingSearchDataFromServer);
    connect(networkManager,&NetworkManager::chatsUpdateDataReceived,accountManager,&AccountManager::processingChatsUpdateDataFromServer);

    connect(accountManager,&AccountManager::loginSuccess,this,&Client::loginSuccess);
    connect(accountManager,&AccountManager::loginFail,this,&Client::loginFail);
    connect(accountManager,&AccountManager::registrationSuccess,this,&Client::registrationSuccess);
    connect(accountManager,&AccountManager::registrationFail,this,&Client::registrationFail);

    connect(accountManager,&AccountManager::clientLogout,this,&Client::clientLogout);

    connect(this,&Client::loadingPersonalChat,messageManager,&MessageManager::loadingPersonalChat);
    connect(this,&Client::sendPersonalMessage,messageManager,&MessageManager::sendPersonalMessage);

    connect(messageManager,&MessageManager::sendMessageJson,networkManager,&NetworkManager::sendData);

    connect(accountManager,&AccountManager::saveMessageToJson,messageManager,&MessageManager::saveMessageToJson);

    connect(accountManager,&AccountManager::checkingChatAvailability,messageManager,&MessageManager::checkingChatAvailability);
    connect(messageManager,&MessageManager::showPersonalChat,this,&Client::showPersonalChat);
    connect(accountManager,&AccountManager::saveMessageFromDatabase,messageManager,&MessageManager::saveMessageFromDatabase);

    //connect(networkManager,&NetworkManager::messageReceived,messageManager,&MessageManager::loadMessageToQml);
    connect(accountManager,&AccountManager::newAccountLoginSuccessful,messageManager,&MessageManager::loadMessagesFromJson);

    connect(accountManager,&AccountManager::transferUserNameAndIdAfterLogin,messageManager,&MessageManager::setActiveUser);
    connect(accountManager,&AccountManager::transferUserNameAndIdAfterLogin,accountManager,&AccountManager::setActiveUser);

    connect(messageManager,&MessageManager::newOutMessage,this,&Client::newOutMessage);
    connect(messageManager,&MessageManager::newInMessage,this,&Client::newInMessage);

    connect(accountManager,&AccountManager::newSearchUser,this,&Client::newSearchUser);

    connect(messageManager,&MessageManager::clearMainListView,this,&Client::clearMainListView);

    connect(accountManager,&AccountManager::newUser,this,&Client::newUser);

    connect(accountManager,&AccountManager::checkActiveDialog,this,&Client::checkActiveDialog);

    connect(this,&Client::changeActiveAccount,accountManager,&AccountManager::changeActiveAccount);
}

QString Client::messageFrom()
{
    return mesFrom;
}

void Client::setMessageFrom(QString value)
{
    mesFrom = value;
}

AccountManager* Client::getAccountManager() {
    return accountManager;
}

NetworkManager* Client::getNetworkManager() {
    return networkManager;
}

MessageManager* Client::getMessageManager() {
    return messageManager;
}

void Client::sendLoginRequest(QString &userlogin, QString &password)
{
    accountManager->login(userlogin,password);
}

void Client::sendSearchToServer(const QString &searchable)
{
    accountManager->sendSearchToServer(searchable);
}

void Client::sendToServer(QString str,QString userLogin)
{
    QJsonObject json;
    json["flag"] = "message";
    json["str"] = str;
    json["login"] = userLogin;
    QJsonDocument doc(json);
    data.clear();
    QDataStream out(&data,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_7);
    out << doc.toJson();
    socket->write(data);
}
