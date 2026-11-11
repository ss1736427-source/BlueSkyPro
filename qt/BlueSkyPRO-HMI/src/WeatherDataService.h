#pragma once

#include <QCryptographicHash>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <QVariantMap>
#include <QStringList>
#include <QVariantList>

class QFile;
class QNetworkReply;

class WeatherDataService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString cacheDirectory READ cacheDirectory CONSTANT)
    Q_PROPERTY(QString lastDownloadedFile READ lastDownloadedFile NOTIFY lastDownloadedFileChanged)

public:
    explicit WeatherDataService(QObject *parent = nullptr);

    QString state() const;
    QString lastError() const;
    QString cacheDirectory() const;
    QString lastDownloadedFile() const;

    // Downloads a bounded NOAA GFS 0.25-degree GRIB2 wind subset asynchronously.
    // Supported direct AGL levels are confirmed by the current NOMADS filter:
    // 2, 10, 20, 30, 40, 50, 80, 100, 1000 and 4000 metres.
    Q_INVOKABLE bool requestWindSubset(const QString &dateUtc,
                                       const QString &cycleUtc,
                                       int forecastHour,
                                       double west,
                                       double south,
                                       double east,
                                       double north,
                                       const QStringList &levelsAglM);

    // Downloads an ordered set of forecast leads sequentially for one common
    // region and vertical level set. Existing SHA-256 cache entries are reused.
    Q_INVOKABLE bool requestWindSubsetBatch(const QString &dateUtc,
                                            const QString &cycleUtc,
                                            const QVariantList &forecastHours,
                                            double west,
                                            double south,
                                            double east,
                                            double north,
                                            const QStringList &levelsAglM);

    Q_INVOKABLE void cancelPending();

    static QUrl buildRequestUrl(const QString &dateUtc,
                                const QString &cycleUtc,
                                int forecastHour,
                                double west,
                                double south,
                                double east,
                                double north,
                                const QStringList &levelsAglM,
                                QString *error = nullptr);
    static bool isValidGrib2File(const QString &path, QString *error = nullptr);
    // Exact canonical URL identity lets concurrent callers join one in-flight transfer.
    static bool isSameRequestIdentity(const QUrl &activeRequestUrl,
                                      const QUrl &requestedUrl);
    // A cached forecast is reusable only when its exact request identity, valid time,
    // file size and SHA-256 all match the manifest. Forecast products are immutable;
    // freshness of the selected model cycle is checked by GFS availability planning.
    static bool isReusableCacheEntry(const QString &filePath,
                                     const QString &manifestPath,
                                     const QString &expectedRequestUrl,
                                     const QString &expectedModelRunUtc,
                                     int expectedForecastHour,
                                     QString *error = nullptr);

signals:
    void stateChanged();
    void lastErrorChanged();
    void lastDownloadedFileChanged();
    void progressChanged(qint64 receivedBytes, qint64 maximumBytes);
    void forecastReady(const QString &filePath, const QVariantMap &manifest);
    void downloadFailed(const QString &reason);
    void forecastBatchReady(const QStringList &filePaths, const QVariantList &manifests);
    void forecastBatchFailed(const QString &reason, const QStringList &completedFilePaths);

private:
    void setState(const QString &value);
    void setError(const QString &value);
    void consumeReplyData();
    void startNextBatchProduct();
    void clearBatch();
    void finishReply();
    void failDownload(const QString &reason);
    bool writeManifest(const QJsonObject &manifest, QString *error = nullptr);

    static constexpr qint64 kMaximumDownloadBytes = 256LL * 1024LL * 1024LL;

    QNetworkAccessManager m_network;
    QPointer<QNetworkReply> m_reply;
    QUrl m_activeRequestUrl;
    QFile *m_partialFile{nullptr};
    QCryptographicHash m_hash{QCryptographicHash::Sha256};
    QString m_state{QStringLiteral("IDLE")};
    QString m_lastError;
    QString m_cacheDirectory;
    QString m_lastDownloadedFile;
    QString m_partialPath;
    QString m_finalPath;
    QString m_manifestPath;
    QJsonObject m_manifest;
    qint64 m_receivedBytes{0};
    bool m_batchActive{false};
    QString m_batchDateUtc;
    QString m_batchCycleUtc;
    double m_batchWest{0.0}, m_batchSouth{0.0}, m_batchEast{0.0}, m_batchNorth{0.0};
    QStringList m_batchLevelsAglM;
    QList<int> m_batchForecastHours;
    int m_batchNextIndex{0};
    QStringList m_batchFilePaths;
    QVariantList m_batchManifests;
};
