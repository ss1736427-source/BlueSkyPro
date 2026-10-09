#include "../src/RouteGeometrySerializer.h"

#include <QJsonArray>
#include <QJsonObject>

#include <cassert>
#include <cmath>

using namespace bluesky::planning;

int main()
{
    Route route;
    route.lineage.route_id = "ROUTE-001";
    route.lineage.route_version = "v3";
    route.waypoints = {
        {"WP-START", {55.75, 37.61}, 100.0, true},
        {"WP-END", {55.76, 37.62}, 125.0, false}
    };

    QJsonObject geometry;
    QString error;
    assert(serializeRouteGeometry(route, geometry, &error));
    assert(error.isEmpty());
    assert(geometry.value("routeId").toString() == "ROUTE-001");
    assert(geometry.value("routeVersion").toString() == "v3");
    assert(geometry.value("coordinateReference").toString() == "WGS84");

    const QJsonArray points = geometry.value("points").toArray();
    assert(points.size() == 2);
    assert(points.at(0).toObject().value("waypointId").toString() == "WP-START");
    assert(points.at(0).toObject().value("mandatory").toBool());
    assert(std::abs(points.at(1).toObject().value("altitudeM").toDouble() - 125.0) < 1e-9);

    QJsonObject request{
        {"schemaVersion", "1.0"},
        {"messageType", "planning.request"},
        {"missionId", "MISSION-001"},
        {"resultId", "RESULT-001"},
        {"inputs", QJsonObject{
            {"zoneStatus", "READY"},
            {"routes", QJsonArray{QJsonObject{{"routeId", "ROUTE-001"}}}},
            {"authoritativeInputMarker", "preserve-me"}
        }}
    };
    assert(attachRouteGeometryToRequest(route, request, &error));
    assert(error.isEmpty());
    const QJsonObject inputs = request.value("inputs").toObject();
    assert(inputs.value("authoritativeInputMarker").toString() == "preserve-me");
    assert(inputs.value("routes").toArray().size() == 1);
    assert(inputs.value("routeGeometry").toObject().value("routeVersion").toString() == "v3");
    assert(inputs.value("routeGeometry").toObject().value("points").toArray().size() == 2);

    Route invalid = route;
    invalid.waypoints[1].position.latitude_deg = 95.0;
    QJsonObject untouched{{"sentinel", true}};
    assert(!serializeRouteGeometry(invalid, untouched, &error));
    assert(error == "INVALID_WGS84_COORDINATE");
    assert(untouched.value("sentinel").toBool());

    QJsonObject requestBeforeFailure = request;
    assert(!attachRouteGeometryToRequest(invalid, request, &error));
    assert(error == "INVALID_WGS84_COORDINATE");
    assert(request == requestBeforeFailure);

    invalid = route;
    invalid.waypoints[1].waypoint_id = invalid.waypoints[0].waypoint_id;
    assert(!serializeRouteGeometry(invalid, untouched, &error));
    assert(error == "DUPLICATE_WAYPOINT_ID");

    invalid = route;
    invalid.waypoints.pop_back();
    assert(!serializeRouteGeometry(invalid, untouched, &error));
    assert(error == "ROUTE_REQUIRES_AT_LEAST_TWO_POINTS");

    QJsonObject invalidEnvelope = request;
    invalidEnvelope.insert("messageType", "planning.result");
    assert(!attachRouteGeometryToRequest(route, invalidEnvelope, &error));
    assert(error == "INVALID_PLANNING_REQUEST_ENVELOPE");
    return 0;
}
