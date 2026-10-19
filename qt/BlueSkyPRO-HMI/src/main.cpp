#include <QtWebEngineQuick/QtWebEngineQuick>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlContext>

#include "PlanningBridge.h"
#include "VirtualFlightAssessmentBridge.h"
#include "ManualGeometryBridge.h"
#include "ManualMissionModelAdapter.h"
#include "RouteController.h"
#include "TileCacheManager.h"
#include "CredentialVault.h"
#include <QQmlApplicationEngine>
#include <QUrl>
#include <QString>

int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();
    QGuiApplication app(argc, argv);

    // Define stable identity before QML components create QSettings instances.
    QCoreApplication::setOrganizationName(QStringLiteral("BlueSky PRO"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("blueskypro.local"));
    QCoreApplication::setApplicationName(QStringLiteral("BlueSky PRO"));

    QQmlApplicationEngine engine;
    PlanningBridge planningBridge;
    VirtualFlightAssessmentBridge virtualFlightAssessmentBridge;
    ManualGeometryBridge manualGeometryBridge;
    ManualMissionModelAdapter manualMissionModelAdapter;
    RouteController routeController;
    TileCacheManager tileCacheManager;
    CredentialVault credentialVault;
    engine.rootContext()->setContextProperty(QStringLiteral("planningBridge"), &planningBridge);
    engine.rootContext()->setContextProperty(QStringLiteral("virtualFlightAssessmentBridge"), &virtualFlightAssessmentBridge);
    engine.rootContext()->setContextProperty(QStringLiteral("manualGeometryBridge"), &manualGeometryBridge);
    engine.rootContext()->setContextProperty(QStringLiteral("manualMissionModelAdapter"), &manualMissionModelAdapter);
    engine.rootContext()->setContextProperty(QStringLiteral("routeController"), &routeController);
    engine.rootContext()->setContextProperty(QStringLiteral("tileCacheManager"), &tileCacheManager);
    engine.rootContext()->setContextProperty(QStringLiteral("credentialVault"), &credentialVault);
    const QString yandexMapsApiKey = qEnvironmentVariable("BLUESKY_YANDEX_MAPS_API_KEY");
    engine.rootContext()->setContextProperty(QStringLiteral("yandexMapsApiKey"), yandexMapsApiKey);
    QString mapTilerApiKey = qEnvironmentVariable("BLUESKY_MAPTILER_API_KEY");
    if (mapTilerApiKey.isEmpty()) mapTilerApiKey = credentialVault.readSecret(QStringLiteral("maptiler"));
    engine.rootContext()->setContextProperty(QStringLiteral("mapTilerApiKey"), mapTilerApiKey);
    const QString cartoApiKey = qEnvironmentVariable("BLUESKY_CARTO_API_KEY");
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
