#ifndef RESPONSEHANDLER_H
#define RESPONSEHANDLER_H

#include <QObject>

#include <QCoreApplication>
#include <QDir>
#include <QFile>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "Core/logger.h"

class ResponseHandler : public QObject
{
    Q_OBJECT
public:
    explicit ResponseHandler(QObject *parent = nullptr);
    void setLogger(Logger *logger);
    void setActiveUser(const QString &userLogin,const int &userId);
public slots:
    void processingLoginResults(const QJsonObject &loginResultsJson);
    void processingRegistrationResults(const QJsonObject &regResultsJson);
    void processingSearchData(const QJsonObject &searchDataJson);
    void processingEditProfile(const QJsonObject &editResultsJson);
    void processingAvatarsUpdate(const QJsonObject &avatarsUpdateJson);

signals:
    void transferUserNameAndIdAfterLogin(const QString &activeUserName,const int &activeUserId);
    void checkAndSendAvatarUpdate(const QString &avatar_url,const int &user_id,const QString& type);
    void loginSuccess(QString &name, int &user_id);
    void loginFail();
    void addAccount(const QString &login, const QString &password, int userId);
    void updatingChats();

    void registrationSuccess();
    void registrationFail(QString error);

    void newSearchUser(QString &userlogin,int &id);

    void editUniqueError();
    void unknownError();
    void editUserlogin(QString editInformation);
    void editPhoneNumber(QString editInformation);
    void editName(QString editInformation);

    void getChatsInfo();

private:
    Logger *logger;
    QString activeUserLogin;
    int activeUserId;
};

#endif // RESPONSEHANDLER_H
