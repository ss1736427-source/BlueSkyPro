#include "TerrainDataService.h"

#include "MapTilerTerrainRgbDecoder.h"
#include "TerrainRgbTileMatrix.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QTimer>
#include <QStandardPaths>
#include <QUrl>

#include <algorithm>

TerrainDataService::TerrainDataService(QObject *parent)
    : QObject(parent), m_network(this)
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    m_cacheDirectory = QDir(base).filePath(QStringLiteral("terrain/maptiler-terrain-rgb-v2"));
    QDir().mkpath(m_cacheDirectory);
}

QString TerrainDataService::state() const { return m_state; }
QString TerrainDataService::lastError() const { return m_lastError; }
int TerrainDataService::completedTiles() const { return m_completedCount; }
int TerrainDataService::totalTiles() const { return m_totalCount; }

void TerrainDataService::setState(const QString &value)
{
    if (m_state == value)
        return;
    m_state = value;
    emit stateChanged();
}

void TerrainDataService::setError(const QString &value)
{
    if (m_lastError == value)
        return;
    m_lastError = value;
    emit lastErrorChanged();
}

QString TerrainDataService::cachePath(int zoom, int x, int y) const
{
    return QDir(m_cacheDirectory).filePath(
        QStringLiteral("%1/%2/%3.webp").arg(zoom).arg(x).arg(y));
}

bool TerrainDataService::validCachedTile(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() <= 0
        || file.size() > kMaximumPayloadBytes)
        return false;
    QImage decoded;
    return MapTilerTerrainRgbDecoder::decodeWebpTile(file.readAll(), &decoded);
}

bool TerrainDataService::requestVisualContextTiles(
    const QString &missionId, double southDeg, double westDeg,
    double northDeg, double eastDeg, int zoom, const QString &apiKey)
{
    const QString id = missionId.trimmed();
    if (id.isEmpty() || apiKey.trimmed().isEmpty()) {
        setError(QStringLiteral("MISSION_ID_AND_MAPTILER_CREDENTIAL_REQUIRED"));
        return false;
    }
    if (!m_active.isEmpty() || !m_queue.isEmpty()) {
        setError(QStringLiteral("TERRAIN_ACQUISITION_ALREADY_ACTIVE"));
        return false;
    }

    QVector<TerrainRgbTileCoordinate> coordinates;
    if (!TerrainRgbTileMatrix::tilesForBounds(
            southDeg, westDeg, northDeg, eastDeg, zoom, &coordinates, kMaximumTiles)) {
        setError(QStringLiteral("INVALID_AOI_OR_TILE_LIMIT_EXCEEDED"));
        return false;
    }

    m_missionId = id;
    m_zoom = zoom;
    m_apiKey = apiKey;
    m_cancelled = false;
    m_completed.clear();
    m_completedCount = 0;
    m_totalCount = coordinates.size();

    for (const auto &coordinate : coordinates) {
        const QString key = QStringLiteral("%1/%2/%3")
                                .arg(zoom).arg(coordinate.x).arg(coordinate.y);
        const QString path = cachePath(zoom, coordinate.x, coordinate.y);
        if (validCachedTile(path)) {
            m_completed.append(QVariantMap{
                {QStringLiteral("zoom"), zoom},
                {QStringLiteral("x"), coordinate.x},
                {QStringLiteral("y"), coordinate.y},
                {QStringLiteral("cachePath"), path},
                {QStringLiteral("source"), QStringLiteral("MAPTILER_TERRAIN_RGB_V2")},
                {QStringLiteral("evidenceClass"), QStringLiteral("VISUAL_ONLY")},
                {QStringLiteral("verticalDatum"), QStringLiteral("UNKNOWN")},
                {QStringLiteral("accuracy"), QStringLiteral("UNKNOWN")}
            });
            ++m_completedCount;
        } else {
            m_queue.enqueue({coordinate.x, coordinate.y, 0});
        }
    }

    emit progressChanged();
    setError({});
    setState(QStringLiteral("ACQUIRING_VISUAL_CONTEXT"));
    if (m_queue.isEmpty()) {
        finishMissionIfComplete();
    } else {
        pump();
    }
    return true;
}

void TerrainDataService::pump()
{
    while (!m_cancelled && m_active.size() < kMaximumConcurrent && !m_queue.isEmpty()) {
        const TileRequest tile = m_queue.dequeue();
        startTile(tile);
    }
    finishMissionIfComplete();
}

void TerrainDataService::startTile(const TileRequest &tile)
{
    QString urlError;
    const QUrl url = MapTilerTerrainRgbDecoder::buildTileUrl(
        m_zoom, tile.x, tile.y, m_apiKey, &urlError);
    if (!url.isValid()) {
        setError(QStringLiteral("TERRAIN_TILE_URL_INVALID"));
        setState(QStringLiteral("FAILED"));
        m_cancelled = true;
        m_apiKey.clear();
        m_queue.clear();
        for (auto *pending : m_active.keys())
            pending->abort();
        emit missionTilesFailed(m_missionId, urlError);
        return;
    }

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("Accept", "image/webp");
    QNetworkReply *reply = m_network.get(request);
    m_active.insert(reply, ActiveRequest{tile, {}});
    QTimer::singleShot(20000, reply, [reply]() {
        if (!reply->isFinished())
            reply->abort();
    });
    connect(reply, &QIODevice::readyRead, this, [this, reply]() {
        auto it = m_active.find(reply);
        if (it == m_active.end())
            return;
        const QByteArray chunk = reply->readAll();
        if (it->payload.size() + chunk.size() > kMaximumPayloadBytes) {
            reply->abort();
            return;
        }
        it->payload.append(chunk);
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        finishTile(reply);
    });
}

void TerrainDataService::finishTile(QNetworkReply *reply)
{
    auto it = m_active.find(reply);
    if (it == m_active.end()) {
        reply->deleteLater();
        return;
    }
    ActiveRequest active = it.value();
    m_active.erase(it);
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray tail = reply->readAll();
    if (active.payload.size() + tail.size() <= kMaximumPayloadBytes)
        active.payload.append(tail);
    const bool networkOk = reply->error() == QNetworkReply::NoError
        && status >= 200 && status < 300
        && !active.payload.isEmpty()
        && active.payload.size() <= kMaximumPayloadBytes;
    reply->deleteLater();

    if (m_cancelled)
        return;

    QImage decoded;
    QString decodeError;
    if (networkOk && MapTilerTerrainRgbDecoder::decodeWebpTile(
            active.payload, &decoded, &decodeError)) {
        const QString path = cachePath(m_zoom, active.tile.x, active.tile.y);
        if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
            setError(QStringLiteral("TERRAIN_CACHE_DIRECTORY_UNAVAILABLE"));
            setState(QStringLiteral("FAILED"));
            m_cancelled = true;
            m_apiKey.clear();
            m_queue.clear();
            for (auto *pending : m_active.keys())
                pending->abort();
            emit missionTilesFailed(m_missionId, lastError());
            return;
        }
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly)
            || file.write(active.payload) != active.payload.size()
            || !file.commit()) {
            setError(QStringLiteral("TERRAIN_CACHE_WRITE_FAILED"));
            setState(QStringLiteral("FAILED"));
            emit missionTilesFailed(m_missionId, lastError());
            m_queue.clear();
            return;
        }
        m_completed.append(QVariantMap{
            {QStringLiteral("zoom"), m_zoom},
            {QStringLiteral("x"), active.tile.x},
            {QStringLiteral("y"), active.tile.y},
            {QStringLiteral("cachePath"), path},
            {QStringLiteral("source"), QStringLiteral("MAPTILER_TERRAIN_RGB_V2")},
            {QStringLiteral("evidenceClass"), QStringLiteral("VISUAL_ONLY")},
            {QStringLiteral("verticalDatum"), QStringLiteral("UNKNOWN")},
            {QStringLiteral("accuracy"), QStringLiteral("UNKNOWN")}
        });
        ++m_completedCount;
        emit progressChanged();
    } else if (active.tile.retry < 1) {
        ++active.tile.retry;
        m_queue.enqueue(active.tile);
    } else {
        setError(networkOk ? decodeError : QStringLiteral("TERRAIN_TILE_DOWNLOAD_FAILED"));
        setState(QStringLiteral("FAILED"));
        m_queue.clear();
        m_cancelled = true;
        m_apiKey.clear();
        for (auto *pending : m_active.keys())
            pending->abort();
        emit missionTilesFailed(m_missionId, lastError());
        return;
    }

    pump();
}

void TerrainDataService::finishMissionIfComplete()
{
    if (m_cancelled || !m_queue.isEmpty() || !m_active.isEmpty())
        return;
    if (m_totalCount <= 0 || m_completedCount != m_totalCount)
        return;
    setState(QStringLiteral("VISUAL_CONTEXT_READY"));
    emit missionTilesReady(m_missionId, m_completed);
    m_apiKey.clear();
}

void TerrainDataService::cancelMission(const QString &missionId)
{
    if (missionId.trimmed().isEmpty() || missionId.trimmed() != m_missionId)
        return;
    m_cancelled = true;
    m_queue.clear();
    for (auto *reply : m_active.keys())
        reply->abort();
    m_active.clear();
    m_apiKey.clear();
    setState(QStringLiteral("CANCELLED"));
    emit missionCancelled(m_missionId);
}
