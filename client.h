#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

class Client : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString messageFrom READ messageFrom WRITE setMessageFrom)
public:
    explicit Client(QObject *parent = nullptr);
    QString messageFrom();
    void setMessageFrom(QString value);
    void connectToServer(QString host,quint16 port);

signals:
    void newInMessage();
    void errorWithConnect();
    void loginSuccess();
    void loginFail();
private:
    QTcpSocket* socket;
    QByteArray data;
protected:
    QString mesFrom;
public slots:
    void login(QString login,QString password);
    void slotReadyRead();
    void sendToServer(QString str);
};

#endif // CLIENT_H
