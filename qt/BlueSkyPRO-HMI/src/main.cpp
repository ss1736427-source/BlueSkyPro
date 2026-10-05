#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlContext>

#include "PlanningBridge.h"
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QString>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Define stable identity before QML components create QSettings instances.
    QCoreApplication::setOrganizationName(QStringLiteral("BlueSky PRO"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("blueskypro.local"));
    QCoreApplication::setApplicationName(QStringLiteral("BlueSky PRO"));

    QQmlApplicationEngine engine;
    PlanningBridge planningBridge;
    engine.rootContext()->setContextProperty(QStringLiteral("planningBridge"), &planningBridge);
    const QString googleMapsApiKey = qEnvironmentVariable("BLUESKY_GOOGLE_MAPS_API_KEY");
    engine.rootContext()->setContextProperty(QStringLiteral("googleMapsApiKey"), googleMapsApiKey);
    const QUrl url(QStringLiteral("qrc:/qt/qml/BlueSky/PRO/qml/App.qml"));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
