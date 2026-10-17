#include "AltitudeTelemetryBridge.h"

using namespace bluesky::operations;

namespace {
QString sourceLabelFor(AltitudeDisplaySource source)
{
    switch (source) {
    case AltitudeDisplaySource::RadioAltimeterAgl:
        return QStringLiteral("RA · AGL");
    case AltitudeDisplaySource::BarometricStandardPressure:
        return QStringLiteral("BARO · STD 1013.25 hPa");
    case AltitudeDisplaySource::BarometricStandardPressureDegraded:
        return QStringLiteral("BARO · STD 1013.25 hPa · DEGRADED");
    case AltitudeDisplaySource::Unavailable:
    default:
        return QStringLiteral("UNAVAILABLE");
    }
}
}

AltitudeTelemetryBridge::AltitudeTelemetryBridge(QObject *parent)
    : QObject(parent)
{
}

bool AltitudeTelemetryBridge::available() const { return m_available; }
double AltitudeTelemetryBridge::valueM() const { return m_valueM; }
QString AltitudeTelemetryBridge::sourceLabel() const { return m_sourceLabel; }
bool AltitudeTelemetryBridge::degraded() const { return m_degraded; }
bool AltitudeTelemetryBridge::timestampQualityDegraded() const { return m_timestampQualityDegraded; }
QString AltitudeTelemetryBridge::freshness() const { return m_freshness; }

bool AltitudeTelemetryBridge::configure(const QString &vehicleId,
                                        const QString &sourceId,
                                        int systemId,
                                        int componentId,
                                        bool radioAltimeterConfigured,
                                        int radioAltimeterSensorId,
                                        int radioAltimeterSensorType,
                                        qint64 maximumAgeMs,
                                        qint64 maximumPairSkewMs)
{
    // Vehicle/capability configuration must supply exact identity, sensor
    // capability and freshness windows. No sensor ID or port is inferred.
    if (vehicleId.trimmed().isEmpty() || sourceId.trimmed().isEmpty() ||
        systemId <= 0 || systemId > 255 ||
        componentId <= 0 || componentId > 255 ||
        (radioAltimeterConfigured &&
         (radioAltimeterSensorId < 0 || radioAltimeterSensorId > 255 ||
          radioAltimeterSensorType < 0 || radioAltimeterSensorType > 255)) ||
        maximumAgeMs < 0 || maximumPairSkewMs < 0) {
        m_configured = false;
        m_runtime.reset();
        publishUnavailable(QStringLiteral("CONFIG_INVALID"));
        return false;
    }

    MavlinkAltitudeDisplaySensorConfig sensorConfig;
    sensorConfig.radio_altimeter_configured = radioAltimeterConfigured;
    sensorConfig.radio_altimeter_sensor_id =
        static_cast<std::uint8_t>(radioAltimeterSensorId);
    sensorConfig.radio_altimeter_sensor_type =
        static_cast<std::uint8_t>(radioAltimeterSensorType);
    sensorConfig.maximum_age_ms = maximumAgeMs;
    sensorConfig.maximum_pair_skew_ms = maximumPairSkewMs;

    m_vehicleId = vehicleId.trimmed();
    m_sourceId = sourceId.trimmed();
    m_systemId = systemId;
    m_componentId = componentId;
    m_runtime = std::make_unique<MavlinkAltitudeDisplayRuntime>(
        sensorConfig, m_vehicleId.toStdString(), m_sourceId.toStdString(),
        static_cast<std::uint8_t>(m_systemId),
        static_cast<std::uint8_t>(m_componentId));
    m_configured = true;
    publishUnavailable(QStringLiteral("MISSING"));
    return true;
}

void AltitudeTelemetryBridge::refresh(qint64 nowMonotonicMs)
{
    if (!m_configured || !m_runtime || nowMonotonicMs <= 0) {
        publishUnavailable(QStringLiteral("MISSING"));
        return;
    }
    publishResult(m_runtime->evaluate(nowMonotonicMs));
}

bool AltitudeTelemetryBridge::ingestAcceptedMessage(
    const DecodedMavlinkMessage &message, qint64 nowMonotonicMs)
{
    if (!m_configured || !m_runtime || nowMonotonicMs <= 0) {
        publishUnavailable(QStringLiteral("MISSING"));
        return false;
    }

    // Caller must pass the accepted object from
    // MavlinkTransportChannelRuntime::takeAcceptedMessage(). QML has no write
    // path, and generic GLOBAL_POSITION_INT altitude is never accepted here.
    if (!message.valid || message.vehicle_id != m_vehicleId.toStdString() ||
        message.source_id != m_sourceId.toStdString() ||
        message.system_id != m_systemId ||
        message.component_id != m_componentId ||
        message.received_timestamp_ms <= 0 ||
        message.received_timestamp_ms > nowMonotonicMs ||
        (message.kind != MavlinkMessageKind::DistanceSensor &&
         message.kind != MavlinkMessageKind::ScaledPressure)) {
        return false;
    }

    if (!m_runtime->ingestAccepted(message)) {
        // Rejecting duplicate/out-of-order input must not leave a previous
        // sample annunciated as current. Re-evaluate freshness against 'now'.
        publishResult(m_runtime->evaluate(nowMonotonicMs));
        return false;
    }

    publishResult(m_runtime->evaluate(nowMonotonicMs));
    return true;
}

void AltitudeTelemetryBridge::publishUnavailable(const QString &freshnessValue)
{
    const bool changed = m_available || m_valueM != 0.0 ||
        m_sourceLabel != QStringLiteral("UNAVAILABLE") || m_degraded ||
        m_timestampQualityDegraded || m_freshness != freshnessValue;
    m_available = false;
    m_valueM = 0.0;
    m_sourceLabel = QStringLiteral("UNAVAILABLE");
    m_degraded = false;
    m_timestampQualityDegraded = false;
    m_freshness = freshnessValue;
    if (changed)
        emit stateChanged();
}

void AltitudeTelemetryBridge::publishResult(
    const MavlinkAltitudeDisplayAdapterResult &result)
{
    const bool availableValue = result.decision.value_available;
    const double value = availableValue ? result.decision.value_m : 0.0;
    const QString label = sourceLabelFor(result.decision.source);
    const bool isDegraded =
        result.decision.degraded || result.timestamp_quality_degraded;
    const QString freshnessValue = availableValue
        ? (isDegraded ? QStringLiteral("DEGRADED") : QStringLiteral("CURRENT"))
        : QStringLiteral("STALE_OR_MISSING");

    const bool changed = m_available != availableValue || m_valueM != value ||
        m_sourceLabel != label || m_degraded != isDegraded ||
        m_timestampQualityDegraded != result.timestamp_quality_degraded ||
        m_freshness != freshnessValue;
    m_available = availableValue;
    m_valueM = value;
    m_sourceLabel = label;
    m_degraded = isDegraded;
    m_timestampQualityDegraded = result.timestamp_quality_degraded;
    m_freshness = freshnessValue;
    if (changed)
        emit stateChanged();
}
