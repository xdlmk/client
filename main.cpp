#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include "client.h"


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

        engine.load(loginUrl);

        QObject *rootObject = engine.rootObjects().first();
        QObject::connect(&client, &Client::loginSuccess, [&engine, mainUrl](QString name) {
            engine.rootObjects().first()->deleteLater();
            engine.rootContext()->setContextProperty("username",name);
            engine.load(mainUrl);
        });
        QObject::connect(&client, &Client::regSuccess, [&engine, mainUrl](QString name) {
            engine.rootObjects().first()->deleteLater();
            engine.rootContext()->setContextProperty("username",name);
            engine.load(mainUrl);
        });

    return app.exec();
}
