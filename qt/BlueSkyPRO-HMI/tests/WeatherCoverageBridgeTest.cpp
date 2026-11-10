#include "WeatherCoverageBridge.h"

#include <cassert>
#include <cmath>

int main()
{
    WeatherCoverageBridge bridge;
    const QString missionJson = QStringLiteral(R"({
        "missionId":"MISSION-WX-01",
        "missionVersion":3,
        "geometryObjects":[{
            "geometryId":"AOI-01",
            "geometryType":"POLYGON_AOI",
            "coordinateReferenceSystem":"EPSG:4326",
            "version":2,
            "coordinates":[[37.60,55.70],[37.61,55.70],[37.61,55.71],[37.60,55.71]]
        }],
        "origin":{
            "longitudeDeg":37.60,
            "latitudeDeg":55.70,
            "coordinateReferenceSystem":"EPSG:4326",
            "originKind":"PLANNED_TAKEOFF"
        }
    })");

    QVariantList waypoints{
        QVariantMap{{"waypointId","WP-1"},{"latitudeDeg",55.70},{"longitudeDeg",37.60},
                    {"altitudeM",50.0},{"altitudeReference","AGL"},{"mandatory",true}},
        QVariantMap{{"waypointId","WP-2"},{"latitudeDeg",55.72},{"longitudeDeg",37.62},
                    {"altitudeM",80.0},{"altitudeReference","AGL"},{"mandatory",true}}
    };
    const QVariantMap route{
        {"routeId","ROUTE-01"},{"routeVersion","4"},
        {"missionId","MISSION-WX-01"},{"missionVersion","3"},
        {"calculationInputVersion","INPUT-7"},{"waypoints",waypoints}
    };

    const QVariantMap result = bridge.buildCoverageRequest(
        missionJson, route,
        QStringLiteral("2026-11-08T06:00:00Z"),
        QStringLiteral("2026-11-08T09:00:00Z"),
        1000.0, QVariantList{30.0, 50.0});
    assert(result.value(QStringLiteral("valid")).toBool());
    const QVariantMap request = result.value(QStringLiteral("request")).toMap();
    assert(request.value(QStringLiteral("missionId")).toString() == QLatin1String("MISSION-WX-01"));
    assert(request.value(QStringLiteral("routeId")).toString() == QLatin1String("ROUTE-01"));
    assert(request.value(QStringLiteral("components")).toList().size() == 3);
    const QVariantMap bounds = request.value(QStringLiteral("bounds")).toMap();
    assert(bounds.value(QStringLiteral("southDeg")).toDouble() < 55.70);
    assert(bounds.value(QStringLiteral("northDeg")).toDouble() > 55.72);
    assert(request.value(QStringLiteral("providerLevelsAglM")).toStringList().size() == 3);

    QVariantMap mismatchedRoute = route;
    mismatchedRoute.insert(QStringLiteral("missionVersion"), QStringLiteral("2"));
    const QVariantMap mismatch = bridge.buildCoverageRequest(
        missionJson, mismatchedRoute,
        QStringLiteral("2026-11-08T06:00:00Z"),
        QStringLiteral("2026-11-08T09:00:00Z"),
        1000.0, QVariantList{30.0, 50.0});
    assert(!mismatch.value(QStringLiteral("valid")).toBool());

    QVariantMap invalidDatumRoute = route;
    QVariantList invalidWaypoints = waypoints;
    QVariantMap invalidPoint = invalidWaypoints[0].toMap();
    invalidPoint.insert(QStringLiteral("altitudeReference"), QStringLiteral("AMSL"));
    invalidWaypoints[0] = invalidPoint;
    invalidDatumRoute.insert(QStringLiteral("waypoints"), invalidWaypoints);
    const QVariantMap invalidDatum = bridge.buildCoverageRequest(
        missionJson, invalidDatumRoute,
        QStringLiteral("2026-11-08T06:00:00Z"),
        QStringLiteral("2026-11-08T09:00:00Z"),
        1000.0, QVariantList{30.0, 50.0});
    assert(!invalidDatum.value(QStringLiteral("valid")).toBool());

    const QVariantMap invalidTime = bridge.buildCoverageRequest(
        missionJson, route,
        QStringLiteral("not-a-time"),
        QStringLiteral("2026-11-08T09:00:00Z"),
        1000.0, QVariantList{30.0, 50.0});
    assert(!invalidTime.value(QStringLiteral("valid")).toBool());

    return 0;
}
