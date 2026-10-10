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

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTemporaryDir cache;
    assert(cache.isValid());
    qputenv("BLUESKY_TILE_CACHE_DIR", cache.path().toUtf8());

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
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();
    assert(!QFileInfo::exists(orphanPath));
    return 0;
}
