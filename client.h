#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>

#include <QFile>
#include <QStandardPaths>
#include <QSettings>

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString messageFrom READ messageFrom WRITE setMessageFrom)

public:
    explicit Client(QObject *parent = nullptr);
    QString messageFrom();
    void setMessageFrom(QString value);

private:
    void connectToServer();
    void createConfigFile(QString userLogin,QString userPassword);

signals:
    void newInMessage(QString name);
    void newOutMessage(QString name);

    void errorWithConnect();
    void connectionSuccess();

    void loginSuccess(QString name);
    void loginFail();

    void regSuccess();
    void regFail(QString error);

private:
    QTimer reconnectTimer;
    QTcpSocket* socket;
    QByteArray data;

protected:
    QString mesFrom;

private slots:
    void onDisconnected();
    void attemptReconnect();

public slots:
    void reg(QString login,QString password);
    void login(QString login,QString password);
    void slotReadyRead();
    void sendToServer(QString str,QString name);
};

#endif // CLIENT_H
