#include "accountmanager.h"

AccountManager::AccountManager(NetworkManager* networkManager,QObject *parent)
    : QObject{parent}
{
    this->networkManager = networkManager;
    avatarGenerator = new AvatarGenerator(this);
    setupConfigManager();
    setupResponseHandler();
}

void AccountManager::login(const QString login, const QString password)
{
    QProtobufSerializer serializer;
    QByteArray data;

    auth::LoginRequest loginRequest;
    loginRequest.setLogin(login);
    loginRequest.setPassword(password);
    cryptoManager->setLastEnteredPassword(password);

    networkManager->getMessageNetwork()->sendData("login", loginRequest.serialize(&serializer));
}

void AccountManager::registerAccount(const QString login, const QString password)
{
    QProtobufSerializer serializer;
    QByteArray data;

    auth::RegisterRequest regRequest;
    regRequest.setLogin(login);
    regRequest.setPassword(password);
    try {
        CryptoKeys keys = cryptoManager->generateKeys(password);
        regRequest.setPublicKey(keys.publicKey);
        regRequest.setEncryptedPrivateKey(keys.encryptedPrivateKey);
        regRequest.setSalt(keys.salt);
        regRequest.setNonce(keys.nonce);
    } catch (const std::exception &e) {
        logger->log(Logger::ERROR, "accountmanager.cpp::registerAccount", "Error generate keys: " + QString(e.what()));
        return;
    }

    networkManager->getMessageNetwork()->sendData("reg", regRequest.serialize(&serializer));
}

void AccountManager::logout()
{
    logger->log(Logger::INFO,"accountmanager.cpp::logout","Client logout process has begun");
    configManager.removeAccount(configManager.getActiveAccount());
    networkManager->getMessageNetwork()->sendData("logout",QByteArray());
}

void AccountManager::sendSearchToServer(const QString &searchable)
{
    search::SearchRequest request;
    request.setSearchable(searchable);
    QProtobufSerializer serializer;
    networkManager->getMessageNetwork()->sendData("search", request.serialize(&serializer));
}

void AccountManager::sendEditProfileRequest(const QString editable, const QString editInformation)
{
    profile::EditProfileRequest request;
    request.setUserId(activeUserId);
    request.setEditable(editable);
    request.setEditInformation(editInformation);
    QProtobufSerializer serializer;
    networkManager->getMessageNetwork()->sendData("edit", request.serialize(&serializer));
}

void AccountManager::clientChangeAccount()
{
    networkManager->getMessageNetwork()->sendData("logout",QByteArray());
}

void AccountManager::setCryptoManager(CryptoManager *cryptoManager)
{
    this->cryptoManager = cryptoManager;
}

void AccountManager::checkAndSendAvatarUpdate(const QString &avatar_url, const int &user_id,const QString& type)
{
    logger->log(Logger::INFO,"accountmanager.cpp::checkAndSendAvatarUpdate","checkAndSendAvatarUpdate starts");
    if(!isAvatarUpToDate(avatar_url, user_id, type)) {
        emit sendAvatarUrl(avatar_url, user_id, type);
    }

}

void AccountManager::sendAvatarsUpdate()
{
    logger->log(Logger::INFO,"accountmanager.cpp::sendAvatarsUpdate","sendAvatarsUpdate starts");
    avatars::AvatarsUpdateRequest request;
    request.setUserId(activeUserId);
    QProtobufSerializer serializer;
    networkManager->getMessageNetwork()->sendData("avatars_update", request.serialize(&serializer));
}

void AccountManager::setActiveUser(const QString &user_login, const int &user_id)
{
    activeUserLogin = user_login;
    activeUserId = user_id;
    responseHandler.setActiveUser(user_login,user_id);
    avatarGenerator->setActiveUser(user_login,user_id);
}

void AccountManager::setLogger(Logger *logger)
{
    this->logger = logger;
    configManager.setLogger(logger);
    responseHandler.setLogger(logger);
}

void AccountManager::createGroup(const QString &groupName, const QString& avatarPath, const QVariantList &selectedContacts)
{
    groups::CreateGroupRequest request;
    request.setGroupName(groupName);
    request.setCreatorId(activeUserId);
    request.setMembers(convertContactToList(selectedContacts));

    QProtobufSerializer serializer;

    if(avatarPath == ""){
        request.setAvatarUrl("basic");
        networkManager->getMessageNetwork()->sendData("create_group", request.serialize(&serializer));
    } else {
        QFile file(avatarPath);
        QFileInfo fileInfo(avatarPath);
        if (!file.open(QIODevice::ReadOnly)) {
            logger->log(Logger::WARN,"networkmanager.cpp::sendAvatar","Failed open avatar");
        }
        QByteArray fileData = file.readAll();
        file.close();
        request.setFileName(fileInfo.baseName());
        request.setFileExtension(fileInfo.suffix());
        request.setFileData(fileData);
        networkManager->getFileNetwork()->sendData("create_group", request.serialize(&serializer));
    }
}

void AccountManager::addGroupMembers(const int &group_id, const QVariantList &selectedContacts)
{
    groups::AddGroupMembersRequest request;
    request.setGroupId(group_id);
    request.setAdminId(this->activeUserId);
    request.setMembers(convertContactsToProto(selectedContacts));

    QProtobufSerializer serializer;
    networkManager->getMessageNetwork()->sendData("add_group_members", request.serialize(&serializer));
}

void AccountManager::createDialogKeys(const QByteArray &createDialogKeysData)
{
    QProtobufSerializer serializer;
    chats::CreateDialogResponse response;

    if (!response.deserialize(&serializer, createDialogKeysData)) {
        logger->log(Logger::INFO, "chatmanager.cpp::createDialogKeys", "Error deserialize request");
        return;
    }

    QByteArray sender_public_key = response.senderPublicKey();
    QByteArray receiver_public_key = response.receiverPublicKey();

    unsigned char sessionKey[crypto_secretbox_KEYBYTES];
    randombytes_buf(sessionKey, crypto_secretbox_KEYBYTES);
    QByteArray sessionKeyData(reinterpret_cast<const char*>(sessionKey), crypto_secretbox_KEYBYTES);

    QByteArray encryptedSessionKeyForSender;
    QByteArray encryptedSessionKeyForReceiver;
    try {
        encryptedSessionKeyForSender = cryptoManager->sealData(sessionKeyData, sender_public_key);
        encryptedSessionKeyForReceiver = cryptoManager->sealData(sessionKeyData, receiver_public_key);
    } catch (const std::exception &e) {
        logger->log(Logger::ERROR, "chatmanager.cpp::createDialogKeys", QString("Error encrypting session key: %1").arg(e.what()));
        return;
    }

    QString baseDir = QCoreApplication::applicationDirPath()
                      + "/.tempData/"
                      + QString::number(response.receiverId()) + "/files/personal";
    QDir dir;
    if (!dir.exists(baseDir)) dir.mkpath(baseDir);
    QString filePath = baseDir + "/" + response.uniqMessageId() + ".txt";
    QString metaFilePath = baseDir + "/" + response.uniqMessageId() + ".meta";
    QString metaVoiceFilePath = baseDir + "/" + response.uniqMessageId() + ".meta.v";

    QFile file(filePath);
    QString content;
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        content = in.readAll();
        file.remove();
        file.close();
    }

    bool withFile = QFile::exists(metaFilePath);
    bool withVoiceFile = QFile::exists(metaVoiceFilePath);

    chats::FileData fileDataMsg;
    if(withFile || withVoiceFile){
        QString filePath;
        QFile metaFile;
        if(withFile) metaFile.setFileName(metaFilePath);
        else if(withVoiceFile) metaFile.setFileName(metaVoiceFilePath);

        if (metaFile.open(QIODevice::ReadOnly)) {
            QTextStream in(&metaFile);
            filePath = in.readAll();
            file.remove();
            file.close();
        }

        QFile file(filePath);
        QFileInfo fileInfo(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            logger->log(Logger::WARN,"messagesender.cpp::createDialogKeys","Failed open file");
        }
        QByteArray encryptedFileData;
        try {
            encryptedFileData = cryptoManager->symmetricEncrypt(file.readAll(), sessionKeyData);
        } catch (const std::exception &e) {
            logger->log(Logger::ERROR, "messagesender.cpp::createDialogKeys", QString("Error encrypting message: ") + QString(e.what()));
            return;
        }
        file.close();

        fileDataMsg.setFileName(fileInfo.baseName());
        fileDataMsg.setFileExtension(fileInfo.suffix());
        fileDataMsg.setFileData(encryptedFileData);
    }
    QString message;
    if(!withVoiceFile) {
        QByteArray encryptedMessage;
        try {
            encryptedMessage = cryptoManager->symmetricEncrypt(content.toUtf8(), sessionKeyData);
        } catch (const std::exception &e) {
            logger->log(Logger::ERROR, "chatmanager.cpp::createDialogKeys", QString("Error encrypting message: ") + QString(e.what()));
            return;
        }
        message = QString::fromUtf8(encryptedMessage.toBase64());
    }

    chats::ChatMessage msg;
    msg.setSenderId(activeUserId);
    msg.setReceiverId(response.receiverId());
    msg.setContent(message);
    msg.setSenderEncryptedSessionKey(encryptedSessionKeyForSender);
    msg.setReceiverEncryptedSessionKey(encryptedSessionKeyForReceiver);
    if(withFile || withVoiceFile) {
        msg.setFile(fileDataMsg);
        if(withVoiceFile) ;
        if(withFile) networkManager->getFileNetwork()->sendData("personal_file_message", msg.serialize(&serializer));
    } else
        networkManager->getMessageNetwork()->sendData("personal_message", msg.serialize(&serializer));
}

void AccountManager::getGroupMembers(const int &group_id)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/groupsInfo/" +
                       QString::number(group_id) + ".pb";

    if (!QFile::exists(filePath)) {
        logger->log(Logger::WARN, "accountmanager.cpp::getGroupMembers", "Group info file not exists: " + filePath);
        return;
    }

    QByteArray protoData;
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly)) {
        protoData = file.readAll();
        file.close();
    } else {
        logger->log(Logger::DEBUG, "accountmanager.cpp::getGroupMembers", "Failed to read proto object from file: " + filePath);
        return;
    }

    QProtobufSerializer serializer;
    chats::GroupInfoItem groupInfo;
    if (!groupInfo.deserialize(&serializer, protoData)) {
        logger->log(Logger::WARN, "accountmanager.cpp::getGroupMembers", "Failed to deserialize GroupInfoItem from file: " + filePath);
        return;
    }

    QVariantList membersList = convertProtoListToVariantList(groupInfo.members());
    emit loadGroupMembers(membersList, group_id);
}

void AccountManager::deleteMemberFromGroup(const int &user_id, const int &group_id)
{
    groups::DeleteMemberRequest request;
    request.setUserId(user_id);
    request.setGroupId(group_id);
    request.setCreatorId(this->activeUserId);

    if(user_id != this->activeUserId){
        QProtobufSerializer serializer;
        networkManager->getMessageNetwork()->sendData("delete_member",request.serialize(&serializer));
    }
}

void AccountManager::removeAccountFromConfigManager()
{
    configManager.removeAccount(configManager.getActiveAccount());
}

void AccountManager::showContacts()
{
    logger->log(Logger::DEBUG,"accountmanager.cpp::showContacts", "showContacts starts!");
    QString appPath = QCoreApplication::applicationDirPath();

    QString personalDirPath = appPath + "/.data/" + QString::number(activeUserId) + "/dialogsInfo";
    QDir personalDir(personalDirPath);
    if (!personalDir.exists()) {
        logger->log(Logger::DEBUG,"accountmanager.cpp::showContacts", "personalDir not exists!");
        return;
    }
    QStringList messageFiles = personalDir.entryList(QStringList() << "*.pb", QDir::Files);

    QVariantList contactsList;
    for (const QString &fileName : messageFiles) {
        QString filePath = personalDirPath + "/" + fileName;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            logger->log(Logger::DEBUG,"accountmanager.cpp::showContacts", "Open file failed:" + filePath);
            continue;
        }

        QByteArray fileData = file.readAll();
        file.close();

        chats::DialogInfoItem item;
        QProtobufSerializer serializer;
        if(!item.deserialize(&serializer,fileData)) {
            continue;
        }

        quint64 id = item.userId();

        QString userlogin = item.userlogin();

        if(id == activeUserId) continue;

        QVariantMap contact;
        contact["id"] = id;
        contact["username"] = userlogin;
        contactsList.append(contact);
    }

    emit loadContacts(contactsList);
}

void AccountManager::getChatsInfo()
{
    chats::ChatsInfoRequest request;
    request.setUserId(activeUserId);
    QProtobufSerializer serializer;
    networkManager->getMessageNetwork()->sendData("chats_info",request.serialize(&serializer));
}

bool AccountManager::isAvatarUpToDate(QString avatar_url, int user_id,const QString& type)
{
    logger->log(Logger::INFO, "accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate starts");

    QString pathToAvatar;
    QString avatarCheckerPath;

    if (type == "personal") {
        pathToAvatar = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/avatars/" + type + "/" +
                       QString::number(user_id) + ".png";
        avatarCheckerPath = QCoreApplication::applicationDirPath() + "/.data/" +
                            QString::number(activeUserId) + "/dialogsInfo/" +
                            QString::number(user_id) + ".pb";
    } else if (type == "group") {
        pathToAvatar = QCoreApplication::applicationDirPath() + "/.data/" +
                       QString::number(activeUserId) + "/avatars/" + type + "/" +
                       QString::number(user_id) + ".png";
        avatarCheckerPath = QCoreApplication::applicationDirPath() + "/.data/" +
                            QString::number(activeUserId) + "/groupsInfo/" +
                            QString::number(user_id) + ".pb";
    }

    QFile avatar(pathToAvatar);

    QFile avatarChecker(avatarCheckerPath);
    if (!avatarChecker.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN, "accountmanager.cpp::isAvatarUpToDate", "Failed to open avatarChecker");
        return false;
    }
    QByteArray avatarCheckerData = avatarChecker.readAll();
    avatarChecker.close();

    QProtobufSerializer serializer;

    if (type == "personal") {
        chats::DialogInfoItem dialogInfo;
        if (!dialogInfo.deserialize(&serializer, avatarCheckerData)) {
            logger->log(Logger::WARN, "accountmanager.cpp::isAvatarUpToDate", "Failed to deserialize DialogInfoItem");
            return false;
        }

        if(avatar_url == "basic") {
            avatarGenerator->generateAvatarImage(dialogInfo.userlogin(), dialogInfo.userId(), "personal");
            return true;
        } else {
            if (dialogInfo.avatarUrl() == avatar_url) {
                if (!avatar.exists() || avatar.size() == 0) {
                    logger->log(Logger::WARN, "accountmanager.cpp::isAvatarUpToDate", "Avatar not downloaded");
                    return false;
                }
                return true;
            }
        }
        logger->log(Logger::DEBUG, "accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate return false (personal)");
    } else if (type == "group") {
        chats::GroupInfoItem groupInfo;
        if (!groupInfo.deserialize(&serializer, avatarCheckerData)) {
            logger->log(Logger::WARN, "accountmanager.cpp::isAvatarUpToDate", "Failed to deserialize GroupInfoItem");
            return false;
        }

        if(avatar_url == "basic") {
            avatarGenerator->generateAvatarImage(groupInfo.groupName(), groupInfo.groupId(), "group");
            return true;
        } else {
            if (groupInfo.avatarUrl() == avatar_url) {
                if (!avatar.exists() || avatar.size() == 0) {
                    logger->log(Logger::WARN, "accountmanager.cpp::isAvatarUpToDate", "Avatar not downloaded");
                    return false;
                }
                return true;
            }
        }
        logger->log(Logger::DEBUG, "accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate return false (group)");
    }

    return false;
}

void AccountManager::sendAuthRequest(const QString &flag, const QString &login, const QString &password)
{
    QProtobufSerializer serializer;
    QByteArray data;
    if(flag == "login"){
        auth::LoginRequest loginRequest;
        loginRequest.setLogin(login);
        loginRequest.setPassword(password);
        data = loginRequest.serialize(&serializer);
    } else if(flag == "reg"){
        auth::RegisterRequest regRequest;
        regRequest.setLogin(login);
        regRequest.setPassword(password);
        data = regRequest.serialize(&serializer);
    }
    networkManager->getMessageNetwork()->sendData(flag, data);
}

QList<groups::GroupMemberContact> AccountManager::convertContactsToProto(const QVariantList &contacts)
{
    QList<groups::GroupMemberContact> protoList;
    for (const QVariant &contact : contacts) {
        QVariantMap contactMap = contact.toMap();
        groups::GroupMemberContact member;
        member.setUserId(contactMap["id"].toULongLong());
        protoList.append(member);
    }
    return protoList;
}

QList<quint64> AccountManager::convertContactToList(const QVariantList &contacts)
{
    QList<quint64> ids;
    for(const QVariant &contact : contacts) {
        QVariantMap contactMap = contact.toMap();
        ids.append(contactMap.value("id").toULongLong());
    }
    return ids;
}

QVariantList AccountManager::convertProtoListToVariantList(const QList<chats::GroupMember> &members)
{
    QVariantList list;
    for (const auto &member : members) {
        QVariantMap memberMap;
        memberMap.insert("id", static_cast<int>(member.id_proto()));
        memberMap.insert("username", member.username());
        memberMap.insert("status", member.status());
        memberMap.insert("avatar_url", member.avatarUrl());

        list.append(memberMap);
    }
    return list;
}

void AccountManager::setupConfigManager()
{
    connect(&configManager,&ConfigManager::sendLoginAfterLogout,this,&AccountManager::login);
    connect(&configManager,&ConfigManager::changeAccount,this,&AccountManager::changeAccount);
    connect(&configManager,&ConfigManager::newUser,this,&AccountManager::newUser);
    connect(this,&AccountManager::changeActiveAccount,&configManager,&ConfigManager::changeActiveAccount);
    connect(this,&AccountManager::checkConfigFile,&configManager,&ConfigManager::checkConfigFile);
}

void AccountManager::setupResponseHandler()
{
    connect(this,&AccountManager::processingLoginResultsFromServer,&responseHandler,&ResponseHandler::processingLoginResults);
    connect(this,&AccountManager::processingRegistrationResultsFromServer,&responseHandler,&ResponseHandler::processingRegistrationResults);
    connect(this,&AccountManager::processingSearchDataFromServer,&responseHandler,&ResponseHandler::processingSearchData);
    connect(this,&AccountManager::processingEditProfileFromServer,&responseHandler,&ResponseHandler::processingEditProfile);
    connect(this,&AccountManager::processingAvatarsUpdateFromServer,&responseHandler,&ResponseHandler::processingAvatarsUpdate);
    connect(this,&AccountManager::processingDialogsInfoSave,&responseHandler,&ResponseHandler::processingDialogsInfoSave);
    connect(this,&AccountManager::processingGroupInfoSave,&responseHandler,&ResponseHandler::processingGroupInfoSave);
    connect(this,&AccountManager::processingDeleteGroupMember,&responseHandler,&ResponseHandler::processingDeleteGroupMember);
    connect(this,&AccountManager::processingAddGroupMember,&responseHandler,&ResponseHandler::processingAddGroupMember);

    connect(&responseHandler,&ResponseHandler::sendData, [this](const QString &flag, const QByteArray& data) {
        networkManager->getMessageNetwork()->sendData(flag,data);
    });
    connect(&responseHandler, &ResponseHandler::savePrivateKey, [this](const QByteArray &encryptedPrivateKey, const QByteArray &salt, const QByteArray &nonce) {
        QString fileUrl;
        fileUrl = QCoreApplication::applicationDirPath() + "/.data/crypto/private_key.pem";
        try {
            cryptoManager->decryptAndSavePrivateKey(encryptedPrivateKey,salt,nonce,fileUrl);
        } catch(const std::exception &e) {
            logger->log(Logger::ERROR, "responseHandler.cpp::savePrivateKey", "Error save private key: " + QString(e.what()));
            return;
        }
    });

    connect(&responseHandler,&ResponseHandler::transferUserNameAndIdAfterLogin,this,&AccountManager::transferUserNameAndIdAfterLogin);
    connect(&responseHandler,&ResponseHandler::checkAndSendAvatarUpdate,this,&AccountManager::checkAndSendAvatarUpdate);
    connect(&responseHandler,&ResponseHandler::loginSuccess,this,&AccountManager::loginSuccess);
    connect(&responseHandler,&ResponseHandler::loginFail,this,&AccountManager::loginFail);
    connect(&responseHandler,&ResponseHandler::addAccount,&configManager,&ConfigManager::addAccount);
    connect(&responseHandler,&ResponseHandler::updatingChats,this,&AccountManager::updatingChats);

    connect(&responseHandler,&ResponseHandler::registrationSuccess,this,&AccountManager::registrationSuccess);
    connect(&responseHandler,&ResponseHandler::registrationFail,this,&AccountManager::registrationFail);

    connect(&responseHandler,&ResponseHandler::newSearchUser,this,&AccountManager::newSearchUser);

    connect(&responseHandler,&ResponseHandler::getChatsInfo,this,&AccountManager::getChatsInfo);

    connect(&responseHandler,&ResponseHandler::getGroupMembers,this,&AccountManager::getGroupMembers);
    connect(&responseHandler,&ResponseHandler::clearMessagesAfterDelete,this,&AccountManager::clearMessagesAfterDelete);

    connect(&responseHandler,&ResponseHandler::editUniqueError,this,&AccountManager::editUniqueError);
    connect(&responseHandler,&ResponseHandler::unknownError,this,&AccountManager::unknownError);
    connect(&responseHandler,&ResponseHandler::editUserlogin,this,&AccountManager::editUserlogin);
    connect(&responseHandler,&ResponseHandler::editPhoneNumber,this,&AccountManager::editPhoneNumber);
    connect(&responseHandler,&ResponseHandler::editName,this,&AccountManager::editName);
}

void AccountManager::updatingChats()
{
    chats::UpdatingChatsRequest request;
    request.setUserId(activeUserId);
    QProtobufSerializer serializer;

    networkManager->getMessageNetwork()->sendData("updating_chats", request.serialize(&serializer));
}
