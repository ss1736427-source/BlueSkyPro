#pragma once

#include "mavlink_altitude_display_runtime.hpp"

#include <QObject>
#include <QVariantMap>

#include <memory>

class AltitudeTelemetryBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY stateChanged)
    Q_PROPERTY(double valueM READ valueM NOTIFY stateChanged)
    Q_PROPERTY(QString sourceLabel READ sourceLabel NOTIFY stateChanged)
    Q_PROPERTY(bool degraded READ degraded NOTIFY stateChanged)
    Q_PROPERTY(bool timestampQualityDegraded READ timestampQualityDegraded NOTIFY stateChanged)
    Q_PROPERTY(QString freshness READ freshness NOTIFY stateChanged)

public:
    explicit AltitudeTelemetryBridge(QObject *parent = nullptr);

    bool available() const;
    double valueM() const;
    QString sourceLabel() const;
    bool degraded() const;
    bool timestampQualityDegraded() const;
    QString freshness() const;

    // Explicitly configure the selected vehicle and radio-altimeter capability.
    // An unconfigured bridge stays unavailable and cannot infer a sensor ID.
    Q_INVOKABLE bool configure(const QString &vehicleId,
                               const QString &sourceId,
                               int systemId,
                               int componentId,
                               bool radioAltimeterConfigured,
                               int radioAltimeterSensorId,
                               int radioAltimeterSensorType,
                               qint64 maximumAgeMs,
                               qint64 maximumPairSkewMs);

    // Input must come from MavlinkTransportChannelRuntime::takeAcceptedMessage().
    // This method accepts only decoded source-specific altitude frames.
    Q_INVOKABLE bool ingestAcceptedMessage(const QVariantMap &message,
                                            qint64 nowMonotonicMs);

signals:
    void stateChanged();

private:
    void publishUnavailable(const QString &freshness);
    void publishResult(const bluesky::operations::MavlinkAltitudeDisplayAdapterResult &result);

    std::unique_ptr<bluesky::operations::MavlinkAltitudeDisplayRuntime> m_runtime;
    QString m_vehicleId;
    QString m_sourceId;
    int m_systemId{0};
    int m_componentId{0};
    bool m_configured{false};
    bool m_available{false};
    double m_valueM{0.0};
    QString m_sourceLabel{QStringLiteral("UNAVAILABLE")};
    bool m_degraded{false};
    bool m_timestampQualityDegraded{false};
    QString m_freshness{QStringLiteral("MISSING")};
};
