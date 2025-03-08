#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QVariantList>
#include <QStandardPaths>
#include <QSettings>
#include <QImage>
#include <QCoreApplication>
#include <QDir>

#include "Network/networkmanager.h"
#include "Core/configmanager.h"
#include "Managers/responsehandler.h"
#include "Core/logger.h"

class AccountManager : public QObject
{
    Q_OBJECT
public:
    explicit AccountManager(NetworkManager* networkManager,QObject *parent = nullptr);
    void login(const QString login, const QString password);
    void registerAccount(const QString login, const QString password);
    void logout();
    void clientChangeAccount();
public slots:
    void checkAndSendAvatarUpdate(const QString &avatar_url,const int &user_id);
    void updatingChats();

    void sendAvatarsUpdate();
    void sendSearchToServer(const QString &searchable);
    void sendEditProfileRequest(const QString editable,const QString editInformation);

    void setActiveUser(const QString &userName,const int &userId);
    void setLogger(Logger* logger);

    void createGroup(const QString& groupName, const QVariantList &selectedContacts);
    void saveGroupInfo(const QJsonObject &receivedGroupInfoJson);
    void getGroupMembers(const int& group_id,const QString& group_name);

    void getContactList();
    void showContacts();
signals:
    void newUser(QString username,int user_id);
    void changeAccount(QString username,QString password);
    void changeActiveAccount(QString username);

    void checkConfigFile();
    void checkingChatAvailability(QString &login, const QString &flag);

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

    void loadContacts(QVariantList contactsList);
    void loadGroupMembers(QVariantList membersList);

    void processingLoginResultsFromServer(const QJsonObject &loginResultsJson);
    void processingRegistrationResultsFromServer(const QJsonObject &regResultsJson);
    void processingSearchDataFromServer(const QJsonObject &searchDataJson);
    void processingEditProfileFromServer(const QJsonObject &editResultsJson);
    void processingAvatarsUpdateFromServer(const QJsonObject &avatarsUpdateJson);
private:
    bool isAvatarUpToDate(QString avatar_url,int user_id);

    int user_id;
    QString activeUserName;

    NetworkManager* networkManager;
    Logger* logger;
    ConfigManager configManager;
    ResponseHandler responseHandler;

    void setupConfigManager();
    void setupResponseHandler();
};

#endif // ACCOUNTMANAGER_H
