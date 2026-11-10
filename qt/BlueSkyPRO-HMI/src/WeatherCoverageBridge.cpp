#include "WeatherCoverageBridge.h"

#include "04_SOFTWARE/PLANNING/model/mission_model.hpp"
#include "04_SOFTWARE/PLANNING/model/route_model.hpp"
#include "04_SOFTWARE/WEATHER/weather_coverage_request.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStringList>

#include <cmath>
#include <string>
#include <utility>

using bluesky::planning::AltitudeReference;
using bluesky::planning::Mission;
using bluesky::planning::MissionGeometryObject;
using bluesky::planning::MissionGeometryVertex;
using bluesky::planning::MissionOrigin;
using bluesky::planning::Route;
using bluesky::planning::RouteWaypoint;
using bluesky::weather::WeatherCoverageRequestBuilder;
using bluesky::weather::WeatherCoverageRequestOptions;

namespace
{
QVariantMap failure(const QString &code)
{
    return {
        {QStringLiteral("valid"), false},
        {QStringLiteral("errors"), QStringList{code}},
        {QStringLiteral("request"), QVariantMap{}}
    };
}

bool readRequiredString(const QJsonObject &object, const QString &key, std::string &target)
{
    const QJsonValue value = object.value(key);
    if (!value.isString() || value.toString().trimmed().isEmpty())
        return false;
    target = value.toString().trimmed().toStdString();
    return true;
}

bool readFiniteNumber(const QJsonObject &object, const QString &key, double &target)
{
    const QJsonValue value = object.value(key);
    if (!value.isDouble() || !std::isfinite(value.toDouble()))
        return false;
    target = value.toDouble();
    return true;
}
}

QVariantMap WeatherCoverageBridge::buildCoverageRequest(
    const QString &missionModelJson,
    const QVariantMap &routeSnapshot,
    const QString &validFromUtc,
    const QString &validToUtc,
    double spatialBufferM,
    const QVariantList &requiredAreaAltitudesAglM) const
{
    if (missionModelJson.trimmed().isEmpty())
        return failure(QStringLiteral("MISSION_MODEL_JSON_REQUIRED"));

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(missionModelJson.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
        return failure(QStringLiteral("MISSION_MODEL_JSON_INVALID"));

    const QJsonObject root = document.object();
    Mission mission;
    if (!readRequiredString(root, QStringLiteral("missionId"), mission.identity.mission_id))
        return failure(QStringLiteral("MISSION_ID_REQUIRED"));

    const QJsonValue missionVersion = root.value(QStringLiteral("missionVersion"));
    if (missionVersion.isString())
        mission.identity.mission_version = missionVersion.toString().toStdString();
    else if (missionVersion.isDouble() && missionVersion.toInt() > 0)
        mission.identity.mission_version = std::to_string(missionVersion.toInt());
    else
        return failure(QStringLiteral("MISSION_VERSION_REQUIRED"));

    const QJsonObject selectedSolution = root.value(QStringLiteral("selectedSolution")).toObject();
    if (selectedSolution.contains(QStringLiteral("calculationInputVersion"))) {
        const QJsonValue inputVersion =
            selectedSolution.value(QStringLiteral("calculationInputVersion"));
        if (!inputVersion.isString())
            return failure(QStringLiteral("MISSION_CALCULATION_INPUT_VERSION_INVALID"));
        mission.selected_solution.calculation_input_version =
            inputVersion.toString().toStdString();
    }

    const QJsonValue geometryValue = root.value(QStringLiteral("geometryObjects"));
    if (!geometryValue.isArray())
        return failure(QStringLiteral("MISSION_GEOMETRY_OBJECTS_INVALID"));

    for (const QJsonValue &geometryValueItem : geometryValue.toArray()) {
        if (!geometryValueItem.isObject())
            return failure(QStringLiteral("MISSION_GEOMETRY_OBJECT_INVALID"));
        const QJsonObject geometryJson = geometryValueItem.toObject();
        MissionGeometryObject geometry;
        if (!readRequiredString(geometryJson, QStringLiteral("geometryId"), geometry.geometry_id)
            || !readRequiredString(geometryJson, QStringLiteral("geometryType"), geometry.geometry_type)
            || !readRequiredString(geometryJson, QStringLiteral("coordinateReferenceSystem"),
                                   geometry.coordinate_reference_system))
            return failure(QStringLiteral("MISSION_GEOMETRY_ID_TYPE_CRS_REQUIRED"));
        const QJsonValue versionValue = geometryJson.value(QStringLiteral("version"));
        if (!versionValue.isDouble() || versionValue.toInt() < 1)
            return failure(QStringLiteral("MISSION_GEOMETRY_VERSION_INVALID"));
        geometry.version = versionValue.toInt();

        const QJsonValue coordinatesValue = geometryJson.value(QStringLiteral("coordinates"));
        if (!coordinatesValue.isArray())
            return failure(QStringLiteral("MISSION_GEOMETRY_COORDINATES_INVALID"));
        for (const QJsonValue &coordinateValue : coordinatesValue.toArray()) {
            if (!coordinateValue.isArray() || coordinateValue.toArray().size() < 2)
                return failure(QStringLiteral("MISSION_GEOMETRY_COORDINATE_INVALID"));
            const QJsonArray coordinate = coordinateValue.toArray();
            if (!coordinate.at(0).isDouble() || !coordinate.at(1).isDouble())
                return failure(QStringLiteral("MISSION_GEOMETRY_COORDINATE_INVALID"));
            MissionGeometryVertex vertex;
            vertex.longitude_deg = coordinate.at(0).toDouble();
            vertex.latitude_deg = coordinate.at(1).toDouble();
            if (coordinate.size() >= 3) {
                if (!coordinate.at(2).isDouble())
                    return failure(QStringLiteral("MISSION_GEOMETRY_ALTITUDE_INVALID"));
                vertex.altitude_m = coordinate.at(2).toDouble();
            }
            if (coordinate.size() >= 4) {
                if (!coordinate.at(3).isString())
                    return failure(QStringLiteral("MISSION_GEOMETRY_ALTITUDE_REFERENCE_INVALID"));
                vertex.altitude_reference = coordinate.at(3).toString().toStdString();
            }
            geometry.coordinates.push_back(vertex);
        }
        mission.geometry_objects.push_back(std::move(geometry));
    }

    const QJsonValue originValue = root.value(QStringLiteral("origin"));
    if (!originValue.isUndefined() && !originValue.isNull()) {
        if (!originValue.isObject())
            return failure(QStringLiteral("MISSION_ORIGIN_INVALID"));
        const QJsonObject originJson = originValue.toObject();
        MissionOrigin origin;
        if (!readFiniteNumber(originJson, QStringLiteral("longitudeDeg"), origin.longitude_deg)
            || !readFiniteNumber(originJson, QStringLiteral("latitudeDeg"), origin.latitude_deg)
            || !readRequiredString(originJson, QStringLiteral("coordinateReferenceSystem"),
                                   origin.coordinate_reference_system)
            || !readRequiredString(originJson, QStringLiteral("originKind"), origin.origin_kind))
            return failure(QStringLiteral("MISSION_ORIGIN_FIELDS_INVALID"));
        mission.origin = origin;
    }

    Route route;
    const auto routeString = [&routeSnapshot](const QString &key, std::string &target) {
        const QString value = routeSnapshot.value(key).toString().trimmed();
        if (value.isEmpty())
            return false;
        target = value.toStdString();
        return true;
    };
    if (!routeString(QStringLiteral("routeId"), route.lineage.route_id)
        || !routeString(QStringLiteral("routeVersion"), route.lineage.route_version)
        || !routeString(QStringLiteral("missionId"), route.lineage.mission_id)
        || !routeString(QStringLiteral("missionVersion"), route.lineage.mission_version)
        || !routeString(QStringLiteral("calculationInputVersion"),
                        route.lineage.calculation_input_version))
        return failure(QStringLiteral("ROUTE_LINEAGE_REQUIRED"));

    const QVariantList waypoints = routeSnapshot.value(QStringLiteral("waypoints")).toList();
    if (waypoints.size() < 2)
        return failure(QStringLiteral("ROUTE_REQUIRES_AT_LEAST_TWO_WAYPOINTS"));
    for (const QVariant &waypointValue : waypoints) {
        const QVariantMap point = waypointValue.toMap();
        RouteWaypoint waypoint;
        waypoint.waypoint_id = point.value(QStringLiteral("waypointId")).toString().toStdString();
        bool latOk = false, lonOk = false, altitudeOk = false;
        waypoint.position.latitude_deg =
            point.value(QStringLiteral("latitudeDeg")).toDouble(&latOk);
        waypoint.position.longitude_deg =
            point.value(QStringLiteral("longitudeDeg")).toDouble(&lonOk);
        waypoint.altitude_m = point.value(QStringLiteral("altitudeM")).toDouble(&altitudeOk);
        if (waypoint.waypoint_id.empty() || !latOk || !lonOk || !altitudeOk)
            return failure(QStringLiteral("ROUTE_WAYPOINT_FIELDS_INVALID"));
        if (point.value(QStringLiteral("altitudeReference")).toString() != QLatin1String("AGL"))
            return failure(QStringLiteral("ROUTE_WAYPOINT_REQUIRES_AGL"));
        waypoint.altitude_reference = AltitudeReference::AGL;
        waypoint.mandatory = point.value(QStringLiteral("mandatory")).toBool();
        route.waypoints.push_back(std::move(waypoint));
    }

    WeatherCoverageRequestOptions options;
    options.spatial_buffer_m = spatialBufferM;
    options.time_window.valid_from_utc = validFromUtc.toStdString();
    options.time_window.valid_to_utc = validToUtc.toStdString();
    for (const QVariant &value : requiredAreaAltitudesAglM) {
        bool ok = false;
        const double altitude = value.toDouble(&ok);
        if (!ok || !std::isfinite(altitude))
            return failure(QStringLiteral("AREA_ALTITUDE_INVALID"));
        options.required_area_altitudes_agl_m.push_back(altitude);
    }

    const auto built = WeatherCoverageRequestBuilder::build(mission, route, options);
    QVariantList errors;
    for (const std::string &error : built.errors)
        errors.push_back(QString::fromStdString(error));
    if (!built.valid)
        return {
            {QStringLiteral("valid"), false},
            {QStringLiteral("errors"), errors},
            {QStringLiteral("request"), QVariantMap{}}
        };

    const auto &request = built.request;
    QVariantList components;
    for (const auto &component : request.components) {
        QVariantList coordinates;
        for (const auto &coordinate : component.coordinates)
            coordinates.push_back(QVariantMap{
                {QStringLiteral("latitudeDeg"), coordinate.latitude_deg},
                {QStringLiteral("longitudeDeg"), coordinate.longitude_deg}
            });
        components.push_back(QVariantMap{
            {QStringLiteral("kind"), static_cast<int>(component.kind)},
            {QStringLiteral("sourceId"), QString::fromStdString(component.source_id)},
            {QStringLiteral("sourceVersion"), QString::fromStdString(component.source_version)},
            {QStringLiteral("geometryType"), QString::fromStdString(component.geometry_type)},
            {QStringLiteral("coordinates"), coordinates}
        });
    }

    QVariantList altitudes;
    for (double altitude : request.required_altitudes_agl_m)
        altitudes.push_back(altitude);
    QStringList providerLevels;
    for (int level : request.provider_levels_agl_m)
        providerLevels.push_back(QString::number(level));

    const QVariantMap requestMap{
        {QStringLiteral("schemaVersion"), QString::fromStdString(request.request_schema_version)},
        {QStringLiteral("missionId"), QString::fromStdString(request.mission_id)},
        {QStringLiteral("missionVersion"), QString::fromStdString(request.mission_version)},
        {QStringLiteral("routeId"), QString::fromStdString(request.route_id)},
        {QStringLiteral("routeVersion"), QString::fromStdString(request.route_version)},
        {QStringLiteral("calculationInputVersion"), QString::fromStdString(request.calculation_input_version)},
        {QStringLiteral("components"), components},
        {QStringLiteral("bounds"), QVariantMap{
            {QStringLiteral("southDeg"), request.bounds.south_deg},
            {QStringLiteral("westDeg"), request.bounds.west_deg},
            {QStringLiteral("northDeg"), request.bounds.north_deg},
            {QStringLiteral("eastDeg"), request.bounds.east_deg}
        }},
        {QStringLiteral("spatialBufferM"), request.spatial_buffer_m},
        {QStringLiteral("validFromUtc"), QString::fromStdString(request.time_window.valid_from_utc)},
        {QStringLiteral("validToUtc"), QString::fromStdString(request.time_window.valid_to_utc)},
        {QStringLiteral("requiredAltitudesAglM"), altitudes},
        {QStringLiteral("providerLevelsAglM"), providerLevels}
    };

    return {
        {QStringLiteral("valid"), true},
        {QStringLiteral("errors"), QVariantList{}},
        {QStringLiteral("request"), requestMap}
    };
}
