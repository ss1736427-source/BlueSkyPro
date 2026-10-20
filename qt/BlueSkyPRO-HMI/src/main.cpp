#include <QtWebEngineQuick/QtWebEngineQuick>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlError>

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
    const QString diagnosticLogPath =
        QDir::temp().filePath(QStringLiteral("BlueSkyPRO-startup-diagnostic.log"));
    const auto writeStartupLog = [&diagnosticLogPath](const QString &message) {
        QFile logFile(diagnosticLogPath);
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            const QByteArray line =
                QDateTime::currentDateTime().toString(Qt::ISODateWithMs).toUtf8()
                + " " + message.toUtf8() + "\n";
            logFile.write(line);
            logFile.flush();
        }
    };

    writeStartupLog(QStringLiteral("START: main entered"));
    writeStartupLog(QStringLiteral("CHECKPOINT: before QtWebEngineQuick::initialize"));
    QtWebEngineQuick::initialize();
    writeStartupLog(QStringLiteral("CHECKPOINT: after QtWebEngineQuick::initialize"));

    writeStartupLog(QStringLiteral("CHECKPOINT: before QGuiApplication constructor"));
    QGuiApplication app(argc, argv);
    writeStartupLog(QStringLiteral("CHECKPOINT: after QGuiApplication constructor"));

    // Define stable identity before QML components create QSettings instances.
    QCoreApplication::setOrganizationName(QStringLiteral("BlueSky PRO"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("blueskypro.local"));
    QCoreApplication::setApplicationName(QStringLiteral("BlueSky PRO"));
    writeStartupLog(QStringLiteral("CHECKPOINT: application identity configured"));

    QQmlApplicationEngine engine;
    writeStartupLog(QStringLiteral("CHECKPOINT: QQmlApplicationEngine constructed"));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::warnings,
        &engine,
        [&writeStartupLog](const QList<QQmlError> &warnings) {
            for (const QQmlError &warning : warnings) {
                writeStartupLog(QStringLiteral("QML WARNING: ") + warning.toString());
            }
        });

    writeStartupLog(QStringLiteral("CHECKPOINT: before PlanningBridge constructor"));
    PlanningBridge planningBridge;
    writeStartupLog(QStringLiteral("CHECKPOINT: after PlanningBridge constructor"));

    writeStartupLog(QStringLiteral("CHECKPOINT: before VirtualFlightAssessmentBridge constructor"));
    VirtualFlightAssessmentBridge virtualFlightAssessmentBridge;
    writeStartupLog(QStringLiteral("CHECKPOINT: after VirtualFlightAssessmentBridge constructor"));

    writeStartupLog(QStringLiteral("CHECKPOINT: before ManualGeometryBridge constructor"));
    ManualGeometryBridge manualGeometryBridge;
    writeStartupLog(QStringLiteral("CHECKPOINT: after ManualGeometryBridge constructor"));

    writeStartupLog(QStringLiteral("CHECKPOINT: before ManualMissionModelAdapter constructor"));
    ManualMissionModelAdapter manualMissionModelAdapter;
    writeStartupLog(QStringLiteral("CHECKPOINT: after ManualMissionModelAdapter constructor"));

    writeStartupLog(QStringLiteral("CHECKPOINT: before RouteController constructor"));
    RouteController routeController;
    writeStartupLog(QStringLiteral("CHECKPOINT: after RouteController constructor"));

    writeStartupLog(QStringLiteral("CHECKPOINT: before TileCacheManager constructor"));
    TileCacheManager tileCacheManager;
    writeStartupLog(QStringLiteral("CHECKPOINT: after TileCacheManager constructor"));

    writeStartupLog(QStringLiteral("CHECKPOINT: before CredentialVault constructor"));
    CredentialVault credentialVault;
    writeStartupLog(QStringLiteral("CHECKPOINT: after CredentialVault constructor"));

    engine.rootContext()->setContextProperty(QStringLiteral("planningBridge"), &planningBridge);
    engine.rootContext()->setContextProperty(QStringLiteral("virtualFlightAssessmentBridge"), &virtualFlightAssessmentBridge);
    engine.rootContext()->setContextProperty(QStringLiteral("manualGeometryBridge"), &manualGeometryBridge);
    engine.rootContext()->setContextProperty(QStringLiteral("manualMissionModelAdapter"), &manualMissionModelAdapter);
    engine.rootContext()->setContextProperty(QStringLiteral("routeController"), &routeController);
    // Explicit alias avoids QML property-name shadowing in component bindings.
    engine.rootContext()->setContextProperty(QStringLiteral("canonicalRouteController"), &routeController);
    engine.rootContext()->setContextProperty(QStringLiteral("tileCacheManager"), &tileCacheManager);
    engine.rootContext()->setContextProperty(QStringLiteral("credentialVault"), &credentialVault);
    writeStartupLog(QStringLiteral("CHECKPOINT: C++ context properties configured"));

    const QString yandexMapsApiKey = qEnvironmentVariable("BLUESKY_YANDEX_MAPS_API_KEY");
    engine.rootContext()->setContextProperty(QStringLiteral("yandexMapsApiKey"), yandexMapsApiKey);
    QString mapTilerApiKey = qEnvironmentVariable("BLUESKY_MAPTILER_API_KEY");
    if (mapTilerApiKey.isEmpty()) mapTilerApiKey = credentialVault.readSecret(QStringLiteral("maptiler"));
    engine.rootContext()->setContextProperty(QStringLiteral("mapTilerApiKey"), mapTilerApiKey);
    const QString cartoApiKey = qEnvironmentVariable("BLUESKY_CARTO_API_KEY");
    engine.rootContext()->setContextProperty(QStringLiteral("cartoApiKey"), cartoApiKey);
    writeStartupLog(QStringLiteral("CHECKPOINT: map provider configuration loaded"));

    const QUrl url(QStringLiteral("qrc:/qt/qml/BlueSky/PRO/qml/App.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [&writeStartupLog]() {
            writeStartupLog(QStringLiteral("ERROR: QQmlApplicationEngine objectCreationFailed"));
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    writeStartupLog(QStringLiteral("CHECKPOINT: before engine.load(App.qml)"));
    engine.load(url);
    writeStartupLog(QStringLiteral("CHECKPOINT: after engine.load(App.qml)"));

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &app, [&writeStartupLog]() {
        writeStartupLog(QStringLiteral("CHECKPOINT: application aboutToQuit"));
    });

    writeStartupLog(QStringLiteral("CHECKPOINT: entering app.exec"));
    return app.exec();
}
