#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include "client.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    const QUrl mainUrl(QStringLiteral("qrc:/qmlqtdesign/Main.qml"));
    const QUrl loginUrl(QStringLiteral("qrc:/qmlqtdesign/LoginPage.qml"));
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

        QObject::connect(&client, &Client::loginSuccess, [&engine, mainUrl]() {
            engine.rootObjects().first()->deleteLater();
            engine.load(mainUrl);
        });

    return app.exec();
}
