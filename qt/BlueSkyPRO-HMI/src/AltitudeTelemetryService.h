#pragma once

#include "AltitudeTelemetryBridge.h"
#include "mavlink_transport_channel_runtime.hpp"

#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QTimer>

#include <memory>
#include <string>

class AltitudeTelemetryService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit AltitudeTelemetryService(AltitudeTelemetryBridge &bridge,
                                      QObject *parent = nullptr);
    ~AltitudeTelemetryService() override;

    QString status() const;
    bool startFromEnvironment();
    void stop();

signals:
    void statusChanged();

private slots:
    void poll();

private:
    void setStatus(const QString &status);
    bool configureFromEnvironment();

    AltitudeTelemetryBridge &m_bridge;
    std::unique_ptr<bluesky::operations::MavlinkTransportChannelRuntime> m_transport;
    QElapsedTimer m_clock;
    QTimer m_timer;
    QString m_status{QStringLiteral("CONFIG_REQUIRED")};
    QString m_channelId;
    QString m_sessionId;
    bool m_reconnectPending{false};
    qint64 m_reconnectAtMs{0};
};
