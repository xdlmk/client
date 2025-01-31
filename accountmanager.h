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

class AccountManager : public QObject
{
    Q_OBJECT
public:
    explicit AccountManager(NetworkManager* networkManager,QObject *parent = nullptr);
    void login(const QString &login, const QString &password);
    void registerAccount(const QString &login, const QString &password); // void reg(QString login,QString password);
    void logout(); // void logout()
    void sendSearchToServer(const QString &searchable);
    void clientChangeAccount();
    void createConfigFile(const QString &userLogin, const QString &userPassword); // void createConfigFile(QString userLogin,QString userPassword);

public slots:
    void processingLoginResultsFromServer(const QJsonObject &loginResultsJson);
    void processingRegistrationResultsFromServer(const QJsonObject &regResultsJson);
    void processingPersonalMessageFromServer(const QJsonObject &personalMessageJson);
    void processingSearchDataFromServer(const QJsonObject &searchDataJson);
    void processingChatsUpdateDataFromServer(QJsonObject &chatsUpdateDataJson);

    void changeActiveAccount(QString username);

    void setActiveUser(const QString &userName,const int &userId);

signals:
    void newUser(QString username);
    void changeAccount(QString username,QString password);
    void newAccountLoginSuccessful(QString& pathToMessagesOnLocal);

    void checkingChatAvailability(QString &login);

    void saveMessageFromDatabase(QJsonObject &chatsUpdateDataJson);
    void saveMessageToJson(QString &userlogin, QString &message, QString &out, QString &time,
                           QString &fullDate, int message_id, int dialog_id, int id);

    void checkActiveDialog(QString login);

    void loginSuccess(QString &name); // void loginSuccess(QString name);
    void loginFail();
    void transferUserNameAndIdAfterLogin(const QString &activeUserName,const int &activeUserId);

    void registrationSuccess(); // void regSuccess();
    void registrationFail(QString &error); // void regFail(QString error);

    void clientLogout();

    void newSearchUser(QString &userlogin,int &id);
private:
    int user_id;
    QString activeUserName;

    NetworkManager* networkManager;

    void updatingChats();
};

#endif // ACCOUNTMANAGER_H
