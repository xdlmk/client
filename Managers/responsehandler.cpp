#include "responsehandler.h"
#include "addMembers.qpb.h"

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
    auth::LoginResponse response;

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

    QByteArray nonce = response.nonce();
    QByteArray salt = response.salt();
    QByteArray encrypted_private_key = response.encryptedPrivateKey();

    if (success == "ok") {
        common::Identifiers identifiers;
        identifiers.setUserId(userId);
        emit sendData("identifiers", identifiers.serialize(&serializer));
        emit transferUserNameAndIdAfterLogin(userlogin, userId);
        emit loginSuccess(userlogin, userId);
        emit checkAndSendAvatarUpdate(avatar_url, userId, "personal");
        emit addAccount(userlogin, password, userId);
        emit savePrivateKey(encrypted_private_key, salt, nonce);
        emit updatingChats();
    } else if (success == "poor") {
        emit loginFail();
    }
}

void ResponseHandler::processingRegistrationResults(const QByteArray &regResultsData)
{
    QProtobufSerializer serializer;
    auth::RegisterResponse response;

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
    search::SearchResponse response;

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
    profile::EditProfileResponse response;

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
    avatars::AvatarsUpdateResponse response;

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

void ResponseHandler::processingGroupInfoSave(const QList<chats::GroupInfoItem> &receivedGroupInfo)
{
    logger->log(Logger::INFO, "responsehandler.cpp::processingGroupInfoSave", "processingGroupInfoSave has begun");

    QString basePath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/groupsInfo";
    QDir saveDir(basePath);
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    } else {
        saveDir.removeRecursively();
        saveDir.mkpath(".");
    }

    for (const auto &groupItem : receivedGroupInfo) {
        quint64 group_id = groupItem.groupId();

        QString savePath = basePath + "/" + QString::number(group_id) + ".pb";

        QProtobufSerializer serializer;
        QByteArray data = groupItem.serialize(&serializer);

        if (!saveProtoObjectToFile(savePath, data)) {
            logger->log(Logger::WARN, "responsehandler.cpp::processingGroupInfoSave", "saveProtoObjectToFile returned false");
        }
    }
}

void ResponseHandler::processingDialogsInfoSave(const QList<chats::DialogInfoItem> &receivedDialogInfo)
{
    logger->log(Logger::INFO, "responsehandler.cpp::processingDialogsInfoSave", "processingDialogsInfoSave has begun");

    QString basePath = QCoreApplication::applicationDirPath() + "/.data/" + QString::number(activeUserId) + "/dialogsInfo";
    QDir saveDir(basePath);
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    } else {
        saveDir.removeRecursively();
        saveDir.mkpath(".");
    }

    for (const auto &dialogItem : receivedDialogInfo) {
        quint64 user_id = dialogItem.userId();

        QString savePath = basePath + "/" + QString::number(user_id) + ".pb";

        QProtobufSerializer serializer;
        QByteArray data = dialogItem.serialize(&serializer);

        if (!saveProtoObjectToFile(savePath, data)) {
            logger->log(Logger::WARN, "responsehandler.cpp::processingDialogsInfoSave", "saveProtoObjectToFile returned false");
        }
    }
}

void ResponseHandler::processingDeleteGroupMember(const QByteArray &receivedDeleteMemberFromGroupData)
{
    QProtobufSerializer serializer;
    groups::DeleteMemberResponse response;

    if (!response.deserialize(&serializer, receivedDeleteMemberFromGroupData)) {
        logger->log(Logger::WARN, "responsehandler.cpp::processingDeleteGroupMember", "Failed to deserialize DeleteMemberResponse");
        return;
    }

    if (response.errorCode() == 0) {
        quint64 group_id = response.groupId();
        quint64 deleted_user_id = response.deletedUserId();
        int error_code = deleteUserFromInfoFile(group_id, deleted_user_id);

        if (error_code == 0) {
            logger->log(Logger::INFO, "responsehandler.cpp::processingDeleteGroupMember",
                        "Member with user_id: " + QString::number(deleted_user_id) + " successfully removed from info list");
            emit getGroupMembers(group_id);
        } else {
            logger->log(Logger::WARN, "responsehandler.cpp::processingDeleteGroupMember",
                        "Member with user_id: " + QString::number(deleted_user_id) + " not removed from info list");
        }

    } else if (response.errorCode() == 1) {
        logger->log(Logger::WARN, "responsehandler.cpp::processingDeleteGroupMember",
                    "Member with user_id: " + QString::number(response.deletedUserId()) + " is not a member of the group");
    } else if (response.errorCode() == 2) {
        logger->log(Logger::WARN, "responsehandler.cpp::processingDeleteGroupMember",
                    "The active user does not have sufficient rights to perform this operation");
    }
}

void ResponseHandler::processingAddGroupMember(const QByteArray &receivedAddMemberFromGroupData)
{
    QProtobufSerializer serializer;
    groups::AddGroupMembersResponse response;

    if (!response.deserialize(&serializer, receivedAddMemberFromGroupData)) {
        logger->log(Logger::WARN, "responsehandler.cpp::processingAddGroupMember",
                    "Failed to deserialize AddGroupMembersResponse");
        return;
    }

    quint64 group_id = response.groupId();

    for (const auto &member : response.addedMembers()) {
        if (member.userId() == static_cast<quint64>(this->activeUserId)) {
            updatingChats();
            return;
        }
    }

    QString pathToGroupInfo = QCoreApplication::applicationDirPath() + "/.data/" +
                              QString::number(activeUserId) + "/groupsInfo/" +
                              QString::number(group_id) + ".pb";

    QByteArray protoData;
    if (!readProtoObjectFromFile(pathToGroupInfo, protoData)) {
        logger->log(Logger::WARN, "responsehandler.cpp::processingAddGroupMember",
                    "Failed to read proto object from file: " + pathToGroupInfo);
        return;
    }

    chats::GroupInfoItem groupInfo;
    if (!groupInfo.deserialize(&serializer, protoData)) {
        logger->log(Logger::WARN, "responsehandler.cpp::processingAddGroupMember",
                    "Failed to deserialize GroupInfoItem from file data");
        return;
    }
    QList<chats::GroupMember> members = groupInfo.members();
    for (const auto &addedMember : response.addedMembers()) {
        chats::GroupMember newMember;
        newMember.setId_proto(addedMember.userId());
        newMember.setUsername(addedMember.username());
        newMember.setStatus(addedMember.status());
        newMember.setAvatarUrl(addedMember.avatarUrl());
        members.append(newMember);
    }
    groupInfo.setMembers(members);

    QByteArray outData = groupInfo.serialize(&serializer);

    if (saveProtoObjectToFile(pathToGroupInfo, outData)) {
        emit getGroupMembers(group_id);
    } else {
        logger->log(Logger::WARN, "responsehandler.cpp::processingAddGroupMember",
                    "Failed to save updated GroupInfoItem to file.");
    }
}

int ResponseHandler::deleteUserFromInfoFile(const int &group_id, const int &user_id)
{
    QString pathToGroupInfo = QCoreApplication::applicationDirPath() + "/.data/" +
                              QString::number(activeUserId) + "/groupsInfo/" +
                              QString::number(group_id) + ".pb";

    if (user_id == this->activeUserId) {
        QFile::remove(pathToGroupInfo);
        QFile::remove(QCoreApplication::applicationDirPath() + "/.data/" +
                      QString::number(activeUserId) + "/messages/group/message_" +
                      QString::number(group_id) + ".pb");
        emit clearMessagesAfterDelete(group_id);
    }

    QByteArray protoData;
    if (!readProtoObjectFromFile(pathToGroupInfo, protoData)) {
        logger->log(Logger::WARN, "responsehandler.cpp::deleteUserFromInfoFile", "Failed to read proto object from file");
        return 1;
    }

    QProtobufSerializer serializer;
    chats::GroupInfoItem groupInfo;
    if (!groupInfo.deserialize(&serializer, protoData)) {
        logger->log(Logger::WARN, "responsehandler.cpp::deleteUserFromInfoFile", "Failed to deserialize proto object");
        return 1;
    }

    QList<chats::GroupMember> newMembers;
    const QList<chats::GroupMember> &members = groupInfo.members();
    for (const auto &member : members) {
        if (static_cast<int>(member.id_proto()) == user_id) {
            logger->log(Logger::DEBUG, "responsehandler.cpp::deleteUserFromInfoFile",
                        "User with id: " + QString::number(user_id) +
                            " successfully removed from group with id: " + QString::number(group_id));
            continue;
        }
        newMembers.append(member);
    }
    groupInfo.setMembers(newMembers);

    QByteArray outData = groupInfo.serialize(&serializer);

    if (!saveProtoObjectToFile(pathToGroupInfo, outData)) {
        logger->log(Logger::WARN, "responsehandler.cpp::deleteUserFromInfoFile", "Group info file not open for write");
        return 1;
    }

    return 0;
}

bool ResponseHandler::saveProtoObjectToFile(const QString &path, const QByteArray &data)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        return true;
    } else {
        logger->log(Logger::WARN, "responsehandler.cpp::saveDialogInfoToFile", "Failed to open file for writing: " + path);
        return false;
    }
}

bool ResponseHandler::readProtoObjectFromFile(const QString &path, QByteArray &data)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        data = file.readAll();
        file.close();
        return true;
    } else {
        logger->log(Logger::WARN, "responsehandler.cpp::readProtoObjectFromFile",
                    QString("Failed to open file for reading: %1, error: %2")
                        .arg(path, file.errorString()));
        return false;
    }
}
