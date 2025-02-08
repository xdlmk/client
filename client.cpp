#include "client.h"

Client::Client(QObject *parent)
    : QObject{parent}
{
    networkManager = new NetworkManager(this);
    messageManager = new MessageManager(this);
    accountManager = new AccountManager(networkManager, this);

    connect(networkManager,&NetworkManager::connectionSuccess,this,&Client::connectionSuccess);
    connect(networkManager,&NetworkManager::connectionError,this,&Client::connectionError);

    connect(networkManager,&NetworkManager::loginResultsReceived,accountManager,&AccountManager::processingLoginResultsFromServer);
    connect(networkManager,&NetworkManager::registrationResultsReceived,accountManager,&AccountManager::processingRegistrationResultsFromServer);
    connect(networkManager,&NetworkManager::messageReceived,accountManager,&AccountManager::processingPersonalMessageFromServer);
    connect(networkManager,&NetworkManager::searchDataReceived,accountManager,&AccountManager::processingSearchDataFromServer);
    connect(networkManager,&NetworkManager::chatsUpdateDataReceived,accountManager,&AccountManager::processingChatsUpdateDataFromServer);
    connect(networkManager,&NetworkManager::editResultsReceived,accountManager,&AccountManager::processingEditProfileFromServer);

    connect(accountManager,&AccountManager::loginSuccess,this,&Client::loginSuccess);
    connect(accountManager,&AccountManager::loginFail,this,&Client::loginFail);
    connect(accountManager,&AccountManager::registrationSuccess,this,&Client::registrationSuccess);
    connect(accountManager,&AccountManager::registrationFail,this,&Client::registrationFail);

    connect(accountManager,&AccountManager::clientLogout,this,&Client::clientLogout);
    connect(this,&Client::clientLogout,accountManager,&AccountManager::logout);

    connect(this,&Client::loadingPersonalChat,messageManager,&MessageManager::loadingPersonalChat);

    connect(this,&Client::sendPersonalMessage,messageManager,&MessageManager::sendPersonalMessage);
    connect(this,&Client::sendSearchToServer,accountManager,&AccountManager::sendSearchToServer);
    connect(this,&Client::sendLoginRequest,accountManager,&AccountManager::login);
    connect(this,&Client::sendRegisterRequest,accountManager,&AccountManager::registerAccount);
    connect(this,&Client::sendEditProfileRequest,accountManager,&AccountManager::sendEditProfileRequest);

    connect(messageManager,&MessageManager::sendMessageJson,networkManager,&NetworkManager::sendData);

    connect(accountManager,&AccountManager::saveMessageToJson,messageManager,&MessageManager::saveMessageToJson);

    connect(accountManager,&AccountManager::checkingChatAvailability,messageManager,&MessageManager::checkingChatAvailability);
    connect(messageManager,&MessageManager::showPersonalChat,this,&Client::showPersonalChat);
    connect(accountManager,&AccountManager::saveMessageFromDatabase,messageManager,&MessageManager::saveMessageFromDatabase);

    connect(accountManager,&AccountManager::editUserlogin,this,&Client::editUserlogin);
    connect(accountManager,&AccountManager::editPhoneNumber,this,&Client::editPhoneNumber);
    connect(accountManager,&AccountManager::editName,this,&Client::editName);
    connect(accountManager,&AccountManager::editUniqueError,this,&Client::editUniqueError);
    connect(accountManager,&AccountManager::unknownError,this,&Client::unknownError);

    connect(accountManager,&AccountManager::newAccountLoginSuccessful,messageManager,&MessageManager::loadMessagesFromJson);

    connect(accountManager,&AccountManager::transferUserNameAndIdAfterLogin,messageManager,&MessageManager::setActiveUser);
    connect(accountManager,&AccountManager::transferUserNameAndIdAfterLogin,accountManager,&AccountManager::setActiveUser);

    connect(messageManager,&MessageManager::newMessage,this,&Client::newMessage);

    connect(accountManager,&AccountManager::newSearchUser,this,&Client::newSearchUser);

    connect(messageManager,&MessageManager::clearMainListView,this,&Client::clearMainListView);

    connect(accountManager,&AccountManager::newUser,this,&Client::newUser);
    connect(this,&Client::configCheck,accountManager,&AccountManager::checkConfigFile);

    connect(accountManager,&AccountManager::checkActiveDialog,this,&Client::checkActiveDialog);

    connect(this,&Client::changeActiveAccount,accountManager,&AccountManager::changeActiveAccount);

    connect(this,&Client::setLoggers,this,&Client::setLogger);
    connect(this,&Client::setLoggers,accountManager,&AccountManager::setLogger);
    connect(this,&Client::setLoggers,messageManager,&MessageManager::setLogger);
    connect(this,&Client::setLoggers,networkManager,&NetworkManager::setLogger);
}

AccountManager* Client::getAccountManager() {
    return accountManager;
}

void Client::setLogger(Logger *logger)
{
    this->logger = logger;
}
