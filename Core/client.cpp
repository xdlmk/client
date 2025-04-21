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

    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::loginResultsReceived, accountManager, &AccountManager::processingLoginResultsFromServer);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::registrationResultsReceived, accountManager, &AccountManager::processingRegistrationResultsFromServer);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::messageReceived, messageHandler, &MessageHandler::processingPersonalMessage);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::groupMessageReceived, messageHandler, &MessageHandler::processingGroupMessage);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::searchDataReceived, accountManager, &AccountManager::processingSearchDataFromServer);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::chatsUpdateDataReceived, messageHandler, &MessageHandler::updatingLatestMessagesFromServer);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::loadMeassgesReceived, messageHandler, &MessageHandler::loadingNextMessages);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::editResultsReceived, accountManager, &AccountManager::processingEditProfileFromServer);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::avatarsUpdateReceived, accountManager, &AccountManager::processingAvatarsUpdateFromServer);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::groupInfoReceived, accountManager, &AccountManager::processingGroupInfoSave);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::dialogsInfoReceived, accountManager, &AccountManager::processingDialogsInfoSave);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::deleteGroupMemberReceived, accountManager, &AccountManager::processingDeleteGroupMember);
    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::addGroupMemberReceived, accountManager, &AccountManager::processingAddGroupMember);

    connect(networkManager->getMessageNetwork(), &MessageNetworkManager::removeAccountFromConfigManager, accountManager, &AccountManager::removeAccountFromConfigManager);
}

void Client::setupAccountConnections() {
    connect(accountManager, &AccountManager::loginSuccess, this, &Client::loginSuccess);
    connect(accountManager, &AccountManager::loginSuccess,networkManager->getFileNetwork(),&FileNetworkManager::connectToFileServer);
    connect(accountManager, &AccountManager::loginFail, this, &Client::loginFail);
    connect(accountManager, &AccountManager::registrationSuccess, this, &Client::registrationSuccess);
    connect(accountManager, &AccountManager::registrationFail, this, &Client::registrationFail);

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
    connect(this, &Client::sendMessageWithFile, messageHandler, &MessageHandler::sendMessageWithFile);

    connect(messageHandler, &MessageHandler::sendMessageJson, networkManager->getMessageNetwork(), &MessageNetworkManager::sendDataJson); // remove
    connect(messageHandler,&MessageHandler::sendMessageData, networkManager->getMessageNetwork(), &MessageNetworkManager::sendData);
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
    connect(networkManager->getFileNetwork(), &FileNetworkManager::uploadFiles, fileManager, &FileManager::uploadFiles);
    connect(networkManager->getFileNetwork(), &FileNetworkManager::uploadVoiceFile, fileManager, &FileManager::uploadVoiceFile);
    connect(networkManager->getFileNetwork(), &FileNetworkManager::uploadAvatar, fileManager, &FileManager::uploadAvatar);

    connect(this, &Client::getFile, fileManager, &FileManager::getFile);
    connect(fileManager, &FileManager::sendToFileServer, networkManager->getFileNetwork(), &FileNetworkManager::sendToFileServer);
    connect(fileManager, &FileManager::sendDataFile, networkManager->getFileNetwork(), &FileNetworkManager::sendData);
    connect(messageHandler, &MessageHandler::sendToFileServer, networkManager->getFileNetwork(), &FileNetworkManager::sendToFileServer);

    connect(messageHandler, &MessageHandler::sendAvatarsUpdate, accountManager, &AccountManager::sendAvatarsUpdate);
    connect(accountManager, &AccountManager::sendAvatarUrl, fileManager, &FileManager::sendAvatarUrl);
    connect(networkManager->getFileNetwork(), &FileNetworkManager::sendAvatarUrl, fileManager, &FileManager::sendAvatarUrl);
    connect(this, &Client::sendNewAvatar, networkManager->getFileNetwork(), &FileNetworkManager::sendAvatar);
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
