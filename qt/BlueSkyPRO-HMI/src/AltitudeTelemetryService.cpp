#include "AltitudeTelemetryService.h"

#include <QMetaObject>
#include <QVariant>

#include <limits>

using namespace bluesky::operations;

namespace {
bool requiredString(const char *name, QString &value)
{
    if (!qEnvironmentVariableIsSet(name))
        return false;
    value = qEnvironmentVariable(name).trimmed();
    return !value.isEmpty();
}

bool requiredInt(const char *name, int minimum, int maximum, int &value)
{
    bool ok = false;
    const QString raw = qEnvironmentVariable(name);
    value = raw.toInt(&ok);
    return ok && value >= minimum && value <= maximum;
}

bool requiredPositiveLongLong(const char *name, qint64 &value)
{
    bool ok = false;
    value = qEnvironmentVariable(name).toLongLong(&ok);
    return ok && value > 0;
}

bool requiredBool(const char *name, bool &value)
{
    if (!qEnvironmentVariableIsSet(name))
        return false;
    const QString raw = qEnvironmentVariable(name).trimmed().toLower();
    if (raw == QStringLiteral("true") || raw == QStringLiteral("1")) {
        value = true;
        return true;
    }
    if (raw == QStringLiteral("false") || raw == QStringLiteral("0")) {
        value = false;
        return true;
    }
    return false;
}
}

AltitudeTelemetryService::AltitudeTelemetryService(AltitudeTelemetryBridge &bridge,
                                                   QObject *parent)
    : QObject(parent), m_bridge(bridge)
{
    m_clock.start();
    m_timer.setInterval(20);
    connect(&m_timer, &QTimer::timeout, this, &AltitudeTelemetryService::poll);
}

AltitudeTelemetryService::~AltitudeTelemetryService()
{
    stop();
}

QString AltitudeTelemetryService::status() const
{
    return m_status;
}

void AltitudeTelemetryService::setStatus(const QString &statusValue)
{
    if (m_status == statusValue)
        return;
    m_status = statusValue;
    emit statusChanged();
}

bool AltitudeTelemetryService::startFromEnvironment()
{
    if (m_timer.isActive())
        return true;

    m_clock.restart();
    if (!configureFromEnvironment())
        return false;

    m_transport = std::make_unique<MavlinkTransportChannelRuntime>();
    MavlinkTransportChannelConfig config;
    QString channelId;
    QString sessionId;
    QString vehicleId;
    QString sourceId;
    QString dialectName;
    QString localHost;
    QString peerHost;
    int systemId = 0;
    int componentId = 0;
    int localPort = 0;
    int peerPort = 0;
    int radioConfiguredInt = 0;
    bool radioConfigured = false;
    int sensorId = 0;
    int sensorType = 0;
    qint64 maximumAgeMs = 0;
    qint64 maximumPairSkewMs = 0;

    const bool identityOk =
        requiredString("BLUESKY_MAVLINK_CHANNEL_ID", channelId) &&
        requiredString("BLUESKY_MAVLINK_SESSION_ID", sessionId) &&
        requiredString("BLUESKY_ALTITUDE_VEHICLE_ID", vehicleId) &&
        requiredString("BLUESKY_ALTITUDE_SOURCE_ID", sourceId) &&
        requiredInt("BLUESKY_ALTITUDE_SYSTEM_ID", 1, 255, systemId) &&
        requiredInt("BLUESKY_ALTITUDE_COMPONENT_ID", 1, 255, componentId) &&
        requiredString("BLUESKY_MAVLINK_DIALECT", dialectName) &&
        requiredString("BLUESKY_MAVLINK_UDP_LOCAL_HOST", localHost) &&
        requiredInt("BLUESKY_MAVLINK_UDP_LOCAL_PORT", 1, 65535, localPort) &&
        requiredString("BLUESKY_MAVLINK_UDP_PEER_HOST", peerHost) &&
        requiredInt("BLUESKY_MAVLINK_UDP_PEER_PORT", 1, 65535, peerPort) &&
        requiredBool("BLUESKY_ALTITUDE_RADIO_CONFIGURED", radioConfigured) &&
        requiredPositiveLongLong("BLUESKY_ALTITUDE_MAX_AGE_MS", maximumAgeMs) &&
        requiredPositiveLongLong("BLUESKY_ALTITUDE_MAX_PAIR_SKEW_MS", maximumPairSkewMs);

    if (!identityOk ||
        (radioConfigured &&
         (!requiredInt("BLUESKY_ALTITUDE_SENSOR_ID", 0, 255, sensorId) ||
          !requiredInt("BLUESKY_ALTITUDE_SENSOR_TYPE", 0, 255, sensorType)))) {
        m_transport.reset();
        setStatus(QStringLiteral("CONFIG_INVALID"));
        return false;
    }

    if (dialectName == QStringLiteral("PX4")) {
        config.dialect = MavlinkDialect::PX4;
    } else if (dialectName == QStringLiteral("ARDUPILOT")) {
        config.dialect = MavlinkDialect::ArduPilot;
    } else {
        m_transport.reset();
        setStatus(QStringLiteral("CONFIG_INVALID"));
        return false;
    }

    config.channel_id = channelId.toStdString();
    config.session_id = sessionId.toStdString();
    config.vehicle_id = vehicleId.toStdString();
    config.source_id = sourceId.toStdString();
    config.system_id = static_cast<std::uint8_t>(systemId);
    config.component_id = static_cast<std::uint8_t>(componentId);
    config.transport = MavlinkTransportType::Udp;
    config.udp_local = MavlinkUdpEndpoint{localHost.toStdString(),
                                          static_cast<std::uint16_t>(localPort)};
    config.udp_ingress_peer = MavlinkUdpEndpoint{peerHost.toStdString(),
                                                 static_cast<std::uint16_t>(peerPort)};

    if (!m_transport->registerChannel(config)) {
        m_transport.reset();
        setStatus(QStringLiteral("CHANNEL_REGISTER_FAILED"));
        return false;
    }

    if (!m_bridge.configure(vehicleId, sourceId, systemId, componentId,
                            radioConfigured, sensorId, sensorType,
                            maximumAgeMs, maximumPairSkewMs)) {
        m_transport.reset();
        setStatus(QStringLiteral("ALTITUDE_CONFIG_REJECTED"));
        return false;
    }

    if (!m_transport->connect(channelId.toStdString())) {
        m_transport.reset();
        setStatus(QStringLiteral("UDP_BIND_FAILED"));
        return false;
    }

    m_channelId = channelId;
    m_sessionId = sessionId;
    m_timer.start();
    setStatus(QStringLiteral("RUNNING"));
    return true;
}

bool AltitudeTelemetryService::configureFromEnvironment()
{
    // Configuration is intentionally explicit. No port, peer, vehicle, or
    // sensor identity is inferred, and incomplete configuration stays offline.
    return true;
}

void AltitudeTelemetryService::stop()
{
    m_timer.stop();
    if (m_transport && !m_channelId.isEmpty())
        m_transport->disconnect(m_channelId.toStdString());
    m_transport.reset();
    m_reconnectPending = false;
    setStatus(QStringLiteral("STOPPED"));
}

void AltitudeTelemetryService::poll()
{
    if (!m_transport || m_channelId.isEmpty() || m_sessionId.isEmpty())
        return;

    const qint64 nowMs = qMax<qint64>(1, m_clock.elapsed());

    if (m_reconnectPending) {
        if (nowMs >= m_reconnectAtMs) {
            if (m_transport->reconnect(m_channelId.toStdString())) {
                m_reconnectPending = false;
                setStatus(QStringLiteral("RUNNING"));
            } else {
                m_reconnectAtMs = nowMs + 1000;
                setStatus(QStringLiteral("RECONNECTING"));
            }
        }
        m_bridge.refresh(nowMs);
        return;
    }

    m_transport->pollReceive(m_channelId.toStdString(), nowMs);
    while (const auto message = m_transport->takeAcceptedMessage(m_channelId.toStdString()))
        m_bridge.ingestAcceptedMessage(*message, nowMs);

    const auto session = m_transport->tickSession(m_sessionId.toStdString(), nowMs);
    if (session.heartbeat_seen && session.link_state == MavlinkLinkState::Lost) {
        m_transport->fail(m_channelId.toStdString());
        m_reconnectPending = true;
        m_reconnectAtMs = nowMs + 1000;
        setStatus(QStringLiteral("LINK_LOST"));
    }

    m_bridge.refresh(nowMs);
}
