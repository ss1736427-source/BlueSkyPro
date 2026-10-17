#include "AltitudeTelemetryBridge.h"

#include <QVariant>

#include <cstdint>
#include <limits>
#include <utility>

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

bool hasNumeric(const QVariantMap &m, const QString &key)
{
    return m.contains(key) && m.value(key).isValid() && m.value(key).canConvert<double>();
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
    // Sensor identifiers and freshness/coherence policy are part of the
    // vehicle capability configuration; never infer them from incoming frames.
    if (vehicleId.trimmed().isEmpty() || sourceId.trimmed().isEmpty() ||
        systemId <= 0 || systemId > 255 || componentId <= 0 || componentId > 255 ||
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

bool AltitudeTelemetryBridge::ingestAcceptedMessage(const QVariantMap &message,
                                                    qint64 nowMonotonicMs)
{
    if (!m_configured || !m_runtime || nowMonotonicMs <= 0) {
        publishUnavailable(QStringLiteral("MISSING"));
        return false;
    }

    // This typed map is a narrow Qt boundary. The caller must translate the
    // actual DecodedMavlinkMessage obtained from takeAcceptedMessage(); raw
    // frame bytes, QML fixtures and generic position altitude are not accepted.
    const auto kindString = message.value(QStringLiteral("kind")).toString();
    MavlinkMessageKind kind;
    if (kindString == QLatin1String("ScaledPressure")) {
        kind = MavlinkMessageKind::ScaledPressure;
    } else if (kindString == QLatin1String("DistanceSensor")) {
        kind = MavlinkMessageKind::DistanceSensor;
    } else {
        return false;
    }

    for (const auto &key : {QStringLiteral("vehicleId"), QStringLiteral("sourceId"),
                            QStringLiteral("systemId"), QStringLiteral("componentId"),
                            QStringLiteral("receivedTimestampMs"), QStringLiteral("sourceTimeBootMs")}) {
        if (!message.contains(key) || !message.value(key).isValid()) {
            return false;
        }
    }
    if (message.value(QStringLiteral("vehicleId")).toString() != m_vehicleId ||
        message.value(QStringLiteral("sourceId")).toString() != m_sourceId ||
        message.value(QStringLiteral("systemId")).toInt() != m_systemId ||
        message.value(QStringLiteral("componentId")).toInt() != m_componentId) {
        return false;
    }

    const auto receiveTimestamp = message.value(QStringLiteral("receivedTimestampMs")).toLongLong();
    const auto bootTime = message.value(QStringLiteral("sourceTimeBootMs")).toUInt();
    if (receiveTimestamp <= 0 || bootTime == 0 || receiveTimestamp > nowMonotonicMs) {
        return false;
    }

    DecodedMavlinkMessage decoded;
    decoded.kind = kind;
    decoded.vehicle_id = m_vehicleId.toStdString();
    decoded.source_id = m_sourceId.toStdString();
    decoded.system_id = static_cast<std::uint8_t>(m_systemId);
    decoded.component_id = static_cast<std::uint8_t>(m_componentId);
    decoded.source_timestamp_ms = receiveTimestamp;
    decoded.received_timestamp_ms = receiveTimestamp;
    decoded.timestamp_is_receive_time_fallback =
        message.value(QStringLiteral("timestampQualityDegraded"), true).toBool();
    decoded.source_time_boot_ms = bootTime;
    decoded.valid = true;

    if (kind == MavlinkMessageKind::DistanceSensor) {
        for (const auto &key : {QStringLiteral("sensorId"), QStringLiteral("sensorType"),
                                QStringLiteral("orientation"), QStringLiteral("distanceM"),
                                QStringLiteral("minimumDistanceCm"), QStringLiteral("maximumDistanceCm")}) {
            if (!message.contains(key) || !message.value(key).isValid()) {
                return false;
            }
        }
        MavlinkDistanceSensorSample sensor;
        sensor.source_time_boot_ms = bootTime;
        sensor.received_timestamp_ms = receiveTimestamp;
        sensor.sensor_id = static_cast<std::uint8_t>(message.value(QStringLiteral("sensorId")).toInt());
        sensor.sensor_type = static_cast<std::uint8_t>(message.value(QStringLiteral("sensorType")).toInt());
        sensor.orientation = static_cast<std::uint8_t>(message.value(QStringLiteral("orientation")).toInt());
        sensor.minimum_distance_cm = static_cast<std::uint16_t>(message.value(QStringLiteral("minimumDistanceCm")).toUInt());
        sensor.maximum_distance_cm = static_cast<std::uint16_t>(message.value(QStringLiteral("maximumDistanceCm")).toUInt());
        const double distance = message.value(QStringLiteral("distanceM")).toDouble();
        if (sensor.sensor_id != message.value(QStringLiteral("sensorId")).toInt() ||
            sensor.sensor_type != message.value(QStringLiteral("sensorType")).toInt() ||
            sensor.orientation != message.value(QStringLiteral("orientation")).toInt() ||
            !hasNumeric(message, QStringLiteral("distanceM")) ||
            sensor.minimum_distance_cm > sensor.maximum_distance_cm ||
            distance < 0.0 || distance * 100.0 < sensor.minimum_distance_cm ||
            distance * 100.0 > sensor.maximum_distance_cm) {
            return false;
        }
        sensor.distance_m = distance;
        sensor.valid = true;
        decoded.distance_sensor = sensor;
    } else {
        if (!hasNumeric(message, QStringLiteral("absolutePressureHpa"))) {
            return false;
        }
        const double pressure = message.value(QStringLiteral("absolutePressureHpa")).toDouble();
        if (pressure < 100.0 || pressure > 1200.0) {
            return false;
        }
        decoded.barometric_pressure = MavlinkBarometricPressureSample{
            bootTime, receiveTimestamp, pressure, true};
    }

    if (!m_runtime->ingestAccepted(decoded)) {
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
    const bool isDegraded = result.decision.degraded || result.timestamp_quality_degraded;
    const QString freshnessValue = availableValue ? (isDegraded ? QStringLiteral("DEGRADED") :
                                                       QStringLiteral("CURRENT")) :
                                                   QStringLiteral("STALE_OR_MISSING");
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
