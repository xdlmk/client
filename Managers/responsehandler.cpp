#include "responsehandler.h"

ResponseHandler::ResponseHandler(QObject *parent)
    : QObject{parent}
{}

void ResponseHandler::setLogger(Logger *logger)
{
    this->logger = logger;
}

void ResponseHandler::setActiveUser(const QString &userLogin, const int &userId)
{
    activeUserLogin = userLogin;
    activeUserId = userId;
}

void ResponseHandler::processingLoginResults(const QJsonObject &loginResultsJson)
{
    logger->log(Logger::INFO,"responsehandler.cpp::processingLoginResults","processingLoginResults has begun");
    QString success = loginResultsJson["success"].toString();
    QString userlogin = loginResultsJson["userlogin"].toString();
    QString password = loginResultsJson["password"].toString();
    int userId = loginResultsJson["user_id"].toInt();

    QString avatar_url = loginResultsJson["avatar_url"].toString();

    if(success == "ok")
    {
        QJsonObject setIdentifiers;
        setIdentifiers["flag"] = "identifiers";
        setIdentifiers["user_id"] = userId;
        emit sendData(setIdentifiers);
        emit transferUserNameAndIdAfterLogin(userlogin,userId);
        emit loginSuccess(userlogin, userId);
        emit checkAndSendAvatarUpdate(avatar_url, userId, "personal");
        emit addAccount(userlogin,password,userId);
        emit updatingChats();
    }
    else if(success == "poor")
    {
        emit loginFail();
    }
}

void ResponseHandler::processingRegistrationResults(const QJsonObject &regResultsJson)
{
    logger->log(Logger::INFO,"responsehandler.cpp::processingRegistrationResults","processingRegistrationResultsFromServer has begun");
    QString success = regResultsJson["success"].toString();
    if(success == "ok")
    {
        emit registrationSuccess() ;
    }
    else if (success == "poor")
    {
        QString error = regResultsJson["errorMes"].toString();
        emit registrationFail(error);
    }
}

void ResponseHandler::processingSearchData(const QJsonObject &searchDataJson)
{
    logger->log(Logger::INFO,"responsehandler.cpp::processingSearchData","processingSearchDataFromServer has begun");
    QJsonArray resultsArray = searchDataJson.value("results").toArray();
    for (const QJsonValue &value : resultsArray) {
        QJsonObject userObj = value.toObject();
        int id = userObj.value("id").toInt();
        QString userlogin = userObj.value("userlogin").toString();
        QString avatar_url = userObj.value("avatar_url").toString();
        emit checkAndSendAvatarUpdate(avatar_url,id,"personal");

        emit newSearchUser(userlogin,id);
    }
}

void ResponseHandler::processingEditProfile(const QJsonObject &editResultsJson)
{
    logger->log(Logger::INFO,"responsehandler.cpp::processingEditProfile","processingEditProfileFromServer has begun");

    QString status = editResultsJson["status"].toString();
    if(status == "poor"){
        QString error = editResultsJson["error"].toString();
        if(error == "Unique error"){
            emit editUniqueError();
            logger->log(Logger::ERROR,"responsehandler.cpp::processingEditProfile","Information changed was not unique");
            return;
        }
        logger->log(Logger::WARN,"responsehandler.cpp::processingEditProfile","Unknown request error");
        emit unknownError();
        return;
    }

    QString editable = editResultsJson["editable"].toString();
    QString editInformation = editResultsJson["editInformation"].toString();

    if(editable == "Username") {
        emit editUserlogin(editInformation);
    } else if (editable == "Phone number") {
        emit editPhoneNumber(editInformation);
    } else if (editable == "Name") {
        emit editName(editInformation);
    }
}

void ResponseHandler::processingAvatarsUpdate(const QJsonObject &avatarsUpdateJson)
{
    logger->log(Logger::INFO,"responsehandler.cpp::processingAvatarsUpdate","processingAvatarsUpdate has begun");
    QJsonArray avatarsArray = avatarsUpdateJson["avatars"].toArray();
    QJsonArray groupsAvatarsArray = avatarsUpdateJson["groups_avatars"].toArray();
    if (avatarsArray.isEmpty()) {
        logger->log(Logger::WARN,"responsehandler.cpp::processingAvatarsUpdate","Urls json array is empty");
    }
    if (groupsAvatarsArray.isEmpty()) {
        logger->log(Logger::WARN,"responsehandler.cpp::processingAvatarsUpdate","Groups urls json array is empty");
    }

    for (const QJsonValue &value : avatarsArray) {
        QJsonObject avatarObject = value.toObject();

        int id = avatarObject["id"].toInt();
        QString avatarUrl = avatarObject["avatar_url"].toString();

        emit checkAndSendAvatarUpdate(avatarUrl, id, "personal");
    }

    for (const QJsonValue &value : groupsAvatarsArray) {
        QJsonObject avatarObject = value.toObject();

        int id = avatarObject["group_id"].toInt();
        QString avatarUrl = avatarObject["avatar_url"].toString();

        emit checkAndSendAvatarUpdate(avatarUrl, id, "group");
    }

    emit getChatsInfo();
}

void ResponseHandler::processingGroupInfoSave(const QJsonObject &receivedGroupInfoJson)
{
    QJsonArray groupsInfoArray = receivedGroupInfoJson["info"].toArray();
    QDir saveDir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo");
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    } else {
        saveDir.removeRecursively();
        saveDir.mkpath(".");
    }

    for(QJsonValue value : groupsInfoArray) {
        QJsonObject groupInfo = value.toObject();
        int group_id = groupInfo["group_id"].toInt();

        QString savePath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo/" + QString::number(group_id) + ".json";

        if(!writeJsonToFile(savePath,groupInfo)) {
            logger->log(Logger::WARN, "responsehandler.cpp::processingGroupInfoSave", "writeJsonToFile return false");
        }
    }
}

void ResponseHandler::processingDialogsInfoSave(const QJsonObject &receivedDialogInfoJson)
{
    QJsonArray dialogsInfoArray = receivedDialogInfoJson["info"].toArray();

    QDir saveDir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo");
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

        QString savePath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo/" + QString::number(user_id) + ".json";
        if(!writeJsonToFile(savePath,dialogInfo)) {
            logger->log(Logger::WARN, "responsehandler.cpp::processingDialogsInfoSave", "writeJsonToFile return false");
        }
    }
}

void ResponseHandler::processingDeleteGroupMember(const QJsonObject &receivedDeleteMemberFromGroup)
{
    if(receivedDeleteMemberFromGroup["error_code"].toInt() == 0) {
        int group_id = receivedDeleteMemberFromGroup["group_id"].toInt();
        int error_code = deleteUserFromInfoFile(group_id, receivedDeleteMemberFromGroup["deleted_user_id"].toInt());
        if(error_code == 0){
            logger->log(Logger::INFO,"responsehandler.cpp::deleteGroupMemberReceived", "Member with user_id: " + QString::number(receivedDeleteMemberFromGroup["deleted_user_id"].toInt()) + " successfuly removed from info list");
            emit getGroupMembers(group_id);
        } else {
            logger->log(Logger::WARN,"responsehandler.cpp::deleteGroupMemberReceived", "Member with user_id: " + QString::number(receivedDeleteMemberFromGroup["deleted_user_id"].toInt()) + " not removed from info list");
        }

    } else if(receivedDeleteMemberFromGroup["error_code"].toInt() == 1) {
        logger->log(Logger::WARN,"responsehandler.cpp::deleteGroupMemberReceived", "Member with user_id: " + QString::number(receivedDeleteMemberFromGroup["deleted_user_id"].toInt()) + " is not a member of the group");
    } else if(receivedDeleteMemberFromGroup["error_code"].toInt() == 2) {
        logger->log(Logger::WARN,"responsehandler.cpp::deleteGroupMemberReceived", "The active user does not have sufficient rights to perform this operation");
    }
}

void ResponseHandler::processingAddGroupMember(const QJsonObject &receivedAddMemberFromGroup)
{
    int group_id = receivedAddMemberFromGroup["group_id"].toInt();
    QJsonArray newMembers = receivedAddMemberFromGroup["addedMembers"].toArray();

    for(const QJsonValue &value : newMembers) {
        QJsonObject member = value.toObject();
        if(member["id"].toInt() == this->activeUserId) {
            updatingChats();
            return;
        }
    }

    QString pathToGroupInfo = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo/" + QString::number(group_id) + ".json";
    QJsonObject json;
    readJsonFromFile(pathToGroupInfo,json);
    QJsonArray members = json["members"].toArray();

    for(const QJsonValue &value : newMembers) {
        QJsonObject member = value.toObject();
        members.append(member);
    }

    json["members"] = members;

    if(writeJsonToFile(pathToGroupInfo,json)) emit getGroupMembers(group_id);
}

int ResponseHandler::deleteUserFromInfoFile(const int &group_id, const int &user_id)
{
    QString pathToGroupInfo = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo/" + QString::number(group_id) + ".json";
    QFile file(pathToGroupInfo);
    if(user_id == this->activeUserId) {
        QFile::remove(pathToGroupInfo);
        QFile::remove(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/messages/group/message_" + QString::number(group_id) + ".json");
        emit clearMessagesAfterDelete(group_id);
    }
    QJsonObject json;
    readJsonFromFile(pathToGroupInfo,json);
    QJsonArray members = json["members"].toArray();
    QJsonArray newMembers;

    for (const QJsonValue &value : members) {
        QJsonObject member = value.toObject();
        int memberId = member["id"].toInt();
        if (memberId == user_id) {
            logger->log(Logger::DEBUG,"responsehandler.cpp::deleteUserFromInfoFile", "User with id: " + QString::number(user_id) + " successfuly removed from group with id: " + QString::number(group_id));
            continue;
        }

        newMembers.append(member);
    }
    json["members"] = newMembers;

    if (!writeJsonToFile(pathToGroupInfo,json)) {
        logger->log(Logger::WARN,"responsehandler.cpp::deleteUserFromInfoFile", "Group info file not open for write");
        return 1;
    }

    return 0;
}

bool ResponseHandler::writeJsonToFile(const QString &path, const QJsonObject &json)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        logger->log(Logger::DEBUG, "responsehandler.cpp::writeJsonToFile", "Open file failed: " + path);
        return false;
    }
    file.write(QJsonDocument(json).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ResponseHandler::readJsonFromFile(const QString &path, QJsonObject &jsonForWriting)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        logger->log(Logger::WARN, "responsehandler.cpp::readJsonFromFile",
                    QString("Failed to open file for reading: %1, error: %2")
                        .arg(path, file.errorString()));
        return false;
    }

    QJsonParseError parseError;
    jsonForWriting = QJsonDocument::fromJson(file.readAll(), &parseError).object();
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        logger->log(Logger::WARN, "responsehandler.cpp::readJsonFromFile",
                    QString("JSON parse error in %1: %2")
                        .arg(path, parseError.errorString()));
        return false;
    }
    return true;
}
