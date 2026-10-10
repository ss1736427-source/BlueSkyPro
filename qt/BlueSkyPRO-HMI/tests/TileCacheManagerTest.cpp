#include "../src/TileCacheManager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QEventLoop>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTimer>
#include <QUrl>
#include <QTcpServer>
#include <QTcpSocket>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

static QString writeCachedTile(const QString &cacheRoot, const QString &key)
{
    QString relative = key;
    relative.replace(QChar(92), QLatin1Char('/'));
    while (relative.startsWith('/'))
        relative.remove(0, 1);

    const QString path = QDir(cacheRoot).filePath(
        QStringLiteral("tiles/") + relative + QStringLiteral(".png"));
    assert(QDir().mkpath(QFileInfo(path).absolutePath()));

    QFile file(path);
    assert(file.open(QIODevice::WriteOnly));
    assert(file.write("cached-tile-fixture") > 0);
    file.close();
    return QFileInfo(path).absoluteFilePath();
}

static bool awaitTile(TileCacheManager &manager, const QString &key, const QString &url,
                      QString *source)
{
    QEventLoop loop;
    bool received = false;
    const auto readyConnection = QObject::connect(
        &manager, &TileCacheManager::tileReady, &loop,
        [&](const QString &readyKey, const QString &fileUrl) {
            if (readyKey == key) {
                *source = fileUrl;
                received = true;
                loop.quit();
            }
        });
    const auto failedConnection = QObject::connect(
        &manager, &TileCacheManager::tileFailed, &loop,
        [&](const QString &failedKey) {
            if (failedKey == key)
                loop.quit();
        });

    *source = manager.requestTile(key, url);
    if (source->isEmpty()) {
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();
    }
    QObject::disconnect(readyConnection);
    QObject::disconnect(failedConnection);
    return received;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTemporaryDir cache;
    assert(cache.isValid());
    qputenv("BLUESKY_TILE_CACHE_DIR", QFile::encodeName(cache.path()));

    const QString lightKey = QStringLiteral("maptiler/hybrid-v4/10/3/4");
    const QString darkKey = QStringLiteral("maptiler/hybrid-v4-dark/10/3/4");
    const QString lightPath = writeCachedTile(cache.path(), lightKey);
    const QString darkPath = writeCachedTile(cache.path(), darkKey);
    assert(lightPath != darkPath);

    const QString expiredKey = QStringLiteral("yandex/future_map/web_mercator/9/10/11");
    const QString orphanKey = QStringLiteral("yandex/future_map/web_mercator/9/10/12");
    const QString expiredPath = writeCachedTile(cache.path(), expiredKey);
    const QString orphanPath = writeCachedTile(cache.path(), orphanKey);
    const QDateTime expiredTime = QDateTime::currentDateTimeUtc().addDays(-31);
    for (const QString &path : {expiredPath, orphanPath}) {
        QFile oldTile(path);
        assert(oldTile.open(QIODevice::ReadWrite));
        assert(oldTile.setFileTime(expiredTime, QFileDevice::FileModificationTime));
        oldTile.close();
    }

    QTcpServer server;
    assert(server.listen(QHostAddress::LocalHost, 0));
    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server]() {
        while (server.hasPendingConnections()) {
            QTcpSocket *socket = server.nextPendingConnection();
            QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket]() {
                const QByteArray request = socket->readAll();
                if (socket->property("responseSent").toBool())
                    return;
                socket->setProperty("responseSent", true);

                const bool noStore = request.contains("/no-store");
                const QByteArray cacheControl = noStore
                    ? QByteArray("no-store") : QByteArray("max-age=1");
                const QByteArray body("tiledata");
                const QByteArray response =
                    "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nCache-Control: "
                    + cacheControl
                    + "\r\nContent-Length: " + QByteArray::number(body.size())
                    + "\r\nConnection: close\r\n\r\n" + body;
                socket->write(response);
                socket->disconnectFromHost();
            });
        }
    });

    TileCacheManager manager;
    const QString lightSource = manager.requestTile(
        lightKey, QStringLiteral("http://127.0.0.1:1/should-not-be-requested"));
    const QString darkSource = manager.requestTile(
        darkKey, QStringLiteral("http://127.0.0.1:1/should-not-be-requested"));

    assert(!lightSource.isEmpty());
    assert(!darkSource.isEmpty());
    assert(QUrl(lightSource).isLocalFile());
    assert(QUrl(darkSource).isLocalFile());
    assert(QFileInfo(QUrl(lightSource).toLocalFile()).canonicalFilePath()
           == QFileInfo(lightPath).canonicalFilePath());
    assert(QFileInfo(QUrl(darkSource).toLocalFile()).canonicalFilePath()
           == QFileInfo(darkPath).canonicalFilePath());
    assert(QUrl(lightSource).toLocalFile() != QUrl(darkSource).toLocalFile());

    // A cache hit must win even if the caller supplies a different URL.
    const QString repeatedSource = manager.requestTile(
        lightKey, QStringLiteral("not-a-network-url"));
    assert(repeatedSource == lightSource);

    // Expired tiles must not be served, and are removed on access.
    const QString expiredSource = manager.requestTile(
        expiredKey, QStringLiteral("invalid://expired-cache-miss"));
    assert(expiredSource.isEmpty());
    assert(!QFileInfo::exists(expiredPath));

    // Expired entries that are not requested are also removed in bounded batches.
    QEventLoop cleanupLoop;
    QTimer::singleShot(50, &cleanupLoop, &QEventLoop::quit);
    cleanupLoop.exec();
    assert(!QFileInfo::exists(orphanPath));

    // max-age from the provider is honored rather than replaced with the 29-day cap.
    const QString shortLivedKey = QStringLiteral("test/short-lived");
    const QString url = QStringLiteral("http://127.0.0.1:%1/max-age")
                            .arg(server.serverPort());
    QString downloadedSource;
    assert(awaitTile(manager, shortLivedKey, url, &downloadedSource));
    assert(QUrl(downloadedSource).isLocalFile());
    assert(!manager.requestTile(shortLivedKey, url).isEmpty());

    QEventLoop expiryLoop;
    QTimer::singleShot(1250, &expiryLoop, &QEventLoop::quit);
    expiryLoop.exec();
    assert(manager.requestTile(shortLivedKey, url).isEmpty());

    // no-store responses are delivered for this request without a disk cache entry.
    const QString noStoreKey = QStringLiteral("test/no-store");
    const QString noStoreUrl = QStringLiteral("http://127.0.0.1:%1/no-store")
                                   .arg(server.serverPort());
    QString noStoreSource;
    assert(awaitTile(manager, noStoreKey, noStoreUrl, &noStoreSource));
    assert(noStoreSource.startsWith(QStringLiteral("data:image/png;base64,")));
    const QString noStorePath = QDir(cache.path()).filePath(
        QStringLiteral("tiles/test/no-store.png"));
    assert(!QFileInfo::exists(noStorePath));
    assert(manager.requestTile(noStoreKey, QStringLiteral("invalid://must-fetch-again"))
           .isEmpty());
    return 0;
}
