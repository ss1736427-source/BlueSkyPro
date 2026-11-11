#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

class WeatherCoverageBridge final : public QObject
{
    Q_OBJECT

public:
    explicit WeatherCoverageBridge(QObject *parent = nullptr) : QObject(parent) {}

    // Converts the canonical mission JSON and RouteController snapshot into
    // the typed weather-coverage contract. No forecast is downloaded here.
    Q_INVOKABLE QVariantMap buildCoverageRequest(
        const QString &missionModelJson,
        const QVariantMap &routeSnapshot,
        const QString &validFromUtc,
        const QString &validToUtc,
        double spatialBufferM,
        const QVariantList &requiredAreaAltitudesAglM) const;

    // Builds one union coverage request for every route candidate before the
    // shared forecast batch is downloaded.
    Q_INVOKABLE QVariantMap buildCoverageRequestForCandidates(
        const QString &missionModelJson,
        const QVariantList &routeSnapshots,
        const QString &validFromUtc,
        const QString &validToUtc,
        double spatialBufferM,
        const QVariantList &requiredAreaAltitudesAglM) const;
};
