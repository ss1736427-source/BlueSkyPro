#pragma once

#include <QByteArray>
#include <QHash>
#include <QImage>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>
#include <QVariantList>
#include <QVariantMap>

class QNetworkReply;

class TerrainDataService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int completedTiles READ completedTiles NOTIFY progressChanged)
    Q_PROPERTY(int totalTiles READ totalTiles NOTIFY progressChanged)

public:
    explicit TerrainDataService(QObject *parent = nullptr);

    QString state() const;
    QString lastError() const;
    int completedTiles() const;
    int totalTiles() const;

    // Downloads mission-AOI MapTiler Terrain RGB tiles into an independent,
    // immutable disk cache. This product is visual context only: it is not
    // authoritative DEM evidence and cannot satisfy terrain/READY validation.
    Q_INVOKABLE bool requestVisualProfileForRoute(const QVariantMap &routeSnapshot,
                                                  int zoom, const QString &apiKey);
    Q_INVOKABLE bool requestVisualContextTiles(const QString &missionId,
                                               double southDeg, double westDeg,
                                               double northDeg, double eastDeg,
                                               int zoom, const QString &apiKey);
    Q_INVOKABLE void cancelMission(const QString &missionId);

signals:
    void stateChanged();
    void lastErrorChanged();
    void progressChanged();
    void missionTilesReady(const QString &missionId, const QVariantList &tiles);
    void missionTilesFailed(const QString &missionId, const QString &reason);
    void missionVisualProfileReady(const QString &missionId, const QString &routeId,
                                   const QString &routeVersion, const QString &calculationInputVersion,
                                   const QVariantList &samples);
    void missionVisualProfileFailed(const QString &missionId, const QString &routeId,
                                    const QString &routeVersion, const QString &reason);
    void missionCancelled(const QString &missionId);

private:
    struct TileRequest {
        int x{0};
        int y{0};
        int retry{0};
    };
    struct ActiveRequest {
        TileRequest tile;
        QByteArray payload;
    };

    void setState(const QString &value);
    void setError(const QString &value);
    void pump();
    void startTile(const TileRequest &tile);
    void finishTile(QNetworkReply *reply);
    QString cachePath(int zoom, int x, int y) const;
    bool validCachedTile(const QString &path, QImage *decoded = nullptr) const;
    void finishMissionIfComplete();
    void failVisualProfile(const QString &reason);

    QNetworkAccessManager m_network;
    QQueue<TileRequest> m_queue;
    QHash<QNetworkReply *, ActiveRequest> m_active;
    // Keep decoded images only for route-profile sampling to avoid a second decode.
    QHash<QString, QImage> m_decodedProfileTiles;
    QVariantList m_completed;
    QString m_missionId;
    QString m_cacheDirectory;
    QString m_state{QStringLiteral("IDLE")};
    QString m_lastError;
    QString m_apiKey;
    QVariantMap m_pendingRouteSnapshot;
    bool m_visualProfileRequested{false};
    quint64 m_profileGeneration{0};
    int m_zoom{0};
    int m_completedCount{0};
    int m_totalCount{0};
    bool m_cancelled{false};
    static constexpr int kMaximumTiles = 256;
    static constexpr int kMaximumConcurrent = 4;
    static constexpr int kMaximumPayloadBytes = 8 * 1024 * 1024;
};
