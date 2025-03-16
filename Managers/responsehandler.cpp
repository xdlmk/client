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
    QString name = loginResultsJson["name"].toString();
    QString password = loginResultsJson["password"].toString();
    int userId = loginResultsJson["user_id"].toInt();

    QString avatar_url = loginResultsJson["avatar_url"].toString();

    if(success == "ok")
    {
        emit transferUserNameAndIdAfterLogin(name,userId);
        emit checkAndSendAvatarUpdate(avatar_url, userId, "personal");
        emit loginSuccess(name, userId);
        emit addAccount(name,password,userId);
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
        logger->log(Logger::ERROR,"responsehandler.cpp::processingAvatarsUpdate","Urls json array is empty");
        return;
    }
    if (groupsAvatarsArray.isEmpty()) {
        logger->log(Logger::ERROR,"responsehandler.cpp::processingAvatarsUpdate","Groups urls json array is empty");
        return;
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
