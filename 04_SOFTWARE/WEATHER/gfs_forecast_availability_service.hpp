#pragma once
#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>
#include <QVector>
#include "gfs_forecast_planner.hpp"
class QNetworkReply;
namespace bluesky::weather {
class GfsForecastAvailabilityService final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
public:
    explicit GfsForecastAvailabilityService(QObject *parent = nullptr);
    QString state() const;
    QString lastError() const;
    Q_INVOKABLE bool checkMissionForecast(const QString &missionStartUtc,
                                          const QString &missionEndUtc,
                                          int lookbackDays = 7);
    Q_INVOKABLE void cancel();
    static QUrl buildAtmosDirectoryUrl(const QString &cycleTimeUtc, QString *error = nullptr);
    static QSet<int> parseAvailableForecastHours(const QByteArray &directoryHtml,
                                                 const QString &cycleHourUtc,
                                                 bool *listingRecognized = nullptr);
    static QStringList recentCandidateCyclesUtc(const QDateTime &nowUtc,
                                                int lookbackDays,
                                                QString *error = nullptr);
    // Availability metadata expires independently of immutable forecast products.
    static constexpr int availabilityCacheTtlSeconds() noexcept { return 600; }
    static bool isAvailabilityCacheFresh(const QDateTime &checkedAtUtc,
                                         const QDateTime &nowUtc,
                                         int ttlSeconds = availabilityCacheTtlSeconds());
signals:
    void stateChanged();
    void lastErrorChanged();
    void progressChanged(int checkedCandidates, int totalCandidates, const QString &cycleTimeUtc);
    void planReady(const QVariantMap &plan);
    void checkFailed(const QString &reason, const QVariantList &candidateDiagnostics);
private:
    struct Candidate { QString cycleTimeUtc; GfsForecastPlan plan; };
    struct CachedAvailabilityListing {
        QSet<int> forecastHours;
        QDateTime checkedAtUtc;
    };
    void checkNextCandidate();
    void finishCurrentCandidate();
    void evaluateCurrentCandidate(const QString &cycleTimeUtc,
                                  const QSet<int> &availableHours,
                                  const QDateTime &checkedAtUtc,
                                  bool cacheHit);
    void failCheck(const QString &reason);
    void setState(const QString &value);
    void setError(const QString &value);
    QVariantList diagnostics() const;
    QNetworkAccessManager m_network;
    QPointer<QNetworkReply> m_reply;
    QVector<Candidate> m_candidates;
    QVariantList m_candidateDiagnostics;
    QHash<QString, CachedAvailabilityListing> m_availabilityCache;
    int m_candidateIndex{0};
    QString m_missionStartUtc, m_missionEndUtc;
    QString m_state{QStringLiteral("IDLE")};
    QString m_lastError;
};
} // namespace bluesky::weather
