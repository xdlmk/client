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

void ResponseHandler::processingLoginResults(const QByteArray &loginResultsData)
{
    logger->log(Logger::INFO, "responsehandler.cpp::processingLoginResults", "processingLoginResults has begun");

    QProtobufSerializer serializer;
    messages::LoginResponse response;

    if (!response.deserialize(&serializer, loginResultsData)) {
        logger->log(Logger::ERROR, "responsehandler.cpp::processingLoginResults", "Failed to parse login response");
        emit loginFail();
        return;
    }

    QString success = response.success();
    QString userlogin = response.userlogin();
    QString password = response.password();
    int userId = response.userId();
    QString avatar_url = response.avatarUrl();

    if (success == "ok") {
        messages::Identifiers identifiers;
        identifiers.setUserId(userId);
        emit sendData("identifiers", identifiers.serialize(&serializer));
        emit transferUserNameAndIdAfterLogin(userlogin, userId);
        emit loginSuccess(userlogin, userId);
        emit checkAndSendAvatarUpdate(avatar_url, userId, "personal");
        emit addAccount(userlogin, password, userId);
        emit updatingChats();
    } else if (success == "poor") {
        emit loginFail();
    }
}

void ResponseHandler::processingRegistrationResults(const QByteArray &regResultsData)
{
    QProtobufSerializer serializer;
    messages::RegisterResponse response;

    if (!response.deserialize(&serializer, regResultsData)) {
        logger->log(Logger::ERROR, "responsehandler.cpp::processingRegistrationResults", "Failed to parse registration response");
        emit registrationFail("Failed to parse response");
        return;
    }
    QString success = response.success();
    QString errorMes = response.errorMes();

    if (success == "ok") {
        emit registrationSuccess();
    } else if (success == "poor") {
        emit registrationFail(errorMes);
    }
}

void ResponseHandler::processingSearchData(const QByteArray &searchData)
{
    logger->log(Logger::INFO, "responsehandler.cpp::processingSearchData", "processingSearchDataFromServer has begun");

    QProtobufSerializer serializer;
    messages::SearchResponse response;

    if (!response.deserialize(&serializer, searchData)) {
        logger->log(Logger::ERROR, "responsehandler.cpp::processingSearchData", "Failed to parse search response");
        return;
    }

    for (const auto &result : response.results()) {
        int id = result.id_proto();
        QString userlogin = result.userlogin();
        QString avatar_url = result.avatarUrl();

        emit checkAndSendAvatarUpdate(avatar_url, id, "personal");
        emit newSearchUser(userlogin, id);
    }
}

void ResponseHandler::processingEditProfile(const QByteArray &editResultsData)
{
    logger->log(Logger::INFO, "responsehandler.cpp::processingEditProfile", "processingEditProfileFromServer has begun");

    QProtobufSerializer serializer;
    messages::EditProfileResponse response;

    if (!response.deserialize(&serializer, editResultsData)) {
        logger->log(Logger::ERROR, "responsehandler.cpp::processingEditProfile", "Failed to parse edit profile response");
        emit unknownError();
        return;
    }

    QString status = response.status();
    if (status == "poor") {
        QString error = response.error();
        if (error == "Unique error") {
            emit editUniqueError();
            logger->log(Logger::ERROR, "responsehandler.cpp::processingEditProfile", "Information changed was not unique");
            return;
        }
        logger->log(Logger::WARN, "responsehandler.cpp::processingEditProfile", "Unknown request error");
        emit unknownError();
        return;
    }

    QString editable = response.editable();
    QString editInformation = response.editInformation();

    if (editable == "Username") {
        emit editUserlogin(editInformation);
    } else if (editable == "Phone number") {
        emit editPhoneNumber(editInformation);
    } else if (editable == "Name") {
        emit editName(editInformation);
    }
}

void ResponseHandler::processingAvatarsUpdate(const QByteArray &avatarsUpdateData)
{
    logger->log(Logger::INFO, "responsehandler.cpp::processingAvatarsUpdate", "processingAvatarsUpdate has begun");

    QProtobufSerializer serializer;
    messages::AvatarsUpdateResponse response;

    if (!response.deserialize(&serializer, avatarsUpdateData)) {
        logger->log(Logger::ERROR, "responsehandler.cpp::processingAvatarsUpdate", "Failed to parse avatars update response");
        return;
    }

    if (response.avatars().isEmpty()) {
        logger->log(Logger::WARN, "responsehandler.cpp::processingAvatarsUpdate", "Urls protobuf array is empty");
    }
    if (response.groupsAvatars().isEmpty()) {
        logger->log(Logger::WARN, "responsehandler.cpp::processingAvatarsUpdate", "Groups urls protobuf array is empty");
    }

    for (const auto &avatarItem : response.avatars()) {
        int id = avatarItem.id_proto();
        QString avatarUrl = avatarItem.avatarUrl();
        emit checkAndSendAvatarUpdate(avatarUrl, id, "personal");
    }

    for (const auto &groupAvatarItem : response.groupsAvatars()) {
        int id = groupAvatarItem.groupId();
        QString avatarUrl = groupAvatarItem.avatarUrl();
        emit checkAndSendAvatarUpdate(avatarUrl, id, "group");
    }

    emit getChatsInfo();
}

void ResponseHandler::processingGroupInfoSave(const QList<messages::GroupInfoItem> &receivedGroupInfo)
{
    logger->log(Logger::INFO, "responsehandler.cpp::processingGroupInfoSave", "processingGroupInfoSave has begun");

    QDir saveDir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo");
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    } else {
        saveDir.removeRecursively();
        saveDir.mkpath(".");
    }

    for (const auto &groupItem : receivedGroupInfo) {
        quint64 group_id = groupItem.groupId();

        QString savePath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo/" + QString::number(group_id) + ".json";

        QJsonObject groupInfoJson;
        groupInfoJson["group_id"] = static_cast<int>(groupItem.groupId());
        groupInfoJson["group_name"] = groupItem.groupName();
        groupInfoJson["avatar_url"] = groupItem.avatarUrl();

        QJsonArray membersArray;
        for (const auto &memberItem : groupItem.members()) {
            QJsonObject memberJson;
            memberJson["id"] = static_cast<int>(memberItem.id_proto());
            memberJson["username"] = memberItem.username();
            memberJson["status"] = memberItem.status();
            memberJson["avatar_url"] = memberItem.avatarUrl();
            membersArray.append(memberJson);
        }
        groupInfoJson["members"] = membersArray;

        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(groupInfoJson).toJson(QJsonDocument::Compact));
            file.close();
        } else {
            logger->log(Logger::WARN, "responsehandler.cpp::processingGroupInfoSave", "Failed to open file for writing");
        }
    }
}

void ResponseHandler::processingDialogsInfoSave(const QList<messages::DialogInfoItem> &receivedDialogInfo)
{
    logger->log(Logger::INFO, "responsehandler.cpp::processingDialogsInfoSave", "processingDialogsInfoSave has begun");

    QDir saveDir(QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo");
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    } else {
        saveDir.removeRecursively();
        saveDir.mkpath(".");
    }

    for (const auto &dialogItem : receivedDialogInfo) {
        quint64 user_id = dialogItem.userId();

        QString savePath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo/" + QString::number(user_id) + ".json";

        QJsonObject dialogInfoJson;
        dialogInfoJson["user_id"] = static_cast<int>(dialogItem.userId());
        dialogInfoJson["username"] = dialogItem.username();
        dialogInfoJson["userlogin"] = dialogItem.userlogin();
        dialogInfoJson["phone_number"] = dialogItem.phoneNumber();
        dialogInfoJson["avatar_url"] = dialogItem.avatarUrl();
        dialogInfoJson["created_at"] = dialogItem.createdAt();

        if (!writeJsonToFile(savePath, dialogInfoJson)) {
            logger->log(Logger::WARN, "responsehandler.cpp::processingDialogsInfoSave", "writeJsonToFile returned false");
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
