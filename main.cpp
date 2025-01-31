#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QQmlEngine>
#include <QQmlContext>

#include <QSettings>
#include <QEventLoop>
#include <QDir>

#include "client.h"
#include "accountmanager.h"
#include "messagemanager.h"
#include "networkmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    QEventLoop loop;

    const QUrl mainUrl(QStringLiteral("resources/qmlFiles/Main.qml"));
    const QUrl switchUrl(QStringLiteral("resources/qmlFiles/pageSwitch.qml"));
    const QUrl loginUrl(QStringLiteral("resources/qmlFiles/LoginPage.qml"));

    Client client;
    AccountManager* accountManager = client.getAccountManager();

    engine.rootContext()->setContextObject(&client);
    engine.rootContext()->setContextProperty("client",&client);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    QObject::connect(&client, &Client::connectionSuccess, [&engine, switchUrl]() {
        qDebug() << "clientLogout received, deleting root object";
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
            qDebug() << "Root object marked for deletion";
        }
        else
        {
            qDebug() << "No root objects found!";
        }
        engine.load(switchUrl);
        qDebug() << "Loaded new URL";
    });

    QObject::connect(&client, &Client::connectionError, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (rootObjects.isEmpty()) {
            engine.load(switchUrl);
        }
    });

    QObject::connect(&client, &Client::loginSuccess, [&engine, mainUrl](QString userLogin) {
        QList<QObject*> rootObjects = engine.rootObjects();
        for (QObject *rootObject : rootObjects) {
            if (rootObject) {
                rootObject->deleteLater();
            }
        }
        engine.clearComponentCache();
        engine.rootContext()->setContextProperty("userlogin", userLogin);
        engine.load(mainUrl);
    });

    QObject::connect(&client, &Client::registrationSuccess, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.load(switchUrl);
    });

    QObject::connect(&client, &Client::addAccount, [&engine, &client, switchUrl, accountManager]() {
        qDebug() << "addAccount received, deleting root object";
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
            qDebug() << "Root object marked for deletion";
        }
        else
        {
            qDebug() << "No root objects found!";
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

    QObject::connect(&client, &Client::clientLogout, [&engine, switchUrl]() {
        qDebug() << "clientLogout received, deleting root object";
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
            qDebug() << "Root object marked for deletion";
        }
        else
            {
            qDebug() << "No root objects found!";
        }
        engine.load(switchUrl);
        qDebug() << "Loaded new URL";

    });

    QObject::connect(accountManager, &AccountManager::changeAccount, [&engine, &client,accountManager](QString username,QString password) {
        qDebug() << "clientLogout received, deleting root object";
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
            qDebug() << "Root object marked for deletion";
        }
        else
        {
            qDebug() << "No root objects found!";
        }
        accountManager->clientChangeAccount();
        accountManager->login(username,password);

    });

    QObject::connect(&client, &Client::changeReceiverUserSignal, [&client](QString userlogin, int id) {
        qDebug() << "changeReceiverUserSignal";
        client.loadingPersonalChat(userlogin);
    });
    // loop for fix crash if server dont be started
    QObject::connect(&client, &Client::connectionSuccess,&loop,&QEventLoop::quit);
    loop.exec();

    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";

    QSettings settings(configFilePath,QSettings::IniFormat);

    int total = settings.value("total",0).toInt();
    int active_account = settings.value("active_account",0).toInt();
    if(total >= 1)
    {
        QString success = settings.value("success"+QString::number(active_account),"").toString();
        qDebug() << success;
        if(success == "ok")
        {
            QString login = settings.value("login"+QString::number(active_account), "").toString();
            QString password = settings.value("password"+QString::number(active_account), "").toString();

            accountManager->login(login,password);
        }
    }
    else
    {
        qDebug() << "Total !=1,2,3";
    }

    return app.exec();
}
