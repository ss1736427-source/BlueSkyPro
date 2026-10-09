#include "TileCacheManager.h"

#include <QCoreApplication>
#include <QDir>
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
}

TileCacheManager::TileCacheManager(QObject *parent)
    : QObject(parent)
{
    m_pumpTimer.setSingleShot(true);
    connect(&m_pumpTimer, &QTimer::timeout, this, &TileCacheManager::pump);
}

QString TileCacheManager::cachePath(const QString &key) const
{
    // Keep the persistent tile cache with the project/runtime on drive E: by default.
    // An environment override supports installations with a different writable data path.
    QString cacheRoot = qEnvironmentVariable("BLUESKY_TILE_CACHE_DIR");
    if (cacheRoot.isEmpty()) {
        cacheRoot = QDir::cleanPath(
            QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../cache"));
    }
    // Provider/style identity is part of the request key so tiles from different
    // basemaps cannot collide. The QML key preserves the legacy Yandex path. 
    const QString root = QDir(cacheRoot).filePath(QStringLiteral("tiles"));

    QString relative = key;
    relative.replace(QChar(92), QLatin1Char('/'));
    while (relative.startsWith('/'))
        relative.remove(0, 1);

    const QString path = root + QLatin1Char('/') + relative + QStringLiteral(".png");
    QDir().mkpath(QFileInfo(path).absolutePath());
    return path;
}

QString TileCacheManager::requestTile(const QString &key, const QString &url)
{
    const QString path = cachePath(key);
    if (QFileInfo::exists(path) && QFileInfo(path).size() > 0)
        return QUrl::fromLocalFile(path).toString();

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
