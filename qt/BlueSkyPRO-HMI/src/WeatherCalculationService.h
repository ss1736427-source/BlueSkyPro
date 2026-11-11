#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>

class WeatherCalculationService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QVariantMap result READ result NOTIFY resultChanged)
    Q_PROPERTY(bool decoderAvailable READ decoderAvailable CONSTANT)

public:
    explicit WeatherCalculationService(QObject* parent = nullptr);

    QString state() const { return m_state; }
    QString lastError() const { return m_lastError; }
    QVariantMap result() const { return m_result; }
    bool decoderAvailable() const;

    Q_INVOKABLE bool evaluateRoute(
        const QVariantList& forecastProducts,
        const QVariantMap& routeSnapshot,
        const QString& missionModelJson,
        const QString& missionStartUtc,
        const QVariantMap& validatedPerformanceProfile);

    // Weather-only re-ranking of a complete, already validated candidate set.
    // Every candidate must carry nominal metrics and a matching flight-safety attestation.
    Q_INVOKABLE bool evaluateCandidates(
        const QVariantList& forecastProducts,
        const QVariantList& candidateSnapshots,
        const QString& missionModelJson,
        const QString& missionStartUtc,
        const QVariantMap& validatedPerformanceProfile,
        const QStringList& objectivePriorities);

signals:
    void stateChanged();
    void lastErrorChanged();
    void resultChanged();
    void evaluationFinished(const QVariantMap& result);

private:
    void setState(const QString& state);
    void setError(const QString& error);

    QString m_state{QStringLiteral("IDLE")};
    QString m_lastError;
    QVariantMap m_result;
    QFutureWatcher<QVariantMap> m_watcher;
};
