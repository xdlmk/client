#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QList>
#include "logger.h"

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);

    void setLogger(Logger *logger);
    void addAccount(const QString &login, const QString &password, int userId);
    void removeAccount(int accountIndex);
    int getActiveAccount();
public slots:
    void changeActiveAccount(QString username);
    void checkConfigFile();
signals:
    void newUser(QString username,int user_id);
    void sendLoginAfterLogout(const QString login, const QString password);
    void changeAccount(QString username,QString password);

private:
    QSettings settings;
    Logger *logger;
};

#endif // CONFIGMANAGER_H
