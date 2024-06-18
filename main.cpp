#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include <QSettings>
#include <QEventLoop>
#include "client.h"

// add processing emit loginFail when using config.ini and the corresponding ones there
// add config.ini creation and update

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    QEventLoop loop;

    const QUrl mainUrl(QStringLiteral("qrc:/qmlqtdesign/qmlFiles/Main.qml"));
    const QUrl switchUrl(QStringLiteral("qrc:/qmlqtdesign/qmlFiles/pageSwitch.qml"));
    const QUrl loginUrl(QStringLiteral("qrc:/qmlqtdesign/qmlFiles/LoginPage.qml"));

    Client client;
    engine.rootContext()->setContextObject(&client);
    engine.rootContext()->setContextProperty("client",&client);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    // connection for the case with successful login and loading of the main page
    QObject::connect(&client, &Client::loginSuccess, [&engine, mainUrl](QString name) {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.rootContext()->setContextProperty("username",name);
        engine.load(mainUrl);
    });
    // connection for the case with successful registration and loading of the login page
    QObject::connect(&client, &Client::regSuccess, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.load(switchUrl);
    });
    // connection in case the configuration file is damaged
    QObject::connect(&client, &Client::loginFail, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.load(switchUrl);

    });
    // connection
    QObject::connect(&client, &Client::errorWithConnect, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (rootObjects.isEmpty()) {
            engine.load(switchUrl);
        }
    });
    // connection
    QObject::connect(&client, &Client::connectionSuccess, [&engine, switchUrl]() {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (rootObjects.isEmpty()) {
            engine.load(switchUrl);
        }
    });
    // loop for fix crash if server dont be started
    QObject::connect(&client, &Client::connectionSuccess,&loop,&QEventLoop::quit);
    loop.exec();

    //connfig file check
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    configFilePath = configFilePath + "/config.ini";

    QSettings settings(configFilePath,QSettings::IniFormat);
    QString success = settings.value("success","").toString();
    qDebug() << success;
    if(success == "ok")
    {
        QString login = settings.value("login", "").toString();
        QString password = settings.value("password", "").toString();

        client.login(login,password);
    }
    else
    {
        engine.load(switchUrl);
    }

    return app.exec();
}
