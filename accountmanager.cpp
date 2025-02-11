#include "accountmanager.h"

AccountManager::AccountManager(NetworkManager* networkManager,QObject *parent)
    : QObject{parent}
{
    this->networkManager = networkManager;
}

void AccountManager::login(const QString login, const QString password)
{
    QJsonObject loginDataJson;
    loginDataJson["flag"] = "login";
    loginDataJson["login"] = login;
    loginDataJson["password"] = password;
    networkManager->sendData(loginDataJson);
}

void AccountManager::checkConfigFile(const QSettings& settings)
{
    logger->log(Logger::INFO,"accountmanager.cpp::checkConfigFile","checkConfigFile has begun");
    int total = settings.value("total",0).toInt();
    for( int i = 1; i<=total;i++)
    {
        emit newUser(settings.value("login"+QString::number(i), "").toString());
    }
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

    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";
    QSettings configFile(configFilePath, QSettings::IniFormat);
    int active_account = configFile.value("active_account",0).toInt();
    configFile.remove("success"+QString::number(active_account));
    configFile.remove("login"+QString::number(active_account));
    configFile.remove("password"+QString::number(active_account));

    for (int i = active_account + 1; i <= configFile.value("total",0).toInt(); ++i) {
        QString success = configFile.value("success" + QString::number(i), "").toString();
        QString login = configFile.value("login" + QString::number(i), "").toString();
        QString password = configFile.value("password" + QString::number(i), "").toString();

        configFile.setValue("success" + QString::number(i - 1), success);
        configFile.setValue("login" + QString::number(i - 1), login);
        configFile.setValue("password" + QString::number(i - 1), password);

        configFile.remove("success" + QString::number(i));
        configFile.remove("login" + QString::number(i));
        configFile.remove("password" + QString::number(i));
    }



    if(configFile.value("total",0).toInt()>=2)
    {
        configFile.setValue("total",configFile.value("total",0).toInt()-1);
        configFile.setValue("active_account",1);
        active_account = configFile.value("active_account",0).toInt();

        QString success = configFile.value("success"+QString::number(active_account),"").toString();
        if(success == "ok")
        {
            QString login = configFile.value("login"+QString::number(active_account), "").toString();
            QString password = configFile.value("password"+QString::number(active_account), "").toString();

            this->login(login,password);
        }
    }
    else
    {
        QFile confFile(configFilePath);
        if (confFile.exists()) {
            if (confFile.remove()) {
                logger->log(Logger::INFO,"accountmanager.cpp::logout","Config file removed successfully");
            } else {
                logger->log(Logger::ERROR,"accountmanager.cpp::logout","Failed to remove config file");
            }
        } else {
            logger->log(Logger::INFO,"accountmanager.cpp::logout","Config file does not exist");
        }
    }
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

void AccountManager::changeActiveAccount(QString username)
{
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";
    QSettings configFile(configFilePath, QSettings::IniFormat);

    int total = configFile.value("total", 0).toInt();

    for (int i = 1; i <= total; ++i) {
        QString currentLogin = configFile.value("login"+ QString::number(i), "").toString();

        if (currentLogin == username) {
            configFile.setValue("active_account", i);
            logger->log(Logger::INFO,"accountmanager.cpp::changeActiveAccount","Active account changed to: " + QString::number(i));
            QString password = configFile.value("password"+ QString::number(i),"").toString();
            emit changeAccount(username,password);

            return;
        }
    }
    logger->log(Logger::WARN,"accountmanager.cpp::changeActiveAccount","User not found: " + username);
}

void AccountManager::clientChangeAccount()
{
    QJsonObject json;
    json["flag"]= "logout";
    networkManager->sendData(json);
}

void AccountManager::createConfigFile(const QString &userLogin, const QString &userPassword)
{
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";
    QSettings settings(configFilePath,QSettings::IniFormat);
    int total = settings.value("total",0).toInt();
    bool already_exists = false;
    for( int i = 1; i<=total; i++ )
    {
        if (userLogin == settings.value("login"+ QString::number(i)))
        {
            already_exists = true;
            break;
        }
    }
    if (!already_exists)
    {
        if(settings.value("total",0).toInt() == 0)
        {
            settings.setValue("total", 1);
            settings.setValue("active_account",1);
            settings.setValue("success1","ok");
            settings.setValue("login1", userLogin);
            settings.setValue("password1", userPassword);
            logger->log(Logger::INFO,"accountmanager.cpp::createConfigFile","Config file created! 1 users");
        }
        else if(settings.value("total",0).toInt() == 1)
        {
            settings.setValue("total", 2);
            settings.setValue("active_account",2);
            settings.setValue("success2","ok");
            settings.setValue("login2", userLogin);
            settings.setValue("password2", userPassword);
            logger->log(Logger::INFO,"accountmanager.cpp::createConfigFile","Config file created! 2 users");
        }
        else if(settings.value("total",0).toInt() == 2)
        {
            settings.setValue("total", 3);
            settings.setValue("active_account",3);
            settings.setValue("success3","ok");
            settings.setValue("login3", userLogin);
            settings.setValue("password3", userPassword);
            logger->log(Logger::INFO,"accountmanager.cpp::createConfigFile","Config file created! 3 users");
        }
    }
    else logger->log(Logger::INFO,"accountmanager.cpp::createConfigFile","Account is recorded in the config");
    checkConfigFile(settings);
}

void AccountManager::processingLoginResultsFromServer(const QJsonObject &loginResultsJson)
{
    QString success = loginResultsJson["success"].toString();
    QString name = loginResultsJson["name"].toString();
    QString password = loginResultsJson["password"].toString();
    int userId = loginResultsJson["user_id"].toInt();

    QString imageString = loginResultsJson["profileImage"].toString();
    QByteArray imageData = QByteArray::fromBase64(imageString.toLatin1());

    if(success == "ok")
    {
        QImage image;
        image.loadFromData(imageData);
        QString filePath(QCoreApplication::applicationDirPath() + "/resources/images/avatar.png");
        if (!image.save(filePath)) {
            logger->log(Logger::ERROR,"accountmanager.cpp::processingLoginResultsFromServer","Failed to save image");
        }

        QDir dir(QCoreApplication::applicationDirPath() + "/resources/messages");
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        emit transferUserNameAndIdAfterLogin(name,userId);

        QString pathToMessages = QCoreApplication::applicationDirPath() + "/resources/messages/message_" + activeUserName + ".json";
        QFile file(pathToMessages);

        if (!file.exists()) {
            logger->log(Logger::INFO,"accountmanager.cpp::processingLoginResultsFromServer","File does not exist, create a new one");

            if (file.open(QIODevice::WriteOnly)) {
                QJsonArray emptyArray;
                QJsonDocument doc(emptyArray);
                file.write(doc.toJson());
                file.close();
            } else {
                logger->log(Logger::ERROR,"accountmanager.cpp::processingLoginResultsFromServer","The file is not created");
            }
        } else {
            logger->log(Logger::INFO,"accountmanager.cpp::processingLoginResultsFromServer","File exist");
        }


        emit loginSuccess(name);
        emit newAccountLoginSuccessful(pathToMessages);
        createConfigFile(name,password);
        updatingChats();
    }
    else if(success == "poor")
    {
        emit loginFail();
    }

}

void AccountManager::processingRegistrationResultsFromServer(const QJsonObject &regResultsJson)
{
    logger->log(Logger::INFO,"accountmanager.cpp::processingRegistrationResultsFromServer","processingRegistrationResultsFromServer has begun");
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

void AccountManager::processingPersonalMessageFromServer(const QJsonObject &personalMessageJson)
{
    QString message = personalMessageJson["message"].toString();
    QString time = personalMessageJson["time"].toString();
    int message_id = personalMessageJson["message_id"].toInt();
    int dialog_id = personalMessageJson["dialog_id"].toInt();
    if(personalMessageJson.contains("fileUrl"))
    {
        getFile(personalMessageJson["fileUrl"].toString());
    }
    QString login;
    QString out = "";
    QString fullDate = "not:done(processingPersonalMessageFromServer)";
    int id;

    logger->log(Logger::INFO,"accountmanager.cpp::processingPersonalMessageFromServer","Personal message received");

    if(personalMessageJson.contains("receiver_login"))
    {
        login = personalMessageJson["receiver_login"].toString();
        id = personalMessageJson["receiver_id"].toInt();
        out = "out";
        emit saveMessageToJson(login, message, out, time,fullDate, message_id,dialog_id,id);
    }
    else
    {
        login = personalMessageJson["sender_login"].toString();
        id = personalMessageJson["sender_id"].toInt();
        emit saveMessageToJson(login, message, out, time, fullDate, message_id,dialog_id,id);
    }
    emit checkActiveDialog(login,message,out,time);
}

void AccountManager::processingSearchDataFromServer(const QJsonObject &searchDataJson)
{
    logger->log(Logger::INFO,"accountmanager.cpp::processingSearchDataFromServer","processingSearchDataFromServer has begun");
    QJsonArray resultsArray = searchDataJson.value("results").toArray();
    for (const QJsonValue &value : resultsArray) {
        QJsonObject userObj = value.toObject();
        int id = userObj.value("id").toInt();
        QString userlogin = userObj.value("userlogin").toString();
        emit newSearchUser(userlogin,id);
    }
}

void AccountManager::processingChatsUpdateDataFromServer(QJsonObject &chatsUpdateDataJson)
{
    logger->log(Logger::INFO,"accountmanager.cpp::processingChatsUpdateDataFromServer","processingChatsUpdateDataFromServer has begun");
    emit saveMessageFromDatabase(chatsUpdateDataJson);
}

void AccountManager::processingEditProfileFromServer(const QJsonObject &editResultsJson)
{
    logger->log(Logger::INFO,"accountmanager.cpp::processingEditProfileFromServer","processingEditProfileFromServer has begun");

    QString status = editResultsJson["status"].toString();
    if(status == "poor"){
        QString error = editResultsJson["error"].toString();
        if(error == "Unique error"){
            emit editUniqueError();
            logger->log(Logger::ERROR,"accountmanager.cpp::processingEditProfileFromServer","Information changed was not unique");
            return;
        }
        logger->log(Logger::WARN,"accountmanager.cpp::processingEditProfileFromServer","Unknown request error");
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

void AccountManager::setActiveUser(const QString &userName, const int &userId)
{
    activeUserName=userName;
    user_id = userId;
}

void AccountManager::setLogger(Logger *logger)
{
    this->logger = logger;
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


