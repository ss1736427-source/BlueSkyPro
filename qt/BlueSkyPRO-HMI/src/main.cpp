#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlContext>

#include "PlanningBridge.h"
#include "TileCacheManager.h"
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
    TileCacheManager tileCacheManager;
    engine.rootContext()->setContextProperty(QStringLiteral("planningBridge"), &planningBridge);
    engine.rootContext()->setContextProperty(QStringLiteral("tileCacheManager"), &tileCacheManager);
    const QString yandexMapsApiKey = qEnvironmentVariable("BLUESKY_YANDEX_MAPS_API_KEY");
    const QString mapTilerApiKey = qEnvironmentVariable("BLUESKY_MAPTILER_API_KEY");
    const QString cartoApiKey = qEnvironmentVariable("BLUESKY_CARTO_API_KEY");
    engine.rootContext()->setContextProperty(QStringLiteral("yandexMapsApiKey"), yandexMapsApiKey);
    engine.rootContext()->setContextProperty(QStringLiteral("mapTilerApiKey"), mapTilerApiKey);
    engine.rootContext()->setContextProperty(QStringLiteral("cartoApiKey"), cartoApiKey);
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
