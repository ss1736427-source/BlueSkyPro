#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

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
        const QStringList& forecastFiles,
        const QVariantMap& routeSnapshot,
        const QString& missionModelJson,
        const QString& missionStartUtc,
        const QVariantMap& validatedPerformanceProfile);

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
