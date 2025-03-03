#include "client.h"

Client::Client(QObject *parent)
    : QObject{parent}
{
    networkManager = new NetworkManager(this);
    messageManager = new MessageManager(this);
    fileManager = new FileManager(this);
    accountManager = new AccountManager(networkManager, this);
    audioManager = new AudioManager(this);

    setupConnections();
}

void Client::setupConnections() {
    setupNetworkConnections();
    setupAccountConnections();
    setupMessageConnections();
    setupFileConnections();
    setupLoggingConnections();
    setupAudioConnections();
}


void Client::setupNetworkConnections() {
    connect(networkManager, &NetworkManager::connectionSuccess, this, &Client::connectionSuccess);
    connect(networkManager, &NetworkManager::connectionError, this, &Client::connectionError);

    connect(networkManager, &NetworkManager::loginResultsReceived, accountManager, &AccountManager::processingLoginResultsFromServer);
    connect(networkManager, &NetworkManager::registrationResultsReceived, accountManager, &AccountManager::processingRegistrationResultsFromServer);
    connect(networkManager, &NetworkManager::messageReceived, messageManager, &MessageManager::savePersonalMessage);
    connect(networkManager, &NetworkManager::groupMessageReceived, messageManager, &MessageManager::saveGroupMessage);
    connect(networkManager, &NetworkManager::searchDataReceived, accountManager, &AccountManager::processingSearchDataFromServer);
    connect(networkManager, &NetworkManager::chatsUpdateDataReceived, messageManager, &MessageManager::saveMessageFromDatabase);
    connect(networkManager, &NetworkManager::editResultsReceived, accountManager, &AccountManager::processingEditProfileFromServer);
    connect(networkManager, &NetworkManager::avatarsUpdateReceived, accountManager, &AccountManager::processingAvatarsUpdateFromServer);
}

void Client::setupAccountConnections() {
    connect(accountManager, &AccountManager::loginSuccess, this, &Client::loginSuccess);
    connect(accountManager, &AccountManager::loginFail, this, &Client::loginFail);
    connect(accountManager, &AccountManager::registrationSuccess, this, &Client::registrationSuccess);
    connect(accountManager, &AccountManager::registrationFail, this, &Client::registrationFail);

    connect(accountManager, &AccountManager::clientLogout, this, &Client::clientLogout);
    connect(this, &Client::clientLogout, accountManager, &AccountManager::logout);

    connect(this, &Client::sendLoginRequest, accountManager, &AccountManager::login);
    connect(this, &Client::sendRegisterRequest, accountManager, &AccountManager::registerAccount);
    connect(this, &Client::sendEditProfileRequest, accountManager, &AccountManager::sendEditProfileRequest);
    connect(this,&Client::sendSearchToServer,accountManager,&AccountManager::sendSearchToServer);
    connect(this,&Client::showContacts,accountManager,&AccountManager::showContacts);
    connect(accountManager, &AccountManager::loadContacts, this, &Client::loadContacts);

    connect(this,&Client::createGroup,accountManager,&AccountManager::createGroup);

    connect(accountManager, &AccountManager::editUserlogin, this, &Client::editUserlogin);
    connect(accountManager, &AccountManager::editPhoneNumber, this, &Client::editPhoneNumber);
    connect(accountManager, &AccountManager::editName, this, &Client::editName);
    connect(accountManager, &AccountManager::editUniqueError, this, &Client::editUniqueError);
    connect(accountManager, &AccountManager::unknownError, this, &Client::unknownError);

    connect(this, &Client::changeActiveAccount, accountManager, &AccountManager::changeActiveAccount);
    connect(this, &Client::configCheck, accountManager, &AccountManager::checkConfigFile);
}

void Client::setupMessageConnections() {
    connect(this, &Client::loadingChat, messageManager, &MessageManager::loadingChat);
    connect(this, &Client::sendMessage, messageManager, &MessageManager::sendMessage);
    connect(this, &Client::sendMessageWithFile, messageManager, &MessageManager::saveMessageAndSendFile);

    connect(messageManager, &MessageManager::sendMessageJson, networkManager, &NetworkManager::sendData);
    connect(messageManager, &MessageManager::sendFile, networkManager, &NetworkManager::sendFile);
    connect(messageManager,&MessageManager::checkAndSendAvatarUpdate,accountManager,&AccountManager::checkAndSendAvatarUpdate);
    connect(messageManager,&MessageManager::getContactList,accountManager,&AccountManager::getContactList);

    connect(accountManager, &AccountManager::checkingChatAvailability, messageManager, &MessageManager::checkingChatAvailability);
    connect(messageManager, &MessageManager::showPersonalChat, this, &Client::showPersonalChat);

    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, messageManager, &MessageManager::setActiveUser);
    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, accountManager, &AccountManager::setActiveUser);
    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, networkManager, &NetworkManager::setActiveUser);
    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, fileManager, &FileManager::setActiveUser);
    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, audioManager, &AudioManager::setActiveUser);

    connect(messageManager, &MessageManager::newMessage, this, &Client::newMessage);
    connect(accountManager, &AccountManager::newSearchUser, this, &Client::newSearchUser);
    connect(messageManager, &MessageManager::clearMainListView, this, &Client::clearMainListView);
    connect(accountManager, &AccountManager::newUser, this, &Client::newUser);
    connect(messageManager, &MessageManager::checkActiveDialog, this, &Client::checkActiveDialog);
}

void Client::setupFileConnections() {
    connect(networkManager, &NetworkManager::sendMessageWithFile, messageManager, &MessageManager::sendMessageWithFile);
    connect(networkManager, &NetworkManager::uploadFiles, fileManager, &FileManager::uploadFiles);
    connect(networkManager, &NetworkManager::uploadVoiceFile, fileManager, &FileManager::uploadVoiceFile);
    connect(networkManager, &NetworkManager::uploadAvatar, fileManager, &FileManager::uploadAvatar);

    connect(this, &Client::getFile, fileManager, &FileManager::getFile);
    connect(fileManager, &FileManager::sendToFileServer, networkManager, &NetworkManager::sendToFileServer);
    connect(messageManager, &MessageManager::sendToFileServer, networkManager, &NetworkManager::sendToFileServer);

    connect(messageManager, &MessageManager::sendAvatarsUpdate, accountManager, &AccountManager::sendAvatarsUpdate);
    connect(accountManager, &AccountManager::sendAvatarUrl, fileManager, &FileManager::sendAvatarUrl);
    connect(networkManager, &NetworkManager::sendAvatarUrl, fileManager, &FileManager::sendAvatarUrl);
    connect(this, &Client::sendNewAvatar, networkManager, &NetworkManager::sendAvatar);
}

void Client::setupLoggingConnections() {
    connect(this, &Client::setLoggers, this, &Client::setLogger);
    connect(this, &Client::setLoggers, accountManager, &AccountManager::setLogger);
    connect(this, &Client::setLoggers, messageManager, &MessageManager::setLogger);
    connect(this, &Client::setLoggers, networkManager, &NetworkManager::setLogger);
    connect(this, &Client::setLoggers, fileManager, &FileManager::setLogger);
    connect(this, &Client::setLoggers, audioManager, &AudioManager::setLogger);
}

void Client::setupAudioConnections() {
    connect(this, &Client::sendVoiceMessage, messageManager, &MessageManager::sendVoiceMessage);
    connect(fileManager, &FileManager::voiceExists, this, &Client::voiceExists);
    connect(this, &Client::startRecording, audioManager, &AudioManager::startRecording);
    connect(this, &Client::stopRecording, audioManager, &AudioManager::stopRecording);
}

AccountManager* Client::getAccountManager() {
    return accountManager;
}

FileManager *Client::getFileManager()
{
    return fileManager;
}

void Client::setLogger(Logger *logger)
{
    this->logger = logger;
}
