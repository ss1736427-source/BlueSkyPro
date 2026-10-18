#include "AltitudeDisplayBridge.h"

#include <cmath>

AltitudeDisplayBridge::AltitudeDisplayBridge(QObject *parent)
    : QObject(parent)
{
}

QString AltitudeDisplayBridge::valueText() const { return m_valueText; }
QString AltitudeDisplayBridge::sourceText() const { return m_sourceText; }
QString AltitudeDisplayBridge::statusText() const { return m_statusText; }
bool AltitudeDisplayBridge::available() const { return m_available; }
bool AltitudeDisplayBridge::degraded() const { return m_degraded; }

bool AltitudeDisplayBridge::publishDecision(double valueM, const QString &source,
                                            bool isDegraded,
                                            bool sourceClockNormalized)
{
    const bool radio = source == QLatin1String("RA_AGL");
    const bool barometric = source == QLatin1String("BARO_STD");
    if (!std::isfinite(valueM) || valueM < 0.0 || (!radio && !barometric)
        || !sourceClockNormalized) {
        markUnavailable();
        return false;
    }

    // Unnormalized source clocks are rejected. DEGRADED is reserved for a
    // clock-qualified decision whose adapter explicitly reports degraded data.
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
