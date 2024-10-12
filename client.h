#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    void loadMessageToQml(const QString username,const QString message,const QString out,const QString date);
    void loadMessageFromJson();
    void saveMessageToJson(const QString username,const QString message,const QString out);
    void connectToServer();
    void createConfigFile(QString userLogin,QString userPassword);

signals:
    void newInMessage(QString name,QString message,QString time);
    void newOutMessage(QString name,QString message,QString time);

    void errorWithConnect();
    void connectionSuccess();

    void loginSuccess(QString name);
    void loginFail();

    void regSuccess();
    void regFail(QString error);

    void clientLogout();

    void newUser(QString username);
    void changeAccount(QString username,QString password);

private:
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
    void changeActiveAccount(const QString username);
    void reg(QString login,QString password);
    void login(QString login,QString password);
    void slotReadyRead();
    void sendToServer(QString str,QString name);
    void logout();
    void clientChangeAccount();
};

#endif // CLIENT_H
