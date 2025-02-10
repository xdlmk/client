#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QStandardPaths>
#include <QSettings>
#include <QCoreApplication>

#include <QImage>

#include "accountmanager.h"
#include "messagemanager.h"
#include "networkmanager.h"
#include "Logger/logger.h"

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);

    AccountManager* getAccountManager();

signals:
    void newMessage(QString username,QString message,QString time, bool isOutgoing);

    void connectionSuccess();
    void connectionError();

    void loginSuccess(QString &name);
    void loginFail();

    void registrationSuccess();
    void registrationFail(QString error);

    void clientLogout();

    void loadingPersonalChat(const QString userlogin);

    void showPersonalChat(QString login,QString message, int id, QString out);
    void checkActiveDialog(QString login,QString message, QString out,QString time);

    void changeReceiverUserSignal(QString userlogin,int id);

    void newSearchUser(QString userlogin,int id);

    void addAccount();

    void sendPersonalMessage(const QString &message, const QString &receiver_login, const int &receiver_id);
    void sendSearchToServer(const QString &searchable);
    void sendLoginRequest(QString userlogin,QString password);
    void sendRegisterRequest(const QString login, const QString password);
    void sendEditProfileRequest(const QString editable,const QString editInformation);

    void editUserlogin(QString editInformation);
    void editPhoneNumber(QString editInformation);
    void editName(QString editInformation);
    void editUniqueError();
    void unknownError();

    void clearMainListView();
    void newUser(QString username);
    void configCheck(const QSettings& settings);
    void clearUserListModel();

    void changeActiveAccount(QString username);

    void setLoggers(Logger* logger);
private:
    Logger* logger;
    NetworkManager *networkManager;
    MessageManager *messageManager;
    AccountManager *accountManager;

    void setLogger(Logger* logger);
};

#endif // CLIENT_H
