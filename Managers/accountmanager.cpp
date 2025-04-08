#include "accountmanager.h"

AccountManager::AccountManager(NetworkManager* networkManager,QObject *parent)
    : QObject{parent}
{
    this->networkManager = networkManager;
    setupConfigManager();
    setupResponseHandler();
}

void AccountManager::login(const QString login, const QString password)
{
    sendAuthRequest("login",login,password);
}

void AccountManager::registerAccount(const QString login, const QString password)
{
    sendAuthRequest("login",login,password);
}

void AccountManager::logout()
{
    QJsonObject json;
    json["flag"]= "logout";
    networkManager->getMessageNetwork()->sendData(json);
    logger->log(Logger::INFO,"accountmanager.cpp::logout","Client logout process has begun");
    configManager.removeAccount(configManager.getActiveAccount());
}

void AccountManager::sendSearchToServer(const QString &searchable)
{
    QJsonObject dataForSearchUsers;
    dataForSearchUsers["flag"] = "search";
    dataForSearchUsers["searchable"] = searchable;
    networkManager->getMessageNetwork()->sendData(dataForSearchUsers);
}

void AccountManager::sendEditProfileRequest(const QString editable, const QString editInformation)
{
    QJsonObject dataEditProfile;
    dataEditProfile["flag"] = "edit";
    dataEditProfile["user_id"] = activeUserId;
    dataEditProfile["editable"] = editable;
    dataEditProfile["editInformation"] = editInformation;
    networkManager->getMessageNetwork()->sendData(dataEditProfile);
}

void AccountManager::clientChangeAccount()
{
    QJsonObject json;
    json["flag"]= "logout";
    networkManager->getMessageNetwork()->sendData(json);
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
    QString appDir = QCoreApplication::applicationDirPath();

    QJsonObject avatarsUpdateJson;
    avatarsUpdateJson["flag"] = "avatars_update";
    avatarsUpdateJson["user_id"] = activeUserId;
    networkManager->getMessageNetwork()->sendData(avatarsUpdateJson);
}

void AccountManager::setActiveUser(const QString &user_login, const int &user_id)
{
    activeUserLogin = user_login;
    activeUserId = user_id;
    responseHandler.setActiveUser(user_login,user_id);
}

void AccountManager::setLogger(Logger *logger)
{
    this->logger = logger;
    configManager.setLogger(logger);
    responseHandler.setLogger(logger);
}

void AccountManager::createGroup(const QString &groupName, const QString& avatarPath, const QVariantList &selectedContacts)
{
    QJsonObject createGroupJson;
    createGroupJson["flag"] = "create_group";
    createGroupJson["groupName"] = groupName;
    createGroupJson["creator_id"] = activeUserId;
    createGroupJson["members"] = convertContactsToArray(selectedContacts);

    if(avatarPath == ""){
        createGroupJson["avatar_url"] = "";
        networkManager->getMessageNetwork()->sendData(createGroupJson);
    } else {
        QFile file(avatarPath);
        QFileInfo fileInfo(avatarPath);
        if (!file.open(QIODevice::ReadOnly)) {
            logger->log(Logger::WARN,"networkmanager.cpp::sendAvatar","Failed open avatar");
        }
        QByteArray fileData = file.readAll();
        file.close();
        createGroupJson["fileName"] = fileInfo.baseName();
        createGroupJson["fileExtension"] = fileInfo.suffix();
        createGroupJson["fileData"] = QString(fileData.toBase64());
        QJsonDocument doc(createGroupJson);
        networkManager->getFileNetwork()->sendToFileServer(doc);
    }
}

void AccountManager::addGroupMembers(const int &group_id, const QVariantList &selectedContacts)
{
    QJsonObject addMembersJson;
    addMembersJson["flag"] = "add_group_members";
    addMembersJson["group_id"] = group_id;
    addMembersJson["admin_id"] = this->activeUserId;
    addMembersJson["members"] = convertContactsToArray(selectedContacts);
    networkManager->getMessageNetwork()->sendData(addMembersJson);
}

void AccountManager::getGroupMembers(const int &group_id)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo/" + QString::number(group_id) + ".json";
    QFile groupInfoFile(filePath);
    if(!groupInfoFile.exists()){
        logger->log(Logger::WARN,"accountmanager.cpp::getGroupMembers", "Group info file not exists: " + filePath);
        return;
    }
    if (!groupInfoFile.open(QIODevice::ReadOnly)) {
        logger->log(Logger::DEBUG,"accountmanager.cpp::getGroupMembers", "Open file failed: " + filePath);
        return;
    }

    QByteArray infoData = groupInfoFile.readAll();
    groupInfoFile.close();
    QJsonDocument doc = QJsonDocument::fromJson(infoData);
    QJsonObject groupInfoJson = doc.object();

    QJsonArray membersArray = groupInfoJson["members"].toArray();

    emit loadGroupMembers(convertArrayToVariantList(membersArray),group_id);
}

void AccountManager::deleteMemberFromGroup(const int &user_id, const int &group_id)
{
    QJsonObject deleteMemberObject;
    deleteMemberObject["flag"] = "delete_member";
    deleteMemberObject["user_id"] = user_id;
    deleteMemberObject["group_id"] = group_id;
    deleteMemberObject["creator_id"] = this->activeUserId;

    if(user_id != this->activeUserId){
        networkManager->getMessageNetwork()->sendData(deleteMemberObject);
    }
}

void AccountManager::removeAccountFromConfigManager()
{
    configManager.removeAccount(configManager.getActiveAccount());
}

void AccountManager::getContactList()
{
    logger->log(Logger::DEBUG,"accountmanager.cpp::getContactList", "getContactList starts!");
    QString appPath = QCoreApplication::applicationDirPath();

    QString personalDirPath = appPath + "/.data/" + QString::number(activeUserId) + "/dialogsInfo";
    QDir personalDir(personalDirPath);
    if (!personalDir.exists()) {
        logger->log(Logger::DEBUG,"accountmanager.cpp::getContactList", "personalDir not exists!");
        return;
    }
    QStringList messageFiles = personalDir.entryList(QStringList() << "*.json", QDir::Files);
    QJsonArray contactsArray;

    for (const QString &fileName : messageFiles) {
        QString filePath = personalDirPath + "/" + fileName;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            logger->log(Logger::DEBUG,"accountmanager.cpp::getContactList", "Open file failed:" + filePath);
            continue;
        }

        QByteArray fileData = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(fileData);

        QJsonObject json = doc.object();
        if (json.isEmpty()) continue;

        int id = json["user_id"].toInt();

        QString userlogin = json["userlogin"].toString();

        if(id == activeUserId) continue;

        QJsonObject contact;
        contact["id"] = id;
        contact["username"] = userlogin;
        contactsArray.append(contact);
    }

    QString savePath = appPath + "/.data/" + QString::number(activeUserId) + "/contacts/contacts.json";
    QDir saveDir(QFileInfo(savePath).absolutePath());
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    }

    QFile saveFile(savePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        logger->log(Logger::DEBUG,"accountmanager.cpp::getContactList", "Open file failed:" + savePath);
        return;
    }

    QJsonDocument saveDoc(contactsArray);
    saveFile.write(saveDoc.toJson(QJsonDocument::Indented));
    saveFile.close();
}

void AccountManager::showContacts()
{
    QJsonArray contactsArray;
    QString contactsFilePath = QCoreApplication::applicationDirPath() + "/.data/" + activeUserLogin + "/contacts/contacts.json";

    QFile contactsFile(contactsFilePath);
    if (!contactsFile.open(QIODevice::ReadOnly)) {
        logger->log(Logger::DEBUG,"accountmanager.cpp::showContacts", "Open file failed:" + contactsFilePath);
        return;
    }
    QByteArray fileData = contactsFile.readAll();
    contactsFile.close();

    contactsArray = QJsonDocument::fromJson(fileData).array();
    QVariantList contactsList;
    for (const QJsonValue &value : contactsArray) {
        contactsList.append(value.toObject().toVariantMap());
    }

    emit loadContacts(contactsList);
}

void AccountManager::getChatsInfo()
{
    QJsonObject infoObject;
    infoObject["flag"] = "chats_info";
    infoObject["user_id"] = activeUserId;
    networkManager->getMessageNetwork()->sendData(infoObject);
}

bool AccountManager::isAvatarUpToDate(QString avatar_url, int user_id,const QString& type)
{
    logger->log(Logger::INFO,"accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate starts");
    QString pathToAvatar;
    QString avatarCheckerPath;
    if(type == "personal") {
        pathToAvatar = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/avatars/" + type + "/" + QString::number(user_id) + ".png";
        avatarCheckerPath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo/" + QString::number(user_id) + ".json";
    } else if(type == "group") {
        pathToAvatar = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/avatars/" + type + "/" + QString::number(user_id) + ".png";
        avatarCheckerPath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo/" + QString::number(user_id) + ".json";
    }
    QFile avatar(pathToAvatar);
    if(!avatar.exists() || avatar.size() == 0) {
        logger->log(Logger::WARN,"accountmanager.cpp::isAvatarUpToDate", "Avatar not downloaded");
        return false;
    }
    QFile avatarChecker(avatarCheckerPath);
    if(!avatarChecker.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"accountmanager.cpp::isAvatarUpToDate", "Failed to open avatarChecker");
        return false;
    }
    QByteArray avatarCheckerData = avatarChecker.readAll();
    avatarChecker.close();
    QJsonDocument avatarCheckerDoc = QJsonDocument::fromJson(avatarCheckerData);

    if(type == "personal") {
        QJsonObject dialogInfo = avatarCheckerDoc.object();
        if(dialogInfo["avatar_url"].toString() == avatar_url) return true;
        logger->log(Logger::DEBUG,"accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate return false (personal)");
    } else if(type == "group") {
        QJsonObject groupInfo = avatarCheckerDoc.object();
        if(groupInfo["avatar_url"].toString() == avatar_url) return true;
        logger->log(Logger::DEBUG,"accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate return false (group)");
    }
    return false;
}

void AccountManager::sendAuthRequest(const QString &flag, const QString &login, const QString &password)
{
    QJsonObject data;
    data["flag"] = flag;
    data["login"] = login;
    data["password"] = password;
    networkManager->getMessageNetwork()->sendData(data);
}

QJsonArray AccountManager::convertContactsToArray(const QVariantList &contacts)
{
    QJsonArray membersArray;
    for (const QVariant &contact : contacts) {
        QVariantMap contactMap = contact.toMap();
        QJsonObject contactJson = QJsonObject::fromVariantMap(contactMap);
        membersArray.append(contactJson);
    }
    return membersArray;
}

QVariantList AccountManager::convertArrayToVariantList(const QJsonArray &array)
{
    QVariantList list;
    for (const QJsonValue &value : array) {
        list.append(value.toObject().toVariantMap());
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
    connect(this,&AccountManager::processingRegistrationResultsFromServer,&responseHandler,&ResponseHandler::processingRegistrationResults);
    connect(this,&AccountManager::processingSearchDataFromServer,&responseHandler,&ResponseHandler::processingSearchData);
    connect(this,&AccountManager::processingEditProfileFromServer,&responseHandler,&ResponseHandler::processingEditProfile);
    connect(this,&AccountManager::processingAvatarsUpdateFromServer,&responseHandler,&ResponseHandler::processingAvatarsUpdate);
    connect(this,&AccountManager::processingDialogsInfoSave,&responseHandler,&ResponseHandler::processingDialogsInfoSave);
    connect(this,&AccountManager::processingGroupInfoSave,&responseHandler,&ResponseHandler::processingGroupInfoSave);
    connect(this,&AccountManager::processingDeleteGroupMember,&responseHandler,&ResponseHandler::processingDeleteGroupMember);
    connect(this,&AccountManager::processingAddGroupMember,&responseHandler,&ResponseHandler::processingAddGroupMember);

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
    QJsonObject mainObject;
    mainObject["flag"] = "updating_chats";
    mainObject["user_id"] = activeUserId;
    networkManager->getMessageNetwork()->sendData(mainObject);
}
