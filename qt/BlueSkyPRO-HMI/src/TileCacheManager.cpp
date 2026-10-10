#include "TileCacheManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUrl>
#include <QDateTime>

namespace {
constexpr int kMaxConcurrentRequests = 6;
constexpr int kRequestSpacingMs = 36; // ~27.8 requests/sec, below the 30 RPS limit.
constexpr int kMaxRetries = 1;
constexpr int kCacheMaxAgeDays = 29;
constexpr int kCleanupBatchSize = 64;
}

TileCacheManager::TileCacheManager(QObject *parent)
    : QObject(parent)
{
    m_pumpTimer.setSingleShot(true);
    connect(&m_pumpTimer, &QTimer::timeout, this, &TileCacheManager::pump);
    m_cleanupTimer.setSingleShot(true);
    connect(&m_cleanupTimer, &QTimer::timeout,
            this, &TileCacheManager::cleanupExpiredCacheBatch);

    const QString root = cacheRootPath();
    if (QDir(root).exists()) {
        m_cleanupIterator = std::make_unique<QDirIterator>(
            root, QStringList{QStringLiteral("*.png")}, QDir::Files,
            QDirIterator::Subdirectories);
        m_cleanupTimer.start(0);
    }
}

QString TileCacheManager::cacheRootPath() const
{
    QString cacheRoot = qEnvironmentVariable("BLUESKY_TILE_CACHE_DIR");
    if (cacheRoot.isEmpty()) {
        cacheRoot = QDir::cleanPath(
            QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../cache"));
    }
    return QDir(cacheRoot).filePath(QStringLiteral("tiles"));
}

QString TileCacheManager::cachePath(const QString &key) const
{
    // Provider/style identity is part of the key so tiles from different basemaps
    // cannot collide. The QML key preserves the legacy Yandex path.
    const QString root = cacheRootPath();

    QString relative = key;
    relative.replace(QChar(92), QLatin1Char('/'));
    while (relative.startsWith('/'))
        relative.remove(0, 1);

    const QString path = root + QLatin1Char('/') + relative + QStringLiteral(".png");
    QDir().mkpath(QFileInfo(path).absolutePath());
    return path;
}

void TileCacheManager::cleanupExpiredCacheBatch()
{
    if (!m_cleanupIterator)
        return;

    const QDateTime cutoff = QDateTime::currentDateTimeUtc().addDays(-kCacheMaxAgeDays);
    int inspected = 0;
    while (m_cleanupIterator->hasNext() && inspected < kCleanupBatchSize) {
        const QString path = m_cleanupIterator->next();
        const QFileInfo info(path);
        if (!info.exists() || info.size() <= 0
                || info.lastModified().toUTC() < cutoff) {
            QFile::remove(path);
        }
        ++inspected;
    }

    if (!m_cleanupIterator->hasNext()) {
        m_cleanupIterator.reset();
        return;
    }

    // Yield to the GUI event loop between bounded batches; never scan the full
    // persistent cache synchronously during application startup.
    m_cleanupTimer.start(1);
}

QString TileCacheManager::requestTile(const QString &key, const QString &url)
{
    const QString path = cachePath(key);
    const QFileInfo cachedInfo(path);
    if (cachedInfo.exists()) {
        const QDateTime cutoff = QDateTime::currentDateTimeUtc().addDays(-kCacheMaxAgeDays);
        if (cachedInfo.size() > 0 && cachedInfo.lastModified().toUTC() >= cutoff)
            return QUrl::fromLocalFile(path).toString();

        // Expired or empty cache entries must not be served again.
        QFile::remove(path);
    }

    if (!m_pending.contains(key)) {
        m_pending.insert(key);
        enqueue(Request{key, url, 0});
    }

    return {};
}

void TileCacheManager::enqueue(const Request &request)
{
    m_queue.enqueue(request);
    pump();
}

void TileCacheManager::pump()
{
    if (m_queue.isEmpty() || m_activeCount >= kMaxConcurrentRequests)
        return;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 elapsed = now - m_lastStartMs;
    if (m_lastStartMs != 0 && elapsed < kRequestSpacingMs) {
        m_pumpTimer.start(static_cast<int>(kRequestSpacingMs - elapsed));
        return;
    }

    const Request request = m_queue.dequeue();
    QNetworkRequest networkRequest(QUrl(request.url));
    networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                                QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_network.get(networkRequest);
    m_activeRequests.insert(reply, request);
    ++m_activeCount;
    m_lastStartMs = QDateTime::currentMSecsSinceEpoch();

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleFinished(reply);
    });

    if (!m_queue.isEmpty())
        m_pumpTimer.start(kRequestSpacingMs);
}

bool TileCacheManager::isTransientFailure(QNetworkReply *reply)
{
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return reply->error() == QNetworkReply::TimeoutError
        || reply->error() == QNetworkReply::TemporaryNetworkFailureError
        || status == 429
        || status >= 500;
}

void TileCacheManager::handleFinished(QNetworkReply *reply)
{
    const Request request = m_activeRequests.take(reply);
    --m_activeCount;

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool ok = reply->error() == QNetworkReply::NoError
        && status >= 200 && status < 300;

    if (ok) {
        const QString path = cachePath(request.key);
        QSaveFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            const QByteArray data = reply->readAll();
            if (file.write(data) == data.size() && file.commit()) {
                emit tileReady(request.key, QUrl::fromLocalFile(path).toString());
            } else {
                emit tileFailed(request.key);
            }
        } else {
            emit tileFailed(request.key);
        }
        m_pending.remove(request.key);
    } else if (request.retryCount < kMaxRetries && isTransientFailure(reply)) {
        const Request retry{request.key, request.url, request.retryCount + 1};
        QTimer::singleShot(1200, this, [this, retry]() {
            m_queue.prepend(retry);
            pump();
        });
    } else {
        m_pending.remove(request.key);
        emit tileFailed(request.key);
    }

    reply->deleteLater();
    pump();
}
