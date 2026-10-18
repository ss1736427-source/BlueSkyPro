#pragma once

#include <QObject>
#include <QString>

#include "mavlink_altitude_display_adapter.hpp"

class AltitudeDisplayBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString valueText READ valueText NOTIFY changed)
    Q_PROPERTY(QString sourceText READ sourceText NOTIFY changed)
    Q_PROPERTY(QString statusText READ statusText NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(bool degraded READ degraded NOTIFY changed)

public:
    explicit AltitudeDisplayBridge(QObject *parent = nullptr);

    QString valueText() const;
    QString sourceText() const;
    QString statusText() const;
    bool available() const;
    bool degraded() const;

    // Accept only the typed decision emitted by the validated operations adapter.
    // This is not a MAVLink transport and is not exposed as a QML-invokable method.
    bool publishAdapterResult(
        const bluesky::operations::MavlinkAltitudeDisplayAdapterResult &result);
    void markUnavailable();

private:
    bool publishDecision(double valueM, const QString &source, bool isDegraded);

signals:
    void changed();

private:
    QString m_valueText{QStringLiteral("—")};
    QString m_sourceText{QStringLiteral("ALTITUDE")};
    QString m_statusText{QStringLiteral("UNAVAILABLE")};
    bool m_available{false};
    bool m_degraded{false};
};
