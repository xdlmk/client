#include <QApplication>
#include <QQmlApplicationEngine>

#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include <QSettings>
#include <QEventLoop>
#include <QDir>

#include "Core/client.h"
#include "Managers/accountmanager.h"
#include "Managers/filemanager.h"
#include "Utils/logger.h"
#include "Utils/apptheme.h"

int main(int argc, char *argv[])
{
    qputenv("QT_MEDIA_BACKEND", "windows");

    QApplication app(argc, argv);
    QQuickStyle::setStyle("Basic");

    QQmlApplicationEngine engine;

    QEventLoop loop;

    const QUrl mainUrl(QStringLiteral("resources/qmlFiles/Main.qml"));
    const QUrl switchUrl(QStringLiteral("resources/qmlFiles/pageSwitch.qml"));
    const QUrl loginUrl(QStringLiteral("resources/qmlFiles/LoginPage.qml"));

    Client client;
    Logger logger;
    FileManager* fileManager = client.getFileManager();
    AppTheme themeManager;

    client.setLoggers(&logger);

    AccountManager* accountManager = client.getAccountManager();

    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";
    QSettings settings(configFilePath,QSettings::IniFormat);

    engine.rootContext()->setContextObject(&client);
    engine.rootContext()->setContextProperty("client",&client);
    engine.rootContext()->setContextProperty("fileManager",fileManager);
    engine.rootContext()->setContextProperty("audioManager", client.getAudioManager());
    engine.rootContext()->setContextProperty("themeManager", &themeManager);
    engine.rootContext()->setContextProperty("logger",&logger);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    QObject::connect(&client, &Client::connectionSuccess, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (rootObjects.isEmpty()) {
            engine.load(switchUrl);
        }
    });
    QObject::connect(&client, &Client::connectionError, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (rootObjects.isEmpty()) {
            engine.load(switchUrl);
        }
    });

    QObject::connect(&client, &Client::loginSuccess, [&engine, mainUrl, &themeManager](QString userLogin, int user_id) {
        QList<QObject*> rootObjects = engine.rootObjects();
        for (QObject *rootObject : rootObjects) {
            if (rootObject) {
                rootObject->deleteLater();
            }
        }
        engine.clearComponentCache();
        themeManager.loadForUser(QString::number(user_id));
        engine.rootContext()->setContextProperty("userlogin", userLogin);
        engine.rootContext()->setContextProperty("activeUserId", user_id);
        engine.rootContext()->setContextProperty("appPath", QCoreApplication::applicationDirPath());
        engine.load(mainUrl);
    });

    QObject::connect(&client, &Client::editUserlogin, [&engine,&settings,&client](QString newUserLogin) {
        engine.rootContext()->setContextProperty("userlogin", newUserLogin);

        int active_account = settings.value("active_account",0).toInt();
        settings.setValue("login"+QString::number(active_account), newUserLogin);

        client.clearUserListModel();
        client.configCheck();

    });

    QObject::connect(&client, &Client::registrationSuccess, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.load(switchUrl);
    });

    QObject::connect(&client, &Client::addAccount, [&engine, &client, switchUrl, accountManager,&logger]() {
        logger.log(Logger::INFO,"main.cpp","addAccount received, deleting root object");
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        accountManager->clientChangeAccount();
        engine.load(switchUrl);
    });

    QObject::connect(&client, &Client::loginFail, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.load(switchUrl);

    });

    QObject::connect(&client, &Client::clientLogout, [&engine, switchUrl,&logger]() {
        logger.log(Logger::INFO,"main.cpp","clientLogout received, deleting root object");
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.load(switchUrl);
    });

    QObject::connect(accountManager, &AccountManager::changeAccount, [&engine, &client,accountManager,&logger](QString username,QString password) {
        logger.log(Logger::INFO,"main.cpp","changeAccount received, deleting root object");
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }

        accountManager->clientChangeAccount();
        accountManager->login(username,password);

    });

    QObject::connect(&client, &Client::connectionSuccess,&loop,&QEventLoop::quit);
    loop.exec();

    int total = settings.value("total",0).toInt();
    quint64 active_account = settings.value("active_account",0).toInt();
    if(total >= 1)
    {
        themeManager.loadForUser(QString::number(active_account));
        QString success = settings.value("success"+QString::number(active_account),"").toString();
        QString login = settings.value("login"+QString::number(active_account), "").toString();
        QString password = settings.value("password"+QString::number(active_account), "").toString();
        accountManager->login(login,password);
    }

    return app.exec();
}
