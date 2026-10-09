#include "RouteGeometrySerializer.h"

#include <QJsonArray>
#include <QJsonObject>

#include <cmath>
#include <set>
#include <string>

namespace bluesky::planning {
namespace {

bool reject(QString *error, const QString &reason)
{
    if (error)
        *error = reason;
    return false;
}

} // namespace

bool serializeRouteGeometry(const Route &route, QJsonObject &output, QString *error)
{
    const auto &lineage = route.lineage;
    if (lineage.route_id.empty() || lineage.route_version.empty())
        return reject(error, QStringLiteral("ROUTE_ID_AND_VERSION_REQUIRED"));

    if (route.waypoints.size() < 2)
        return reject(error, QStringLiteral("ROUTE_REQUIRES_AT_LEAST_TWO_POINTS"));

    QJsonArray points;
    std::set<std::string> waypointIds;

    for (const auto &waypoint : route.waypoints) {
        const auto &position = waypoint.position;
        if (waypoint.waypoint_id.empty())
            return reject(error, QStringLiteral("WAYPOINT_ID_REQUIRED"));
        if (!waypointIds.insert(waypoint.waypoint_id).second)
            return reject(error, QStringLiteral("DUPLICATE_WAYPOINT_ID"));
        if (!std::isfinite(position.latitude_deg)
            || position.latitude_deg < -90.0 || position.latitude_deg > 90.0
            || !std::isfinite(position.longitude_deg)
            || position.longitude_deg < -180.0 || position.longitude_deg > 180.0)
            return reject(error, QStringLiteral("INVALID_WGS84_COORDINATE"));
        if (!std::isfinite(waypoint.altitude_m))
            return reject(error, QStringLiteral("INVALID_WAYPOINT_ALTITUDE"));

        QJsonObject point;
        point.insert(QStringLiteral("waypointId"),
                     QString::fromStdString(waypoint.waypoint_id));
        point.insert(QStringLiteral("latitude"), position.latitude_deg);
        point.insert(QStringLiteral("longitude"), position.longitude_deg);
        point.insert(QStringLiteral("altitudeM"), waypoint.altitude_m);
        point.insert(QStringLiteral("mandatory"), waypoint.mandatory);
        points.append(point);
    }

    QJsonObject geometry;
    geometry.insert(QStringLiteral("routeId"), QString::fromStdString(lineage.route_id));
    geometry.insert(QStringLiteral("routeVersion"), QString::fromStdString(lineage.route_version));
    geometry.insert(QStringLiteral("coordinateReference"), QStringLiteral("WGS84"));
    geometry.insert(QStringLiteral("points"), points);

    output = geometry;
    if (error)
        error->clear();
    return true;
}

bool attachRouteGeometryToRequest(const Route &route,
                                  QJsonObject &request,
                                  QString *error)
{
    if (request.value(QStringLiteral("schemaVersion")).toString() != QLatin1String("1.0")
        || request.value(QStringLiteral("messageType")).toString() != QLatin1String("planning.request")
        || request.value(QStringLiteral("missionId")).toString().isEmpty()
        || request.value(QStringLiteral("resultId")).toString().isEmpty()) {
        return reject(error, QStringLiteral("INVALID_PLANNING_REQUEST_ENVELOPE"));
    }

    QJsonObject inputs = request.value(QStringLiteral("inputs")).toObject();
    if (inputs.isEmpty())
        return reject(error, QStringLiteral("PLANNING_REQUEST_INPUTS_REQUIRED"));

    QJsonObject geometry;
    if (!serializeRouteGeometry(route, geometry, error))
        return false;

    inputs.insert(QStringLiteral("routeGeometry"), geometry);
    request.insert(QStringLiteral("inputs"), inputs);
    if (error)
        error->clear();
    return true;
}

} // namespace bluesky::planning
