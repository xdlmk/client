#include "configmanager.h"

ConfigManager::ConfigManager(QObject *parent)
    : QObject{parent}, settings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/config.ini", QSettings::IniFormat)
{}

void ConfigManager::setLogger(Logger *logger) {
    this->logger = logger;
}

void ConfigManager::addAccount(const QString &login, const QString &password, int userId) {
    int total = settings.value("total", 0).toInt();
    for (int i = 1; i <= total; ++i) {
        if (login == settings.value("login" + QString::number(i))) {
            emit checkConfigFile(settings);
            logger->log(Logger::INFO, "configmanager.cpp::addAccount", "Account already exists");
            return;
        }
    }
    total++;
    if(total > 3) {
        logger->log(Logger::INFO, "configmanager.cpp::addAccount", "Account not exists, but total is 3");
        emit checkConfigFile(settings);
        return;
    }
    settings.setValue("total", total);
    settings.setValue("active_account", total);
    settings.setValue("success" + QString::number(total), "ok");
    settings.setValue("login" + QString::number(total), login);
    settings.setValue("id" + QString::number(total), userId);
    settings.setValue("password" + QString::number(total), password);
    logger->log(Logger::INFO, "configmanager.cpp::addAccount", "Added account: " + login);

    emit checkConfigFile(settings);
}

void ConfigManager::removeAccount(int accountIndex) {
    int total = settings.value("total", 0).toInt();
    if (accountIndex > total || accountIndex < 1) return;
    for (int i = accountIndex; i < total; ++i) {
        settings.setValue("success" + QString::number(i), settings.value("success" + QString::number(i + 1)));
        settings.setValue("login" + QString::number(i), settings.value("login" + QString::number(i + 1)));
        settings.setValue("password" + QString::number(i), settings.value("password" + QString::number(i + 1)));
        settings.remove("success" + QString::number(i + 1));
        settings.remove("login" + QString::number(i + 1));
        settings.remove("password" + QString::number(i + 1));
    }
    settings.setValue("total", total - 1);
    if (total - 1 == 0) QFile::remove(settings.fileName());
    else {
        settings.setValue("active_account",1);
        if(settings.value("success1","").toString() == "ok") {
            QString login = settings.value("login1", "").toString();
            QString password = settings.value("password1", "").toString();

            emit sendLoginAfterLogout(login,password);
        }
    }
    if (logger) logger->log(Logger::INFO, "ConfigManager::removeAccount", "Removed account index: " + QString::number(accountIndex));
}

void ConfigManager::changeActiveAccount(QString username) {
    int total = settings.value("total", 0).toInt();
    for (int i = 1; i <= total; ++i) {
        if (settings.value("login" + QString::number(i)).toString() == username) {
            settings.setValue("active_account", i);
            if (logger) logger->log(Logger::INFO, "ConfigManager::changeActiveAccount", "Changed active account to: " + username);
            QString password = settings.value("password"+ QString::number(i),"").toString();
            emit changeAccount(username,password);

            return;
        }
    }
    if (logger) logger->log(Logger::WARN, "ConfigManager::changeActiveAccount", "User not found: " + username);
}

int ConfigManager::getActiveAccount() {
    return settings.value("active_account", 1).toInt();
}
