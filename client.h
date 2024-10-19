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

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString messageFrom READ messageFrom WRITE setMessageFrom)

public:
    explicit Client(QObject *parent = nullptr);
    QString messageFrom();
    void setMessageFrom(QString value);

private:
    void showMessageList(QString login);
    void saveMessageFromDatabase(QJsonObject json);
    void updatingChats();
    void loadMessageToQml(const QString username,const QString message,const QString out,const QString date);
    void loadMessageFromJson(const QString &filepath);
    void saveMessageToJson(const QString userlogin, const QString message, const QString out, const QString time,const QString FullDate, const int message_id,const int dialog_id,const int id);
    void connectToServer();
    void createConfigFile(QString userLogin,QString userPassword);

signals:

    void showPersonalChat(QString login,QString message,int id,QString out);

    void clearMainListView();
    void newInMessage(QString name,QString message,QString time);
    void newOutMessage(QString name,QString message,QString time);

    void errorWithConnect();
    void connectionSuccess();

    void loginSuccess(QString name);
    void loginFail();

    void regSuccess();
    void regFail(QString error);

    void clientLogout();

    void checkActiveDialog(QString userlogin);
    void changeReceiverUserSignal(QString userlogin,int id);

    void newSearchUser(QString userlogin,int id);

    void newUser(QString username);
    void changeAccount(QString username,QString password);
    void addAccount();

private:
    int user_id;
    QString activeUserName;
    QTimer reconnectTimer;
    QTcpSocket* socket;
    QByteArray data;

protected:
    QString mesFrom;

private slots:
    void onDisconnected();
    void attemptReconnect();

public slots:
    void readPersonalJson(const QString userlogin);
    void sendPersonalMessage(const QString &message,const QString &receiver_login,const int &receiver_id);
    void checkMessage(const QString &userlogin);

    void changeActiveAccount(const QString username);
    void reg(QString login,QString password);
    void login(QString login,QString password);
    void slotReadyRead();
    void sendSearchToServer(const QString &searchable);
    void sendToServer(QString str,QString name);
    void logout();
    void clientChangeAccount();
};

#endif // CLIENT_H
