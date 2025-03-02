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

void AccountManager::checkAndSendAvatarUpdate(const QString &avatar_url, const int &user_id)
{
    logger->log(Logger::INFO,"accountmanager.cpp::checkAndSendAvatarUpdate","checkAndSendAvatarUpdate starts");
    if(!isAvatarUpToDate(avatar_url,user_id)) {
        emit sendAvatarUrl(avatar_url,user_id);
    }
}

void AccountManager::sendAvatarsUpdate()
{
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

void AccountManager::createGroup(const QString &groupName)
{
    QJsonObject createGroupJson;
    createGroupJson["flag"] = "create_group";
    createGroupJson["groupName"] = groupName;
    createGroupJson["creator_id"] = user_id;
    networkManager->sendData(createGroupJson);
}

bool AccountManager::isAvatarUpToDate(QString avatar_url, int user_id)
{
    logger->log(Logger::INFO,"accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate starts");
    QFile avatar(QCoreApplication::applicationDirPath() + "/avatars/" + activeUserName + "/" + QString::number(user_id) + ".png");
    if(!avatar.open(QIODevice::ReadWrite)) {
        logger->log(Logger::INFO,"accountmanager.cpp::isAvatarUpToDate", "Avatar not downloaded");
        return false;
    }
    QString avatarCheckerPath = QCoreApplication::applicationDirPath() + "/.fileChecker/" + activeUserName + "/avatarChecker.json";
    QFile avatarChecker(avatarCheckerPath);
    if(!avatarChecker.open(QIODevice::ReadWrite)) {
        logger->log(Logger::WARN,"accountmanager.cpp::isAvatarUpToDate", "Failed to open avatarChecker");
        return false;
    }
    QByteArray avatarCheckerData = avatarChecker.readAll();
    avatarChecker.close();
    QJsonDocument avatarCheckerDoc = QJsonDocument::fromJson(avatarCheckerData);
    QJsonArray avatarCheckerArray = avatarCheckerDoc.array();
    for (const auto &item : avatarCheckerArray) {
        QJsonObject jsonObject = item.toObject();
        if (jsonObject.contains("user_id") && jsonObject.contains("avatar_url")) {
            if ((jsonObject["user_id"].toInt() == user_id) and (jsonObject["avatar_url"].toString() == avatar_url)) {
                return true;
            }
        }
    }
    logger->log(Logger::DEBUG,"accountmanager.cpp::isAvatarUpToDate", "isAvatarUpToDate return false");
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

    connect(&responseHandler,&ResponseHandler::editUniqueError,this,&AccountManager::editUniqueError);
    connect(&responseHandler,&ResponseHandler::unknownError,this,&AccountManager::unknownError);
    connect(&responseHandler,&ResponseHandler::editUserlogin,this,&AccountManager::editUserlogin);
    connect(&responseHandler,&ResponseHandler::editPhoneNumber,this,&AccountManager::editPhoneNumber);
    connect(&responseHandler,&ResponseHandler::editName,this,&AccountManager::editName);
}

void AccountManager::updatingChats()
{
    QString folderPath = QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal";

    QDir dir(folderPath);

    QStringList fileList = dir.entryList(QStringList() << "message_*.json", QDir::Files);

    QJsonArray jsonArray;
    QJsonArray dialogIdsArray;

    QRegularExpression regex("^message_(.*)\\.json$");


    for (const QString& fileName : fileList) {
        QRegularExpressionMatch match = regex.match(fileName);
        if (match.hasMatch()) {
            QString login = match.captured(1);
            if(login == "") continue;

            emit checkingChatAvailability(login);
            QJsonObject loginObject;


            QFile file(QCoreApplication::applicationDirPath() + "/resources/" + activeUserName + "/personal" +"/message_" + login + ".json");
            if (!file.open(QIODevice::ReadWrite)) {
                logger->log(Logger::INFO,"accountmanager.cpp::updatingChats","File not open with error: " + file.errorString());
                return;
            }

            QByteArray fileData = file.readAll();
            if (!fileData.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(fileData);
                QJsonArray chatHistory = doc.array();

                if (!chatHistory.isEmpty()) {

                    QJsonObject lastMessageObject = chatHistory.last().toObject();
                    loginObject["message_id"] = lastMessageObject["message_id"];
                    loginObject["login"] = login;
                    loginObject["dialog_id"] = lastMessageObject["dialog_id"];
                    dialogIdsArray.append(loginObject["dialog_id"].toInt());

                } else {
                    logger->log(Logger::INFO,"accountmanager.cpp::updatingChats","ChatHistory is empty");
                }
            }
            file.close();

            if (!loginObject.isEmpty()) {
                jsonArray.append(loginObject);
            }
            else {
                logger->log(Logger::INFO,"accountmanager.cpp::updatingChats","loginObject is empty");
            }
        }
    }

    QJsonObject mainObject;
    mainObject["flag"] = "updating_chats";
    if(!fileList.isEmpty()) {
        mainObject["chats"] = jsonArray;
    }
    mainObject["userlogin"] = activeUserName;
    mainObject["dialogIds"] = dialogIdsArray;

    networkManager->sendData(mainObject);
}


