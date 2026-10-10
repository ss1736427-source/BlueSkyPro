#include "../src/TileCacheManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QUrl>

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
    return 0;
}
