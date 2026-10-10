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
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr int kMaxConcurrentRequests = 6;
constexpr int kRequestSpacingMs = 36; // ~27.8 requests/sec, below the 30 RPS limit.
constexpr int kMaxRetries = 1;
constexpr int kCacheMaxAgeDays = 29;
constexpr int kCleanupBatchSize = 64;

QString cacheMetadataPath(const QString &tilePath)
{
    return tilePath + QStringLiteral(".meta");
}

bool hasCacheDirective(const QByteArray &header, const QByteArray &directive)
{
    const QList<QByteArray> directives = header.toLower().split(',');
    for (QByteArray item : directives) {
        item = item.trimmed();
        const qsizetype equals = item.indexOf('=');
        if (equals >= 0)
            item = item.left(equals).trimmed();
        if (item == directive)
            return true;
    }
    return false;
}

bool parseMaxAge(const QByteArray &header, qint64 *seconds)
{
    const QList<QByteArray> directives = header.toLower().split(',');
    for (QByteArray item : directives) {
        item = item.trimmed();
        if (!item.startsWith("max-age="))
            continue;

        QByteArray value = item.mid(8).trimmed();
        if (value.size() >= 2 && value.startsWith('"') && value.endsWith('"'))
            value = value.mid(1, value.size() - 2);
        bool ok = false;
        const qint64 parsed = value.toLongLong(&ok);
        if (!ok || parsed < 0)
            return false;
        *seconds = parsed;
        return true;
    }
    return false;
}
}

TileCacheManager::TileCacheManager(QObject *parent)
    : QObject(parent)
{
    m_diagnosticsEnabled = qEnvironmentVariableIntValue("BLUESKY_PERF_DIAGNOSTICS") == 1;
    recordDiagnostic(QStringLiteral("diagnostics_started"),
                     {{QStringLiteral("enabled"), m_diagnosticsEnabled}});
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

void TileCacheManager::recordDiagnostic(const QString &eventName,
                                        const QVariantMap &metrics) const
{
    if (!m_diagnosticsEnabled)
        return;

    QJsonObject object;
    object.insert(QStringLiteral("event"), eventName);
    object.insert(QStringLiteral("timestamp_utc"),
                  QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    for (auto it = metrics.cbegin(); it != metrics.cend(); ++it)
        object.insert(it.key(), QJsonValue::fromVariant(it.value()));

    qInfo().noquote() << QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
    emit const_cast<TileCacheManager *>(this)->diagnosticEvent(eventName, metrics);
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

QDateTime TileCacheManager::cacheExpiry(const QString &tilePath) const
{
    const QString metadataPath = cacheMetadataPath(tilePath);
    if (QFileInfo::exists(metadataPath)) {
        QFile metadata(metadataPath);
        if (!metadata.open(QIODevice::ReadOnly))
            return {};
        const QByteArray value = metadata.readAll().trimmed();
        const QDateTime expiry = QDateTime::fromString(
            QString::fromUtf8(value), Qt::ISODateWithMs);
        return expiry.isValid() ? expiry.toUTC() : QDateTime{};
    }

    // Backward compatibility for tiles created before expiry metadata existed.
    const QFileInfo info(tilePath);
    return info.lastModified().toUTC().addDays(kCacheMaxAgeDays);
}

void TileCacheManager::removeCacheEntry(const QString &tilePath) const
{
    QFile::remove(tilePath);
    QFile::remove(cacheMetadataPath(tilePath));
}

void TileCacheManager::cleanupExpiredCacheBatch()
{
    if (!m_cleanupIterator)
        return;

    const QDateTime now = QDateTime::currentDateTimeUtc();
    int inspected = 0;
    while (m_cleanupIterator->hasNext() && inspected < kCleanupBatchSize) {
        const QString path = m_cleanupIterator->next();
        const QFileInfo info(path);
        const QDateTime expiry = cacheExpiry(path);
        if (!info.exists() || info.size() <= 0 || !expiry.isValid() || expiry <= now)
            removeCacheEntry(path);
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

void TileCacheManager::beginViewUpdate()
{
    m_currentViewKeys.clear();

    // Requests not yet started are tied to the previous viewport. Drop them
    // so rapid zooming/panning cannot leave a long FIFO backlog of obsolete tiles.
    while (!m_queue.isEmpty()) {
        const Request queued = m_queue.dequeue();
        bool isActive = false;
        for (auto it = m_activeRequests.cbegin(); it != m_activeRequests.cend(); ++it) {
            if (it.value().key == queued.key) {
                isActive = true;
                break;
            }
        }
        if (!isActive)
            m_pending.remove(queued.key);
    }

    m_pumpTimer.stop();
}

QString TileCacheManager::requestTile(const QString &key, const QString &url)
{
    m_currentViewKeys.insert(key);
    const QString path = cachePath(key);
    const QFileInfo cachedInfo(path);
    if (cachedInfo.exists()) {
        const QDateTime expiry = cacheExpiry(path);
        if (cachedInfo.size() > 0 && expiry.isValid()
                && expiry > QDateTime::currentDateTimeUtc()) {
            recordDiagnostic(QStringLiteral("cache_hit"),
                             {{QStringLiteral("file_size_bytes"), cachedInfo.size()}});
            return QUrl::fromLocalFile(path).toString();
        }

        // Expired or empty cache entries must not be served again.
        removeCacheEntry(path);
    }

    recordDiagnostic(QStringLiteral("cache_miss"), {});
    if (!m_pending.contains(key)) {
        m_pending.insert(key);
        enqueue(Request{key, url, 0, QDateTime::currentMSecsSinceEpoch(), 0});
    }

    return {};
}

void TileCacheManager::enqueue(const Request &request)
{
    Request queued = request;
    if (queued.queuedAtMs == 0)
        queued.queuedAtMs = QDateTime::currentMSecsSinceEpoch();
    m_queue.enqueue(queued);
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

    // Before spending a network slot, discard requests that no longer belong
    // to the latest viewport. beginViewUpdate() normally clears the queue, but
    // this guard also handles retries and rapid view changes safely.
    while (!m_queue.isEmpty() && !m_currentViewKeys.contains(m_queue.head().key)) {
        const Request obsolete = m_queue.dequeue();
        m_pending.remove(obsolete.key);
    }
    if (m_queue.isEmpty())
        return;

    Request request = m_queue.dequeue();
    request.startedAtMs = QDateTime::currentMSecsSinceEpoch();
    recordDiagnostic(QStringLiteral("network_started"),
                     {{QStringLiteral("queue_wait_ms"), request.startedAtMs - request.queuedAtMs},
                      {QStringLiteral("active_requests"), m_activeCount + 1},
                      {QStringLiteral("queued_requests"), m_queue.size()},
                      {QStringLiteral("retry"), request.retryCount}});
    QNetworkRequest networkRequest{QUrl(request.url)};
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
    const qint64 networkDurationMs = request.startedAtMs > 0
        ? QDateTime::currentMSecsSinceEpoch() - request.startedAtMs : -1;
    const bool ok = reply->error() == QNetworkReply::NoError
        && status >= 200 && status < 300;

    if (ok) {
        const QByteArray data = reply->readAll();
        recordDiagnostic(QStringLiteral("network_complete"),
                         {{QStringLiteral("duration_ms"), networkDurationMs},
                          {QStringLiteral("http_status"), status},
                          {QStringLiteral("bytes"), data.size()},
                          {QStringLiteral("network_error"), static_cast<int>(reply->error())}});
        const QByteArray cacheControl = reply->rawHeader("Cache-Control");
        qint64 maxAgeSeconds = 0;
        const bool hasMaxAgeDirective = hasCacheDirective(cacheControl, "max-age");
        const bool hasMaxAge = parseMaxAge(cacheControl, &maxAgeSeconds);
        const bool doNotPersist = hasCacheDirective(cacheControl, "no-store")
            || hasCacheDirective(cacheControl, "no-cache")
            || (hasMaxAgeDirective && !hasMaxAge);
        const QDateTime now = QDateTime::currentDateTimeUtc();
        QDateTime expiry;

        if (hasMaxAge) {
            expiry = now.addSecs(qMin(maxAgeSeconds,
                                      qint64(kCacheMaxAgeDays) * 24 * 60 * 60));
        } else if (!reply->rawHeader("Expires").isEmpty()) {
            expiry = QDateTime::fromString(
                QString::fromLatin1(reply->rawHeader("Expires")), Qt::RFC2822Date).toUTC();
        } else {
            expiry = now.addDays(kCacheMaxAgeDays);
        }

        // no-cache/no-store and immediately stale responses are delivered for
        // this request only. A data URL avoids writing a persistent tile file.
        if (!data.isEmpty() && (doNotPersist || !expiry.isValid() || expiry <= now)) {
            emit tileReady(request.key,
                           QStringLiteral("data:image/png;base64,")
                               + QString::fromLatin1(data.toBase64()));
        } else if (!data.isEmpty()) {
            const qint64 cacheWriteStartedMs = QDateTime::currentMSecsSinceEpoch();
            const QString path = cachePath(request.key);
            QSaveFile file(path);
            bool saved = false;
            if (file.open(QIODevice::WriteOnly)
                    && file.write(data) == data.size() && file.commit()) {
                QSaveFile metadata(cacheMetadataPath(path));
                const QByteArray timestamp = expiry.toUTC().toString(Qt::ISODateWithMs).toUtf8();
                if (metadata.open(QIODevice::WriteOnly)
                        && metadata.write(timestamp) == timestamp.size()
                        && metadata.commit()) {
                    saved = true;
                }
            }

            recordDiagnostic(QStringLiteral("cache_write"),
                             {{QStringLiteral("duration_ms"), QDateTime::currentMSecsSinceEpoch() - cacheWriteStartedMs},
                              {QStringLiteral("bytes"), data.size()},
                              {QStringLiteral("success"), saved}});
            if (saved) {
                emit tileReady(request.key, QUrl::fromLocalFile(path).toString());
            } else {
                removeCacheEntry(path);
                emit tileReady(request.key,
                               QStringLiteral("data:image/png;base64,")
                                   + QString::fromLatin1(data.toBase64()));
            }
        } else {
            emit tileFailed(request.key);
        }
        m_pending.remove(request.key);
    } else if (request.retryCount < kMaxRetries && isTransientFailure(reply)) {
        recordDiagnostic(QStringLiteral("network_failure_retry"),
                         {{QStringLiteral("duration_ms"), networkDurationMs},
                          {QStringLiteral("http_status"), status},
                          {QStringLiteral("network_error"), static_cast<int>(reply->error())},
                          {QStringLiteral("retry"), request.retryCount + 1}});
        const Request retry{request.key, request.url, request.retryCount + 1};
        QTimer::singleShot(1200, this, [this, retry]() {
            // Do not retry a tile after the user has moved to another viewport.
            if (!m_currentViewKeys.contains(retry.key)) {
                m_pending.remove(retry.key);
                pump();
                return;
            }
            m_queue.prepend(retry);
            pump();
        });
    } else {
        recordDiagnostic(QStringLiteral("tile_failure"),
                         {{QStringLiteral("duration_ms"), networkDurationMs},
                          {QStringLiteral("http_status"), status},
                          {QStringLiteral("network_error"), static_cast<int>(reply->error())},
                          {QStringLiteral("retry"), request.retryCount}});
        m_pending.remove(request.key);
        emit tileFailed(request.key);
    }

    reply->deleteLater();
    pump();
}
