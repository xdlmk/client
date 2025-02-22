#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QStandardPaths>
#include <QSettings>
#include <QImage>
#include <QCoreApplication>
#include <QDir>

#include "networkmanager.h"
#include "Logger/logger.h"

class AccountManager : public QObject
{
    Q_OBJECT
public:
    explicit AccountManager(NetworkManager* networkManager,QObject *parent = nullptr);
    void login(const QString login, const QString password);
    void registerAccount(const QString login, const QString password);
    void logout();
    void clientChangeAccount();
    void createConfigFile(const QString &userLogin, const QString &userPassword);

public slots:
    void checkAndSendAvatarUpdate(const QString &avatar_url,const int &user_id);

    void checkConfigFile(const QSettings& settings);

    void processingLoginResultsFromServer(const QJsonObject &loginResultsJson);
    void processingRegistrationResultsFromServer(const QJsonObject &regResultsJson);
    void processingPersonalMessageFromServer(const QJsonObject &personalMessageJson);
    void processingSearchDataFromServer(const QJsonObject &searchDataJson);
    void processingChatsUpdateDataFromServer(QJsonObject &chatsUpdateDataJson);
    void processingEditProfileFromServer(const QJsonObject &editResultsJson);
    void processingAvatarsUpdateFromServer(const QJsonObject &avatarsUpdateJson);

    void sendAvatarsUpdate();
    void sendSearchToServer(const QString &searchable);
    void sendEditProfileRequest(const QString editable,const QString editInformation);

    void changeActiveAccount(QString username);

    void setActiveUser(const QString &userName,const int &userId);
    void setLogger(Logger* logger);

signals:
    void newUser(QString username);
    void changeAccount(QString username,QString password);
    void newAccountLoginSuccessful(QString& pathToMessagesOnLocal);

    void checkingChatAvailability(QString &login);

    void saveMessageFromDatabase(QJsonObject &chatsUpdateDataJson);
    void saveMessageToJson(QString &userlogin, QString &message, QString &out, QString &time,
                           QString &fullDate, int message_id, int dialog_id, int id, QString &fileUrl);

    void checkActiveDialog(QString login,QString message, QString out,
                           QString time,QString fileName,QString fileUrl);

    void loginSuccess(QString &name, int &user_id);
    void loginFail();
    void transferUserNameAndIdAfterLogin(const QString &activeUserName,const int &activeUserId);

    void registrationSuccess();
    void registrationFail(QString error);

    void editUserlogin(QString editInformation);
    void editPhoneNumber(QString editInformation);
    void editName(QString editInformation);
    void editUniqueError();
    void unknownError();

    void clientLogout();
    void sendAvatarUrl(const QString &avatar_url,const int& user_id);

    void newSearchUser(QString &userlogin,int &id);
private:
    bool isAvatarUpToDate(QString avatar_url,int user_id);

    int user_id;
    QString activeUserName;

    NetworkManager* networkManager;
    Logger* logger;

    void updatingChats();
};

#endif // ACCOUNTMANAGER_H
