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

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString messageFrom READ messageFrom WRITE setMessageFrom)

public:
    explicit Client(QObject *parent = nullptr);
    QString messageFrom();
    void setMessageFrom(QString value);

    AccountManager* getAccountManager();
    NetworkManager* getNetworkManager();
    MessageManager* getMessageManager();

signals:
    void newOutMessage(QString username,QString message,QString time);
    void newInMessage(QString username,QString message,QString time);

    void connectionSuccess();
    void connectionError();

    void loginSuccess(QString &name);
    void loginFail();

    void registrationSuccess();
    void registrationFail(QString &error);

    void clientLogout();

    void loadingPersonalChat(const QString userlogin);

    void showPersonalChat(QString login,QString message, int id, QString out);
    void checkActiveDialog(QString userlogin);

    void changeReceiverUserSignal(QString userlogin,int id);

    void newSearchUser(QString userlogin,int id);

    void addAccount();

    void sendPersonalMessage(const QString &message, const QString &receiver_login, const int &receiver_id);

    void clearMainListView();
    void newUser(QString username);

    void changeActiveAccount(QString username);


private:
    NetworkManager *networkManager;
    MessageManager *messageManager;
    AccountManager *accountManager;

    int user_id;
    QString activeUserName;
    QTimer reconnectTimer;
    QTcpSocket* socket;
    QByteArray data;

protected:
    QString mesFrom;

public slots:
    void sendLoginRequest(QString &userlogin,QString &password);

    void sendSearchToServer(const QString &searchable);
    void sendToServer(QString str,QString name);
};

#endif // CLIENT_H
