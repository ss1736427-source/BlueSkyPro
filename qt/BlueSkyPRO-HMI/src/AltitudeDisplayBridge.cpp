#include "AltitudeDisplayBridge.h"

#include <cmath>

using bluesky::operations::AltitudeDisplaySource;
using bluesky::operations::MavlinkAltitudeDisplayAdapterResult;

AltitudeDisplayBridge::AltitudeDisplayBridge(QObject *parent)
    : QObject(parent)
{
}

QString AltitudeDisplayBridge::valueText() const { return m_valueText; }
QString AltitudeDisplayBridge::sourceText() const { return m_sourceText; }
QString AltitudeDisplayBridge::statusText() const { return m_statusText; }
bool AltitudeDisplayBridge::available() const { return m_available; }
bool AltitudeDisplayBridge::degraded() const { return m_degraded; }

bool AltitudeDisplayBridge::publishAdapterResult(
    const MavlinkAltitudeDisplayAdapterResult &result)
{
    const auto &decision = result.decision;
    if (!decision.value_available || !std::isfinite(decision.value_m)
        || decision.value_m < 0.0) {
        markUnavailable();
        return false;
    }

    QString source;
    switch (decision.source) {
    case AltitudeDisplaySource::RadioAltimeterAgl:
        source = QStringLiteral("RA_AGL");
        break;
    case AltitudeDisplaySource::BarometricStandardPressure:
    case AltitudeDisplaySource::BarometricStandardPressureDegraded:
        source = QStringLiteral("BARO_STD");
        break;
    case AltitudeDisplaySource::Unavailable:
        markUnavailable();
        return false;
    }

    // The adapter owns freshness and timestamp-domain qualification. Preserve
    // its degraded flag and explicitly report receive-time fallback quality.
    return publishDecision(decision.value_m, source,
                           decision.degraded || result.timestamp_quality_degraded);
}

bool AltitudeDisplayBridge::publishDecision(double valueM, const QString &source,
                                            bool isDegraded)
{
    const bool radio = source == QLatin1String("RA_AGL");
    const bool barometric = source == QLatin1String("BARO_STD");
    if (!std::isfinite(valueM) || valueM < 0.0 || (!radio && !barometric)) {
        markUnavailable();
        return false;
    }

    m_valueText = QString::number(valueM, 'f', 1) + QStringLiteral(" m");
    m_sourceText = radio ? QStringLiteral("RA · AGL")
                         : QStringLiteral("BARO · STD 1013.25 hPa");
    m_available = true;
    m_degraded = isDegraded;
    m_statusText = m_degraded ? QStringLiteral("DEGRADED") : QStringLiteral("LIVE");
    emit changed();
    return true;
}

void AltitudeDisplayBridge::markUnavailable()
{
    const bool wasChanged = m_valueText != QStringLiteral("—")
        || m_sourceText != QStringLiteral("ALTITUDE")
        || m_statusText != QStringLiteral("UNAVAILABLE")
        || m_available || m_degraded;
    m_valueText = QStringLiteral("—");
    m_sourceText = QStringLiteral("ALTITUDE");
    m_statusText = QStringLiteral("UNAVAILABLE");
    m_available = false;
    m_degraded = false;
    if (wasChanged)
        emit changed();
}
