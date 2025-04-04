#include "client.h"

Client::Client(QObject *parent)
    : QObject{parent}
{
    networkManager = new NetworkManager(this);
    messageHandler = new MessageHandler(this);
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
    connect(networkManager, &NetworkManager::messageReceived, messageHandler, &MessageHandler::processingPersonalMessage);
    connect(networkManager, &NetworkManager::groupMessageReceived, messageHandler, &MessageHandler::processingGroupMessage);
    connect(networkManager, &NetworkManager::searchDataReceived, accountManager, &AccountManager::processingSearchDataFromServer);
    connect(networkManager, &NetworkManager::chatsUpdateDataReceived, messageHandler, &MessageHandler::updatingLatestMessagesFromServer);
    connect(networkManager, &NetworkManager::loadMeassgesReceived, messageHandler, &MessageHandler::loadingNextMessages);
    connect(networkManager, &NetworkManager::editResultsReceived, accountManager, &AccountManager::processingEditProfileFromServer);
    connect(networkManager, &NetworkManager::avatarsUpdateReceived, accountManager, &AccountManager::processingAvatarsUpdateFromServer);
    connect(networkManager, &NetworkManager::groupInfoReceived, accountManager, &AccountManager::processingGroupInfoSave);
    connect(networkManager, &NetworkManager::dialogsInfoReceived, accountManager, &AccountManager::processingDialogsInfoSave);
    connect(networkManager, &NetworkManager::deleteGroupMemberReceived, accountManager, &AccountManager::processingDeleteGroupMember);
    connect(networkManager, &NetworkManager::addGroupMemberReceived, accountManager, &AccountManager::processingAddGroupMember);

    connect(networkManager, &NetworkManager::removeAccountFromConfigManager, accountManager, &AccountManager::removeAccountFromConfigManager);
}

void Client::setupAccountConnections() {
    connect(accountManager, &AccountManager::loginSuccess, this, &Client::loginSuccess);
    connect(accountManager, &AccountManager::loginFail, this, &Client::loginFail);
    connect(accountManager, &AccountManager::registrationSuccess, this, &Client::registrationSuccess);
    connect(accountManager, &AccountManager::registrationFail, this, &Client::registrationFail);

    connect(accountManager, &AccountManager::clientLogout, this, &Client::clientLogout);
    connect(this, &Client::clientLogout, accountManager, &AccountManager::logout);
    connect(this, &Client::checkAndSendAvatarUpdate, accountManager, &AccountManager::checkAndSendAvatarUpdate);

    connect(this, &Client::sendLoginRequest, accountManager, &AccountManager::login);
    connect(this, &Client::sendRegisterRequest, accountManager, &AccountManager::registerAccount);
    connect(this, &Client::sendEditProfileRequest, accountManager, &AccountManager::sendEditProfileRequest);
    connect(this,&Client::sendSearchToServer,accountManager,&AccountManager::sendSearchToServer);
    connect(this,&Client::showContacts,accountManager,&AccountManager::showContacts);
    connect(accountManager, &AccountManager::loadContacts, this, &Client::loadContacts);
    connect(accountManager, &AccountManager::loadGroupMembers, this, &Client::loadGroupMembers);
    connect(accountManager, &AccountManager::clearMessagesAfterDelete, this, &Client::clearMessagesAfterDelete);


    connect(this,&Client::createGroup,accountManager,&AccountManager::createGroup);
    connect(this,&Client::addGroupMembers,accountManager,&AccountManager::addGroupMembers);
    connect(this,&Client::getGroupMembers,accountManager,&AccountManager::getGroupMembers);
    connect(this, &Client::deleteMemberFromGroup, accountManager, &AccountManager::deleteMemberFromGroup);

    connect(accountManager, &AccountManager::editUserlogin, this, &Client::editUserlogin);
    connect(accountManager, &AccountManager::editPhoneNumber, this, &Client::editPhoneNumber);
    connect(accountManager, &AccountManager::editName, this, &Client::editName);
    connect(accountManager, &AccountManager::editUniqueError, this, &Client::editUniqueError);
    connect(accountManager, &AccountManager::unknownError, this, &Client::unknownError);

    connect(this, &Client::changeActiveAccount, accountManager, &AccountManager::changeActiveAccount);
    connect(this, &Client::configCheck, accountManager, &AccountManager::checkConfigFile);
}

void Client::setupMessageConnections() {
    connect(this, &Client::loadingChat, messageHandler, &MessageHandler::loadingChat);
    connect(this, &Client::sendMessage, messageHandler, &MessageHandler::sendMessage);
    connect(this, &Client::requestMessageDownload, messageHandler, &MessageHandler::sendRequestMessagesLoading);
    connect(this, &Client::sendMessageWithFile, messageHandler, &MessageHandler::saveMessageAndSendFile);

    connect(messageHandler, &MessageHandler::sendMessageJson, networkManager, &NetworkManager::sendData);
    connect(messageHandler, &MessageHandler::sendFile, networkManager, &NetworkManager::sendFile);
    connect(messageHandler,&MessageHandler::checkAndSendAvatarUpdate,accountManager,&AccountManager::checkAndSendAvatarUpdate);
    connect(messageHandler,&MessageHandler::getContactList,accountManager,&AccountManager::getContactList);
    connect(messageHandler,&MessageHandler::getChatsInfo,accountManager,&AccountManager::getChatsInfo);

    connect(messageHandler,&MessageHandler::removeAccountFromConfigManager,accountManager,&AccountManager::removeAccountFromConfigManager);

    connect(accountManager, &AccountManager::checkingChatAvailability, messageHandler, &MessageHandler::checkingChatAvailability);

    connect(messageHandler, &MessageHandler::showPersonalChat, this, &Client::showPersonalChat);

    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, messageHandler, &MessageHandler::setActiveUser);
    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, accountManager, &AccountManager::setActiveUser);
    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, networkManager, &NetworkManager::setActiveUser);
    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, fileManager, &FileManager::setActiveUser);
    connect(accountManager, &AccountManager::transferUserNameAndIdAfterLogin, audioManager, &AudioManager::setActiveUser);

    connect(messageHandler, &MessageHandler::newMessage, this, &Client::newMessage);
    connect(accountManager, &AccountManager::newSearchUser, this, &Client::newSearchUser);
    connect(messageHandler, &MessageHandler::clearMainListView, this, &Client::clearMainListView);
    connect(accountManager, &AccountManager::newUser, this, &Client::newUser);
    connect(messageHandler, &MessageHandler::checkActiveDialog, this, &Client::checkActiveDialog);
    connect(messageHandler, &MessageHandler::returnChatToPosition, this, &Client::returnChatToPosition);
    connect(messageHandler, &MessageHandler::insertMessage, this, &Client::insertMessage);
}

void Client::setupFileConnections() {
    connect(networkManager, &NetworkManager::sendMessageWithFile, messageHandler, &MessageHandler::sendMessageWithFile);
    connect(networkManager, &NetworkManager::uploadFiles, fileManager, &FileManager::uploadFiles);
    connect(networkManager, &NetworkManager::uploadVoiceFile, fileManager, &FileManager::uploadVoiceFile);
    connect(networkManager, &NetworkManager::uploadAvatar, fileManager, &FileManager::uploadAvatar);

    connect(this, &Client::getFile, fileManager, &FileManager::getFile);
    connect(fileManager, &FileManager::sendToFileServer, networkManager, &NetworkManager::sendToFileServer);
    connect(messageHandler, &MessageHandler::sendToFileServer, networkManager, &NetworkManager::sendToFileServer);

    connect(messageHandler, &MessageHandler::sendAvatarsUpdate, accountManager, &AccountManager::sendAvatarsUpdate);
    connect(accountManager, &AccountManager::sendAvatarUrl, fileManager, &FileManager::sendAvatarUrl);
    connect(networkManager, &NetworkManager::sendAvatarUrl, fileManager, &FileManager::sendAvatarUrl);
    connect(this, &Client::sendNewAvatar, networkManager, &NetworkManager::sendAvatar);
}

void Client::setupLoggingConnections() {
    connect(this, &Client::setLoggers, this, &Client::setLogger);
    connect(this, &Client::setLoggers, accountManager, &AccountManager::setLogger);
    connect(this, &Client::setLoggers, messageHandler, &MessageHandler::setLogger);
    connect(this, &Client::setLoggers, networkManager, &NetworkManager::setLogger);
    connect(this, &Client::setLoggers, fileManager, &FileManager::setLogger);
    connect(this, &Client::setLoggers, audioManager, &AudioManager::setLogger);
}

void Client::setupAudioConnections() {
    connect(this, &Client::sendVoiceMessage, messageHandler, &MessageHandler::sendVoiceMessage);
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
