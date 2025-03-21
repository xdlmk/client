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
    QJsonObject loginDataJson;
    loginDataJson["flag"] = "login";
    loginDataJson["login"] = login;
    loginDataJson["password"] = password;
    networkManager->sendData(loginDataJson);
}

void AccountManager::registerAccount(const QString login, const QString password)
{
    QJsonObject registrationDataJson;
    registrationDataJson["flag"] = "reg";
    registrationDataJson["login"] = login;
    registrationDataJson["password"] = password;
    networkManager->sendData(registrationDataJson);
}

void AccountManager::logout()
{
    QJsonObject json;
    json["flag"]= "logout";
    networkManager->sendData(json);
    logger->log(Logger::INFO,"accountmanager.cpp::logout","Client logout process has begun");
    configManager.removeAccount(configManager.getActiveAccount());
}

void AccountManager::sendSearchToServer(const QString &searchable)
{
    QJsonObject dataForSearchUsers;
    dataForSearchUsers["flag"] = "search";
    dataForSearchUsers["searchable"] = searchable;
    networkManager->sendData(dataForSearchUsers);
}

void AccountManager::sendEditProfileRequest(const QString editable, const QString editInformation)
{
    QJsonObject dataEditProfile;
    dataEditProfile["flag"] = "edit";
    dataEditProfile["user_id"] = user_id;
    dataEditProfile["editable"] = editable;
    dataEditProfile["editInformation"] = editInformation;
    networkManager->sendData(dataEditProfile);
}

void AccountManager::clientChangeAccount()
{
    QJsonObject json;
    json["flag"]= "logout";
    networkManager->sendData(json);
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
    QString personalDirPath = appDir + "/resources/" + activeUserName + "/personal";
    QDir personalDir(personalDirPath);
    if (!personalDir.exists()) {
        logger->log(Logger::WARN,"accountmanager.cpp::sendAvatarsUpdate","Dir with messages files not exists");
        return;
    }
    QStringList filters;
    filters << "message_*.json";
    QStringList fileList = personalDir.entryList(filters, QDir::Files);
    QList<int> idList;

    for (const QString &fileName : fileList) {
        QString filePath = personalDirPath + "/" + fileName;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            logger->log(Logger::WARN,"accountmanager.cpp::sendAvatarsUpdate","Failed to open file: " + filePath);
            continue;
        }

        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);

        if (!doc.isArray()) {
            logger->log(Logger::WARN,"accountmanager.cpp::sendAvatarsUpdate", "File does not contain a JSON array: " + fileName);
            continue;
        }

        QJsonArray jsonArray = doc.array();
        if (jsonArray.isEmpty()) {
            logger->log(Logger::WARN,"accountmanager.cpp::sendAvatarsUpdate", "Empty JSON array in file: " + fileName);
            continue;
        }
        QJsonObject lastObject = jsonArray.last().toObject();
        int id = lastObject["id"].toInt();
        idList.append(id);
    }

    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini";
    QSettings settings(configFilePath, QSettings::IniFormat);

    int total = settings.value("total", 0).toInt();
    for (int i = 1; i <= total; ++i) {
        int configId = settings.value("id" + QString::number(i), -1).toInt();
        if (configId != -1 && !idList.contains(configId)) {
            idList.append(configId);
        }
    }

    QJsonObject avatarsUpdateJson;
    avatarsUpdateJson["flag"] = "avatars_update";
    QJsonArray idArray;
    for (int id : idList) {
        idArray.append(id);
    }
    avatarsUpdateJson["ids"] = idArray;

    QString groupInfoDirPath = appDir + "/.data/" + activeUserName + "/groupsInfo";
    QJsonArray groupIds;

    QStringList groupFilters;
    groupFilters << "*.json";

    QDir dir(groupInfoDirPath);
    dir.setNameFilters(groupFilters);
    if (dir.exists()) {
        QFileInfoList fileList = dir.entryInfoList(QDir::Files);
        for (const QFileInfo &fileInfo : fileList) {
            QString fileName = fileInfo.baseName();
            int groupId = fileName.toInt();
            groupIds.append(groupId);
        }
        avatarsUpdateJson["groups_ids"] = groupIds;
    }
    networkManager->sendData(avatarsUpdateJson);
}

void AccountManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserName = userName;
    user_id = userId;
    responseHandler.setActiveUser(userName,userId);
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
    createGroupJson["creator_id"] = user_id;

    QJsonArray membersArray;
    for (const QVariant &contact : selectedContacts) {
        QVariantMap contactMap = contact.toMap();
        QJsonObject contactJson = QJsonObject::fromVariantMap(contactMap);
        membersArray.append(contactJson);
    }
    createGroupJson["members"] = membersArray;

    if(avatarPath == ""){
        createGroupJson["avatar_url"] = "";
        networkManager->sendData(createGroupJson);
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
        networkManager->sendToFileServer(doc);
    }
}

void AccountManager::addGroupMembers(const int &group_id, const QVariantList &selectedContacts)
{
    QJsonObject addMembersJson;
    addMembersJson["flag"] = "add_group_members";
    addMembersJson["group_id"] = group_id;
    addMembersJson["admin_id"] = this->user_id;
    QJsonArray membersArray;
    for (const QVariant &contact : selectedContacts) {
        QVariantMap contactMap = contact.toMap();
        QJsonObject contactJson = QJsonObject::fromVariantMap(contactMap);
        membersArray.append(contactJson);
    }
    addMembersJson["members"] = membersArray;
    networkManager->sendData(addMembersJson);
}

void AccountManager::saveGroupInfo(const QJsonObject &receivedGroupInfoJson)
{
    QJsonArray groupsInfoArray = receivedGroupInfoJson["info"].toArray();
    QDir saveDir(QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/groupsInfo");
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    } else {
        saveDir.removeRecursively();
        saveDir.mkpath(".");
    }

    for(QJsonValue value : groupsInfoArray) {
        QJsonObject groupInfo = value.toObject();
        int group_id = groupInfo["group_id"].toInt();

        QString savePath = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/groupsInfo/" + QString::number(group_id) + ".json";

        QFile saveFile(savePath);
        if (!saveFile.open(QIODevice::WriteOnly)) {
            logger->log(Logger::DEBUG,"accountmanager.cpp::saveGroupInfo", "Open file failed:" + savePath);
            return;
        }

        QJsonDocument saveDoc(groupInfo);
        saveFile.write(saveDoc.toJson(QJsonDocument::Indented));
        saveFile.close();
    }
}

void AccountManager::saveDialogsInfo(const QJsonObject &receivedDialogInfoJson)
{
    QJsonArray dialogsInfoArray = receivedDialogInfoJson["info"].toArray();

    QDir saveDir(QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/dialogsInfo");
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    } else {
        saveDir.removeRecursively();
        saveDir.mkpath(".");
    }
    for(QJsonValue value : dialogsInfoArray) {
        QJsonObject dialogInfo = value.toObject();
        if(!dialogInfo.contains("user_id")) continue;
        int user_id = dialogInfo["user_id"].toInt();

        QString savePath = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/dialogsInfo/" + QString::number(user_id) + ".json";
        QFile saveFile(savePath);
        if (!saveFile.open(QIODevice::WriteOnly)) {
            logger->log(Logger::DEBUG,"accountmanager.cpp::saveDialogsInfo", "Open file failed:" + savePath);
            return;
        }

        QJsonDocument saveDoc(dialogInfo);
        saveFile.write(saveDoc.toJson(QJsonDocument::Indented));
        saveFile.close();
    }
}

void AccountManager::getGroupMembers(const int &group_id)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/groupsInfo/" + QString::number(group_id) + ".json";
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
    QVariantList membersList;

    for (const QJsonValue &value : membersArray) {
        membersList.append(value.toObject().toVariantMap());
    }

    emit loadGroupMembers(membersList,group_id);
}

void AccountManager::deleteMemberFromGroup(const int &user_id, const int &group_id)
{
    QJsonObject deleteMemberObject;
    deleteMemberObject["flag"] = "delete_member";
    deleteMemberObject["user_id"] = user_id;
    deleteMemberObject["group_id"] = group_id;
    deleteMemberObject["creator_id"] = this->user_id;

    if(user_id != this->user_id){
        networkManager->sendData(deleteMemberObject);
    }
}

void AccountManager::deleteGroupMemberReceived(const QJsonObject &receivedDeleteMemberFromGroup)
{
    if(receivedDeleteMemberFromGroup["error_code"].toInt() == 0) {
        int group_id = receivedDeleteMemberFromGroup["group_id"].toInt();
        int error_code = deleteUserFromInfoFile(group_id, receivedDeleteMemberFromGroup["deleted_user_id"].toInt());
        if(error_code == 0){
            logger->log(Logger::INFO,"accountmanager.cpp::deleteGroupMemberReceived", "Member with user_id: " + QString::number(receivedDeleteMemberFromGroup["deleted_user_id"].toInt()) + " successfuly removed from info list");
            getGroupMembers(group_id);
        } else {
            logger->log(Logger::WARN,"accountmanager.cpp::deleteGroupMemberReceived", "Member with user_id: " + QString::number(receivedDeleteMemberFromGroup["deleted_user_id"].toInt()) + " not removed from info list");
        }

    } else if(receivedDeleteMemberFromGroup["error_code"].toInt() == 1) {
        logger->log(Logger::WARN,"accountmanager.cpp::deleteGroupMemberReceived", "Member with user_id: " + QString::number(receivedDeleteMemberFromGroup["deleted_user_id"].toInt()) + " is not a member of the group");
    } else if(receivedDeleteMemberFromGroup["error_code"].toInt() == 2) {
        logger->log(Logger::WARN,"accountmanager.cpp::deleteGroupMemberReceived", "The active user does not have sufficient rights to perform this operation");
    }
}

void AccountManager::addGroupMemberReceived(const QJsonObject &receivedAddMemberFromGroup)
{
    int group_id = receivedAddMemberFromGroup["group_id"].toInt();
    QJsonArray newMembers = receivedAddMemberFromGroup["addedMembers"].toArray();

    for(const QJsonValue &value : newMembers) {
        QJsonObject member = value.toObject();
        if(member["id"].toInt() == this->user_id) {
            updatingChats();
            return;
        }
    }

    QString pathToGroupInfo = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/groupsInfo/" + QString::number(group_id) + ".json";
    QFile file(pathToGroupInfo);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"accountmanager.cpp::addGroupMemberReceived", "Group info file not open for read");
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject json = doc.object();
    file.close();
    QJsonArray members = json["members"].toArray();


    for(const QJsonValue &value : newMembers) {
        QJsonObject member = value.toObject();
        members.append(member);
    }
    json["members"] = members;

    if (!file.open(QIODevice::WriteOnly)) {
        logger->log(Logger::WARN,"accountmanager.cpp::addGroupMemberReceived", "Group info file not open for write");
    }
    file.write(QJsonDocument(json).toJson());
    file.close();
    getGroupMembers(group_id);
}

void AccountManager::getContactList()
{
    logger->log(Logger::DEBUG,"accountmanager.cpp::getContactList", "getContactList starts!");
    QString appPath = QCoreApplication::applicationDirPath();
    QString personalDirPath = appPath + "/resources/" + activeUserName + "/personal";

    QDir personalDir(personalDirPath);
    if (!personalDir.exists()) {
        logger->log(Logger::DEBUG,"accountmanager.cpp::getContactList", "personalDir not exists!");
        return;
    }

    QStringList messageFiles = personalDir.entryList(QStringList() << "message_*.json", QDir::Files);
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

        QJsonArray jsonArray = doc.array();
        if (jsonArray.isEmpty()) continue;

        QJsonObject lastObject = jsonArray.last().toObject();
        int id = lastObject["id"].toInt();

        QString username = fileName.mid(8, fileName.length() - 13);
        if(username == activeUserName) continue;

        QJsonObject contact;
        contact["id"] = id;
        contact["username"] = username;
        contactsArray.append(contact);
    }
    QString savePath = appPath + "/.data/" + activeUserName + "/contacts/contacts.json";
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
    QString contactsFilePath = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/contacts/contacts.json";

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
    infoObject["userlogin"] = activeUserName;
    networkManager->sendData(infoObject);
}

int AccountManager::deleteUserFromInfoFile(const int &group_id, const int &user_id)
{
    QString pathToGroupInfo = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/groupsInfo/" + QString::number(group_id) + ".json";
    QFile file(pathToGroupInfo);
    if(user_id == this->user_id) {
        QFile::remove(pathToGroupInfo);
        QFile::remove(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/group/message_" + QString::number(group_id) + ".json");
        emit clearMessagesAfterDelete(group_id);
    }
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN,"accountmanager.cpp::deleteUserFromInfoFile", "Group info file not open for read");
        return 1;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject json = doc.object();
    file.close();
    QJsonArray members = json["members"].toArray();
    QJsonArray newMembers;

    for (const QJsonValue &value : members) {
        QJsonObject member = value.toObject();
        int memberId = member["id"].toInt();
        if (memberId == user_id) {
            logger->log(Logger::DEBUG,"accountmanager.cpp::deleteUserFromInfoFile", "User with id: " + QString::number(user_id) + " successfuly removed from group with id: " + QString::number(group_id));
            continue;
        }

        newMembers.append(member);
    }
    json["members"] = newMembers;

    if (!file.open(QIODevice::WriteOnly)) {
        logger->log(Logger::WARN,"accountmanager.cpp::deleteUserFromInfoFile", "Group info file not open for write");
        return 1;
    }
    file.write(QJsonDocument(json).toJson());
    file.close();
    return 0;
}

bool AccountManager::isAvatarUpToDate(QString avatar_url, int user_id,const QString& type)
{
    logger->log(Logger::INFO,"accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate starts");
    QString pathToAvatar;
    QString avatarCheckerPath;
    if(type == "personal") {
        pathToAvatar = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/avatars/" + type + "/" + QString::number(user_id) + ".png";
        avatarCheckerPath = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/dialogsInfo/" + QString::number(user_id) + ".json";
    } else if(type == "group") {
        pathToAvatar = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/avatars/" + type + "/" + QString::number(user_id) + ".png";
        avatarCheckerPath = QCoreApplication::applicationDirPath() + "/.data/" + activeUserName + "/groupsInfo/" + QString::number(user_id) + ".json";
    }
    QFile avatar(pathToAvatar);
    if(!avatar.exists() || avatar.size() == 0) {
        logger->log(Logger::INFO,"accountmanager.cpp::isAvatarUpToDate", "Avatar not downloaded");
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
    mainObject["userlogin"] = activeUserName;
    networkManager->sendData(mainObject);
}


