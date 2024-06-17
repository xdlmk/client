#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include <QSettings>
#include "client.h"

// add processing emit loginFail when using config.ini and the corresponding ones there
// add config.ini creation and update

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    const QUrl mainUrl(QStringLiteral("qrc:/qmlqtdesign/qmlFiles/Main.qml"));
    const QUrl loginUrl(QStringLiteral("qrc:/qmlqtdesign/qmlFiles/pageSwitch.qml"));

    Client client;
    engine.rootContext()->setContextObject(&client);
    engine.rootContext()->setContextProperty("client",&client);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    QObject::connect(&client, &Client::loginSuccess, [&engine, mainUrl](QString name) {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.rootContext()->setContextProperty("username",name);
        engine.load(mainUrl);
    });

    QObject::connect(&client, &Client::regSuccess, [&engine, mainUrl](QString name) {
        QList<QObject*> rootObjects = engine.rootObjects();
        if (!rootObjects.isEmpty()) {
            QObject *rootObject = rootObjects.first();
            rootObject->deleteLater();
        }
        engine.rootContext()->setContextProperty("username",name);
        engine.load(mainUrl);
    });

    //init file (add its creation and update)
    QString configFilePath = "config.ini";
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
        engine.load(loginUrl);    
    }

    return app.exec();
}
