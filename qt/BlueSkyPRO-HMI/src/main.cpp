#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QtWebEngineQuick>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    const bool yandexMapsConfigured =
        !qEnvironmentVariable("BLUESKY_YANDEX_MAPS_API_KEY").isEmpty();
    if (yandexMapsConfigured)
        QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);

    // Define stable identity before QML components create QSettings instances.
    QCoreApplication::setOrganizationName(QStringLiteral("BlueSky PRO"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("blueskypro.local"));
    QCoreApplication::setApplicationName(QStringLiteral("BlueSky PRO"));

    QQmlApplicationEngine engine;

    // Keep the Yandex Maps key outside the repository.
    // Windows CMD: set BLUESKY_YANDEX_MAPS_API_KEY=...
    const QString yandexMapsApiKey =
        qEnvironmentVariable("BLUESKY_YANDEX_MAPS_API_KEY");
    engine.rootContext()->setContextProperty(
        QStringLiteral("yandexMapsApiKey"),
        yandexMapsApiKey);

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
