#pragma once

#include <QObject>
#include <QString>

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

    // Called only by the future validated telemetry adapter. This method is not
    // a MAVLink transport and must not be fed preview or generic GPS altitude.
    bool publishDecision(double valueM, const QString &source,
                                     bool isDegraded, bool sourceClockNormalized);
    void markUnavailable();

signals:
    void changed();

private:
    QString m_valueText{QStringLiteral("—")};
    QString m_sourceText{QStringLiteral("ALTITUDE")};
    QString m_statusText{QStringLiteral("UNAVAILABLE")};
    bool m_available{false};
    bool m_degraded{false};
};
