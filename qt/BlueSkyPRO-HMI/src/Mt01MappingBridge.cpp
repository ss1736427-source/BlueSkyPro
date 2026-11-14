#include "Mt01MappingBridge.h"

#include "04_SOFTWARE/PLANNING/mt01_mapping_pipeline.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QVariantList>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>

using namespace bluesky::planning;

namespace {

QStringList stringArray(const QJsonValue &value, const QString &errorCode)
{
    QStringList result;
    if (!value.isArray())
        return {errorCode};
    for (const QJsonValue &item : value.toArray()) {
        if (!item.isString() || item.toString().trimmed().isEmpty())
            return {errorCode};
        result.push_back(item.toString());
    }
    return result;
}

bool readNumber(const QVariantMap &map, const QString &key, double &target)
{
    const QVariant value = map.value(key);
    bool ok = false;
    const double number = value.toDouble(&ok);
    if (!value.isValid() || !ok || !std::isfinite(number))
        return false;
    target = number;
    return true;
}

QString altitudeReferenceName(AltitudeReference reference)
{
    switch (reference) {
    case AltitudeReference::AGL: return QStringLiteral("AGL");
    case AltitudeReference::AMSL: return QStringLiteral("AMSL");
    case AltitudeReference::FlightLevel: return QStringLiteral("FL");
    case AltitudeReference::Unknown: break;
    }
    return QStringLiteral("UNKNOWN");
}

QVariantMap serializeRouteCandidate(const Route &route)
{
    QVariantList waypoints;
    for (const RouteWaypoint &waypoint : route.waypoints) {
        QVariantMap item{
            {QStringLiteral("waypointId"), QString::fromStdString(waypoint.waypoint_id)},
            {QStringLiteral("latitudeDeg"), waypoint.position.latitude_deg},
            {QStringLiteral("longitudeDeg"), waypoint.position.longitude_deg},
            {QStringLiteral("altitudeM"), waypoint.altitude_m},
            {QStringLiteral("mandatory"), waypoint.mandatory},
            {QStringLiteral("altitudeReference"), altitudeReferenceName(waypoint.altitude_reference)}
        };
        if (waypoint.task_execution_duration_s)
            item.insert(QStringLiteral("taskExecutionDurationS"), *waypoint.task_execution_duration_s);
        if (waypoint.task_execution_energy_wh)
            item.insert(QStringLiteral("taskExecutionEnergyWh"), *waypoint.task_execution_energy_wh);
        waypoints.push_back(item);
    }

    QVariantList segments;
    for (const RouteSegment &segment : route.segments) {
        QVariantMap item{
            {QStringLiteral("segmentId"), QString::fromStdString(segment.segment_id)},
            {QStringLiteral("fromWaypointId"), QString::fromStdString(segment.from_waypoint_id)},
            {QStringLiteral("toWaypointId"), QString::fromStdString(segment.to_waypoint_id)},
            {QStringLiteral("distanceM"), segment.distance_m},
            {QStringLiteral("nominalSpeedMps"), segment.nominal_speed_mps}
        };
        if (segment.bank_angle_rad)
            item.insert(QStringLiteral("bankAngleRad"), *segment.bank_angle_rad);
        if (segment.lateral_acceleration_mps2)
            item.insert(QStringLiteral("lateralAccelerationMps2"), *segment.lateral_acceleration_mps2);
        segments.push_back(item);
    }

    const RouteLineage &lineage = route.lineage;
    const RouteConstraints &constraints = route.constraints;
    const RouteEnvironmentReference &environment = route.environment;
    return {
        {QStringLiteral("routeId"), QString::fromStdString(lineage.route_id)},
        {QStringLiteral("routeVersion"), QString::fromStdString(lineage.route_version)},
        {QStringLiteral("missionId"), QString::fromStdString(lineage.mission_id)},
        {QStringLiteral("missionVersion"), QString::fromStdString(lineage.mission_version)},
        {QStringLiteral("generatorId"), QString::fromStdString(lineage.generator_id)},
        {QStringLiteral("generatorVersion"), QString::fromStdString(lineage.generator_version)},
        {QStringLiteral("calculationInputVersion"), QString::fromStdString(lineage.calculation_input_version)},
        {QStringLiteral("coordinateReferenceSystem"), QStringLiteral("EPSG:4326")},
        {QStringLiteral("environmentSnapshotRefs"), QVariantMap{
            {QStringLiteral("terrainSnapshotId"), QString::fromStdString(environment.terrain_snapshot_id)},
            {QStringLiteral("airspaceSnapshotId"), QString::fromStdString(environment.airspace_snapshot_id)},
            {QStringLiteral("weatherSnapshotId"), QString::fromStdString(environment.weather_snapshot_id)},
            {QStringLiteral("windSnapshotId"), QString::fromStdString(environment.wind_snapshot_id)}
        }},
        {QStringLiteral("constraints"), QVariantMap{
            {QStringLiteral("minimumAltitudeM"), constraints.minimum_altitude_m},
            {QStringLiteral("maximumAltitudeM"), constraints.maximum_altitude_m},
            {QStringLiteral("minimumClearanceM"), constraints.minimum_clearance_m},
            {QStringLiteral("avoidRestrictedAreas"), constraints.avoid_restricted_areas}
        }},
        {QStringLiteral("waypoints"), waypoints},
        {QStringLiteral("segments"), segments},
        {QStringLiteral("trajectoryGeometryComplete"), route.trajectory_geometry_complete}
    };
}

QVariantMap blocked(const QString &code)
{
    return {
        {QStringLiteral("status"), QStringLiteral("NEEDS_INPUT")},
        {QStringLiteral("coverageCandidateAvailable"), false},
        {QStringLiteral("flightReleaseEligible"), false},
        {QStringLiteral("coverageValid"), false},
        {QStringLiteral("findings"), QStringList{code}}
    };
}

QVariantMap blockedFindings(const QStringList &findings)
{
    return {
        {QStringLiteral("status"), QStringLiteral("NEEDS_INPUT")},
        {QStringLiteral("coverageCandidateAvailable"), false},
        {QStringLiteral("flightReleaseEligible"), false},
        {QStringLiteral("coverageValid"), false},
        {QStringLiteral("findings"), findings}
    };
}

bool utcEpoch(const QString &value, std::int64_t &epoch)
{
    const QString text = value.trimmed();
    if (text.isEmpty() || !text.endsWith(QLatin1Char('Z')))
        return false;
    QDateTime parsed = QDateTime::fromString(text, Qt::ISODateWithMs);
    if (!parsed.isValid())
        parsed = QDateTime::fromString(text, Qt::ISODate);
    if (!parsed.isValid())
        return false;
    epoch = parsed.toUTC().toSecsSinceEpoch();
    return epoch > 0;
}

bool parseNotamSnapshot(const QVariantMap &parameters,
                        Mt01MappingRequest &request,
                        QStringList &errors)
{
    const auto requiredString = [&parameters](const QString &key) {
        return parameters.value(key).toString().trimmed();
    };
    const QString snapshotId = requiredString(QStringLiteral("notamSnapshotId"));
    const QString sourceId = requiredString(QStringLiteral("notamSourceId"));
    const QString sourceVersion = requiredString(QStringLiteral("notamSourceVersion"));
    if (snapshotId.isEmpty() || sourceId.isEmpty() || sourceVersion.isEmpty()
        || !parameters.contains(QStringLiteral("notamSnapshotEpochS"))
        || !parameters.contains(QStringLiteral("notamRecords"))) {
        errors.push_back(QStringLiteral("NOTAM_SNAPSHOT_IDENTITY_AND_RECORDS_REQUIRED"));
        return false;
    }
    bool epochOk = false;
    const qlonglong snapshotEpoch =
        parameters.value(QStringLiteral("notamSnapshotEpochS")).toLongLong(&epochOk);
    if (!epochOk || snapshotEpoch <= 0) {
        errors.push_back(QStringLiteral("NOTAM_SNAPSHOT_EPOCH_INVALID"));
        return false;
    }
    if (!utcEpoch(requiredString(QStringLiteral("plannedMissionStartUtc")),
                  request.planned_mission_start_epoch_s)
        || !utcEpoch(requiredString(QStringLiteral("plannedMissionEndUtc")),
                     request.planned_mission_end_epoch_s)
        || request.planned_mission_end_epoch_s < request.planned_mission_start_epoch_s) {
        errors.push_back(QStringLiteral("NOTAM_PLANNED_MISSION_UTC_WINDOW_REQUIRED"));
        return false;
    }
    double safetyBuffer = 0.0;
    if (!readNumber(parameters, QStringLiteral("notamSafetyBufferM"), safetyBuffer)
        || safetyBuffer <= 0.0) {
        errors.push_back(QStringLiteral("NOTAM_POSITIVE_SAFETY_BUFFER_REQUIRED"));
        return false;
    }
    request.notam_safety_buffer_m = safetyBuffer;
    request.notam_snapshot.snapshot_id = snapshotId.toStdString();
    request.notam_snapshot.source_id = sourceId.toStdString();
    request.notam_snapshot.source_version = sourceVersion.toStdString();
    request.notam_snapshot.evaluation_epoch_s = snapshotEpoch;
    double queryLatitude = 0.0, queryLongitude = 0.0, queryRadiusM = 0.0;
    if (!readNumber(parameters, QStringLiteral("notamQueryLatitude"), queryLatitude)
        || !readNumber(parameters, QStringLiteral("notamQueryLongitude"), queryLongitude)
        || !readNumber(parameters, QStringLiteral("notamQueryRadiusM"), queryRadiusM)
        || queryLatitude < -90.0 || queryLatitude > 90.0
        || queryLongitude < -180.0 || queryLongitude > 180.0
        || queryRadiusM <= 0.0) {
        errors.push_back(QStringLiteral("NOTAM_SNAPSHOT_COVERAGE_METADATA_REQUIRED"));
        return false;
    }
    request.notam_snapshot.coverage_center = {queryLatitude, queryLongitude};
    request.notam_snapshot.coverage_radius_m = queryRadiusM;
    request.notam_snapshot.planned_mission_start_epoch_s = request.planned_mission_start_epoch_s;
    request.notam_snapshot.planned_mission_end_epoch_s = request.planned_mission_end_epoch_s;

    const QVariantList records = parameters.value(QStringLiteral("notamRecords")).toList();
    for (const QVariant &recordValue : records) {
        const QVariantMap record = recordValue.toMap();
        const QString notamId = record.value(QStringLiteral("id")).toString().trimmed();
        if (notamId.isEmpty()) {
            errors.push_back(QStringLiteral("NOTAM_RECORD_ID_MISSING"));
            return false;
        }
        if (record.value(QStringLiteral("notamInterpretationVersion")).toInt() != 1) {
            errors.push_back(QStringLiteral("NOTAM_CANONICAL_INTERPRETATION_REQUIRED: ") + notamId);
            return false;
        }

        std::int64_t validFrom = 0, validTo = 0;
        if (!utcEpoch(record.value(QStringLiteral("effectiveStart")).toString(), validFrom)
            || !utcEpoch(record.value(QStringLiteral("effectiveEnd")).toString(), validTo)
            || validTo < validFrom) {
            errors.push_back(QStringLiteral("NOTAM_VALIDITY_INTERVAL_INVALID: ") + notamId);
            return false;
        }
        if (validFrom > request.planned_mission_end_epoch_s
            || validTo < request.planned_mission_start_epoch_s)
            continue;

        const QString restrictionStatus =
            record.value(QStringLiteral("restrictionGeometryStatus")).toString();
        if (restrictionStatus == QLatin1String("DISPLAY_ONLY_NON_AREA_GEOMETRY")
            && record.value(QStringLiteral("restrictionClass")).toString()
                == QLatin1String("DISPLAY_ONLY"))
            continue;

        const QString restrictionClass =
            record.value(QStringLiteral("restrictionClass")).toString();
        const QString interpretationState =
            record.value(QStringLiteral("interpretationState")).toString();
        if (restrictionClass == QLatin1String("DISPLAY_ONLY"))
            continue;
        if (restrictionClass != QLatin1String("PROHIBITED")) {
            errors.push_back(QStringLiteral("NOTAM_RESTRICTION_SEMANTICS_UNRESOLVED: ")
                             + notamId + QStringLiteral(" [") + restrictionClass
                             + QStringLiteral("]"));
            return false;
        }
        if (interpretationState != QLatin1String("CONFIRMED")
            || restrictionStatus != QLatin1String("VALIDATED")
            || !record.value(QStringLiteral("geometryValidated")).toBool()) {
            errors.push_back(QStringLiteral("NOTAM_RESTRICTION_GEOMETRY_OR_VERTICAL_BOUNDS_UNVALIDATED: ")
                             + notamId);
            return false;
        }

        const QVariantList components =
            record.value(QStringLiteral("planningComponents")).toList();
        if (components.isEmpty()) {
            errors.push_back(QStringLiteral("NOTAM_CANONICAL_PLANNING_COMPONENTS_MISSING: ")
                             + notamId);
            return false;
        }

        for (const QVariant &componentValue : components) {
            const QVariantMap component = componentValue.toMap();
            const QString componentId = component.value(QStringLiteral("componentId"))
                .toString().trimmed();
            if (componentId.isEmpty()
                || component.value(QStringLiteral("restrictionClass")).toString()
                    != QLatin1String("PROHIBITED")
                || component.value(QStringLiteral("interpretationState")).toString()
                    != QLatin1String("CONFIRMED")
                || !component.value(QStringLiteral("geometryValidated")).toBool()) {
                errors.push_back(QStringLiteral("NOTAM_CANONICAL_COMPONENT_NOT_CONFIRMED: ")
                                 + notamId);
                return false;
            }
            if (component.value(QStringLiteral("interiorRingCount")).toInt() > 0) {
                errors.push_back(QStringLiteral("NOTAM_INTERIOR_RINGS_UNSUPPORTED_FOR_PLANNING: ")
                                 + componentId);
                return false;
            }

            bool lowerOk = false, upperOk = false;
            const double lower = component.value(QStringLiteral("lowerMeters")).toDouble(&lowerOk);
            const double upper = component.value(QStringLiteral("upperMeters")).toDouble(&upperOk);
            const QString lowerDatum = component.value(QStringLiteral("lowerDatum"))
                .toString().trimmed().toUpper();
            const QString upperDatum = component.value(QStringLiteral("upperDatum"))
                .toString().trimmed().toUpper();
            NotamAltitudeReference datum = NotamAltitudeReference::Unknown;
            if (lowerDatum == upperDatum) {
                if (lowerDatum == QLatin1String("AGL")) datum = NotamAltitudeReference::AGL;
                else if (lowerDatum == QLatin1String("AMSL")
                         || lowerDatum == QLatin1String("MSL"))
                    datum = NotamAltitudeReference::AMSL;
                else if (lowerDatum == QLatin1String("FL"))
                    datum = NotamAltitudeReference::FlightLevel;
            }
            if (!lowerOk || !upperOk || !std::isfinite(lower) || !std::isfinite(upper)
                || lower > upper || datum == NotamAltitudeReference::Unknown) {
                errors.push_back(QStringLiteral("NOTAM_ALTITUDE_DATUM_UNSUPPORTED: ") + notamId);
                return false;
            }

            const QVariantList coordinates =
                component.value(QStringLiteral("coordinates")).toList();
            if (coordinates.size() < 4) {
                errors.push_back(QStringLiteral("NOTAM_COMPONENT_RING_INVALID: ") + componentId);
                return false;
            }

            NotamZone zone;
            zone.notam_id = notamId.toStdString();
            zone.component_id = componentId.toStdString();
            zone.restriction_type = NotamRestrictionType::Prohibited;
            zone.geometry_type = NotamGeometryType::Polygon;
            zone.lower_altitude_m = lower;
            zone.upper_altitude_m = upper;
            zone.altitude_reference = datum;
            zone.valid_from_epoch_s = validFrom;
            zone.valid_to_epoch_s = validTo;
            for (const QVariant &coordinateValue : coordinates) {
                const QVariantList coordinate = coordinateValue.toList();
                if (coordinate.size() < 2) {
                    errors.push_back(QStringLiteral("NOTAM_COMPONENT_COORDINATE_INVALID: ")
                                     + componentId);
                    return false;
                }
                bool longitudeOk = false, latitudeOk = false;
                const double longitude = coordinate.at(0).toDouble(&longitudeOk);
                const double latitude = coordinate.at(1).toDouble(&latitudeOk);
                if (!longitudeOk || !latitudeOk || !std::isfinite(longitude)
                    || !std::isfinite(latitude) || longitude < -180.0 || longitude > 180.0
                    || latitude < -90.0 || latitude > 90.0) {
                    errors.push_back(QStringLiteral("NOTAM_COMPONENT_COORDINATE_INVALID: ")
                                     + componentId);
                    return false;
                }
                zone.polygon.push_back({latitude, longitude});
            }
            if (zone.polygon.size() >= 2
                && std::abs(zone.polygon.front().latitude_deg - zone.polygon.back().latitude_deg) < 1e-9
                && std::abs(zone.polygon.front().longitude_deg - zone.polygon.back().longitude_deg) < 1e-9)
                zone.polygon.pop_back();
            if (zone.polygon.size() < 3) {
                errors.push_back(QStringLiteral("NOTAM_COMPONENT_RING_INVALID: ") + componentId);
                return false;
            }
            request.notam_snapshot.zones.push_back(std::move(zone));
        }
    }
    return true;
}

bool parseMission(const QJsonObject &json, Mission &mission, QStringList &errors)
{
    mission.identity.mission_id = json.value(QStringLiteral("missionId")).toString().toStdString();
    mission.identity.mission_version = json.value(QStringLiteral("missionVersion")).toString().toStdString();
    if (mission.identity.mission_version.empty() && json.value(QStringLiteral("missionVersion")).isDouble())
        mission.identity.mission_version = std::to_string(json.value(QStringLiteral("missionVersion")).toInt());
    mission.mission_mode = json.value(QStringLiteral("missionMode")).toString().toStdString();

    const QStringList templates = stringArray(json.value(QStringLiteral("selectedTemplateIds")),
                                               QStringLiteral("MISSION_TEMPLATE_IDS_INVALID"));
    if (templates.size() == 1 && templates.front() == QStringLiteral("MISSION_TEMPLATE_IDS_INVALID"))
        errors.append(templates);
    else
        for (const QString &id : templates) mission.selected_template_ids.push_back(id.toStdString());

    const QJsonObject task = json.value(QStringLiteral("task")).toObject();
    mission.task.description = task.value(QStringLiteral("description")).toString().toStdString();
    const QJsonObject intent = json.value(QStringLiteral("intent")).toObject();
    mission.intent.objective = intent.value(QStringLiteral("objective")).toString().toStdString();
    mission.intent.task_type = intent.value(QStringLiteral("taskType")).toString().toStdString();

    const QJsonValue vehiclesValue = json.value(QStringLiteral("vehicleAssignments"));
    if (!vehiclesValue.isArray()) {
        errors.push_back(QStringLiteral("VEHICLE_ASSIGNMENTS_INVALID"));
    } else {
        for (const QJsonValue &value : vehiclesValue.toArray()) {
            if (!value.isObject()) { errors.push_back(QStringLiteral("VEHICLE_ASSIGNMENT_INVALID")); continue; }
            const QJsonObject item = value.toObject();
            VehicleAssignment assignment;
            assignment.vehicle_id = item.value(QStringLiteral("vehicleId")).toString().toStdString();
            assignment.configuration_version = item.value(QStringLiteral("configurationVersion")).toString().toStdString();
            assignment.configuration_id = item.value(QStringLiteral("configurationId")).toString().toStdString();
            assignment.configuration_type = item.value(QStringLiteral("configurationType")).toString().toStdString();
            assignment.validation_state = item.value(QStringLiteral("validationState")).toString().toStdString();
            assignment.approval_state = item.value(QStringLiteral("approvalState")).toString().toStdString();
            mission.vehicle_assignments.push_back(std::move(assignment));
        }
    }

    const QJsonValue equipmentValue = json.value(QStringLiteral("equipmentAssignments"));
    if (!equipmentValue.isArray()) {
        errors.push_back(QStringLiteral("EQUIPMENT_ASSIGNMENTS_INVALID"));
    } else {
        for (const QJsonValue &value : equipmentValue.toArray()) {
            if (!value.isObject()) { errors.push_back(QStringLiteral("EQUIPMENT_ASSIGNMENT_INVALID")); continue; }
            const QJsonObject item = value.toObject();
            EquipmentAssignment assignment;
            assignment.vehicle_id = item.value(QStringLiteral("vehicleId")).toString().toStdString();
            assignment.equipment_id = item.value(QStringLiteral("equipmentId")).toString().toStdString();
            assignment.equipment_profile_id = item.value(QStringLiteral("equipmentProfileId")).toString().toStdString();
            assignment.equipment_class = item.value(QStringLiteral("equipmentClass")).toString().toStdString();
            assignment.configuration_version = item.value(QStringLiteral("configurationVersion")).toString().toStdString();
            assignment.compatibility_state = item.value(QStringLiteral("compatibilityState")).toString().toStdString();
            assignment.verification_state = item.value(QStringLiteral("verificationState")).toString().toStdString();
            assignment.calibration_state = item.value(QStringLiteral("calibrationState")).toString().toStdString();
            mission.equipment_assignments.push_back(std::move(assignment));
        }
    }

    const QJsonValue operationsValue = json.value(QStringLiteral("operations"));
    if (!operationsValue.isArray()) {
        errors.push_back(QStringLiteral("MISSION_OPERATIONS_INVALID"));
    } else {
        for (const QJsonValue &value : operationsValue.toArray()) {
            if (!value.isObject()) {
                errors.push_back(QStringLiteral("MISSION_OPERATION_INVALID"));
                continue;
            }
            const QJsonObject item = value.toObject();
            MissionOperation operation;
            operation.operation_id = item.value(QStringLiteral("operationId")).toString().trimmed().toStdString();
            operation.template_id = item.value(QStringLiteral("templateId")).toString().trimmed().toStdString();
            operation.operation_type = item.value(QStringLiteral("operationType")).toString().trimmed().toStdString();
            operation.geometry_id = item.value(QStringLiteral("geometryId")).toString().trimmed().toStdString();
            const QJsonValue equipmentIds = item.value(QStringLiteral("equipmentIds"));
            if (!equipmentIds.isArray()) {
                errors.push_back(QStringLiteral("MISSION_OPERATION_EQUIPMENT_IDS_INVALID"));
            } else {
                for (const QJsonValue &equipmentId : equipmentIds.toArray()) {
                    if (!equipmentId.isString() || equipmentId.toString().trimmed().isEmpty()) {
                        errors.push_back(QStringLiteral("MISSION_OPERATION_EQUIPMENT_ID_INVALID"));
                        continue;
                    }
                    operation.equipment_ids.push_back(equipmentId.toString().trimmed().toStdString());
                }
            }
            const QJsonValue estimateValue = item.value(QStringLiteral("executionEstimate"));
            if (!estimateValue.isObject()) {
                errors.push_back(QStringLiteral("MISSION_OPERATION_EXECUTION_ESTIMATE_INVALID"));
            } else {
                const QJsonObject estimate = estimateValue.toObject();
                auto &target = operation.execution_estimate;
                target.state = estimate.value(QStringLiteral("state")).toString().trimmed().toStdString();
                if (estimate.contains(QStringLiteral("durationS"))) {
                    if (!estimate.value(QStringLiteral("durationS")).isDouble())
                        errors.push_back(QStringLiteral("MISSION_OPERATION_DURATION_INVALID"));
                    else
                        target.duration_s = estimate.value(QStringLiteral("durationS")).toDouble();
                }
                if (estimate.contains(QStringLiteral("energyWh"))) {
                    if (!estimate.value(QStringLiteral("energyWh")).isDouble())
                        errors.push_back(QStringLiteral("MISSION_OPERATION_ENERGY_INVALID"));
                    else
                        target.energy_wh = estimate.value(QStringLiteral("energyWh")).toDouble();
                }
                target.source_type = estimate.value(QStringLiteral("sourceType")).toString().trimmed().toStdString();
                target.source_id = estimate.value(QStringLiteral("sourceId")).toString().trimmed().toStdString();
                target.source_version = estimate.value(QStringLiteral("sourceVersion")).toString().trimmed().toStdString();
                target.method_version = estimate.value(QStringLiteral("methodVersion")).toString().trimmed().toStdString();
                target.vehicle_id = estimate.value(QStringLiteral("vehicleId")).toString().trimmed().toStdString();
                target.vehicle_configuration_version =
                    estimate.value(QStringLiteral("vehicleConfigurationVersion")).toString().trimmed().toStdString();
                const QJsonValue profiles = estimate.value(QStringLiteral("equipmentProfileIds"));
                if (!profiles.isUndefined() && !profiles.isNull()) {
                    if (!profiles.isArray()) {
                        errors.push_back(QStringLiteral("MISSION_OPERATION_EQUIPMENT_PROFILES_INVALID"));
                    } else {
                        for (const QJsonValue &profile : profiles.toArray()) {
                            if (!profile.isString() || profile.toString().trimmed().isEmpty()) {
                                errors.push_back(QStringLiteral("MISSION_OPERATION_EQUIPMENT_PROFILE_INVALID"));
                                continue;
                            }
                            target.equipment_profile_ids.push_back(profile.toString().trimmed().toStdString());
                        }
                    }
                }
            }
            if (operation.operation_id.empty() || operation.template_id.empty()
                || operation.operation_type.empty() || operation.geometry_id.empty())
                errors.push_back(QStringLiteral("MISSION_OPERATION_IDENTITY_INCOMPLETE"));
            for (const auto &existing : mission.operations) {
                if (existing.operation_id == operation.operation_id) {
                    errors.push_back(QStringLiteral("MISSION_OPERATION_ID_DUPLICATE"));
                    break;
                }
            }
            mission.operations.push_back(std::move(operation));
        }
    }

    const QJsonValue geometriesValue = json.value(QStringLiteral("geometryObjects"));
    if (!geometriesValue.isArray()) {
        errors.push_back(QStringLiteral("MISSION_GEOMETRY_OBJECTS_INVALID"));
    } else {
        for (const QJsonValue &value : geometriesValue.toArray()) {
            if (!value.isObject()) { errors.push_back(QStringLiteral("MISSION_GEOMETRY_OBJECT_INVALID")); continue; }
            const QJsonObject item = value.toObject();
            MissionGeometryObject geometry;
            geometry.geometry_id = item.value(QStringLiteral("geometryId")).toString().toStdString();
            geometry.geometry_type = item.value(QStringLiteral("geometryType")).toString().toStdString();
            geometry.coordinate_reference_system =
                item.value(QStringLiteral("coordinateReferenceSystem")).toString().toStdString();
            geometry.version = item.value(QStringLiteral("version")).toInt(1);

            const QStringList geometryTemplates = stringArray(
                item.value(QStringLiteral("templateIds")), QStringLiteral("GEOMETRY_TEMPLATE_IDS_INVALID"));
            if (geometryTemplates.size() == 1
                && geometryTemplates.front() == QStringLiteral("GEOMETRY_TEMPLATE_IDS_INVALID")) {
                errors.append(geometryTemplates);
            } else {
                for (const QString &id : geometryTemplates)
                    geometry.template_ids.push_back(id.toStdString());
            }

            const QJsonValue coordinatesValue = item.value(QStringLiteral("coordinates"));
            if (!coordinatesValue.isArray()) {
                errors.push_back(QStringLiteral("GEOMETRY_COORDINATES_INVALID"));
            } else {
                for (const QJsonValue &pointValue : coordinatesValue.toArray()) {
                    if (!pointValue.isArray() || pointValue.toArray().size() < 2) {
                        errors.push_back(QStringLiteral("GEOMETRY_COORDINATE_INVALID"));
                        continue;
                    }
                    const QJsonArray point = pointValue.toArray();
                    if (!point.at(0).isDouble() || !point.at(1).isDouble()
                        || !std::isfinite(point.at(0).toDouble())
                        || !std::isfinite(point.at(1).toDouble())) {
                        errors.push_back(QStringLiteral("GEOMETRY_COORDINATE_INVALID"));
                        continue;
                    }
                    MissionGeometryVertex vertex;
                    vertex.longitude_deg = point.at(0).toDouble();
                    vertex.latitude_deg = point.at(1).toDouble();
                    if (point.size() > 2 && point.at(2).isDouble())
                        vertex.altitude_m = point.at(2).toDouble();
                    if (point.size() > 3 && point.at(3).isString())
                        vertex.altitude_reference = point.at(3).toString().toStdString();
                    geometry.coordinates.push_back(vertex);
                }
            }
            mission.geometry_objects.push_back(std::move(geometry));
        }
    }
    synchronizeMissionLifecycleIdentity(mission);
    return errors.isEmpty();
}

} // namespace

QVariantMap Mt01MappingBridge::calculate(const QString &missionJson,
                                         const QVariantMap &parameters) const
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(missionJson.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
        return blocked(QStringLiteral("MISSION_JSON_INVALID"));

    Mission mission;
    QStringList parseErrors;
    parseMission(document.object(), mission, parseErrors);
    if (!parseErrors.isEmpty()) {
        QVariantMap result = blocked(QStringLiteral("MISSION_JSON_INCOMPLETE"));
        result.insert(QStringLiteral("findings"), parseErrors);
        return result;
    }

    // Report mission/configuration blockers before requesting camera numbers.
    // This lets the HMI show the actual missing operator decisions instead of
    // hiding them behind a generic "numeric parameters required" message.
    QStringList assignmentFindings;
    if (mission.identity.mission_id.empty() || mission.identity.mission_version.empty())
        assignmentFindings.push_back(QStringLiteral("MISSION_IDENTITY_REQUIRED"));
    if (mission.task.description.empty())
        assignmentFindings.push_back(QStringLiteral("TASK_DESCRIPTION_REQUIRED"));
    if (mission.intent.objective.empty() || mission.intent.task_type.empty())
        assignmentFindings.push_back(QStringLiteral("MISSION_INTENT_REQUIRED"));
    if (mission.vehicle_assignments.size() != 1) {
        assignmentFindings.push_back(QStringLiteral("MT01_REQUIRES_EXACTLY_ONE_ASSIGNED_VEHICLE"));
    } else {
        const auto &vehicle = mission.vehicle_assignments.front();
        if (vehicle.vehicle_id.empty() || vehicle.configuration_id.empty()
            || vehicle.configuration_version.empty())
            assignmentFindings.push_back(QStringLiteral("VEHICLE_CONFIGURATION_BINDING_REQUIRED"));
        if (vehicle.configuration_type != "STANDARD"
            && vehicle.configuration_type != "CUSTOM"
            && vehicle.configuration_type != "DERIVED")
            assignmentFindings.push_back(QStringLiteral("VEHICLE_CONFIGURATION_TYPE_INVALID"));
        if (vehicle.validation_state != "VALIDATED"
            && vehicle.validation_state != "APPROVED")
            assignmentFindings.push_back(QStringLiteral("VEHICLE_CONFIGURATION_NOT_VALIDATED"));
        if (vehicle.configuration_type == "STANDARD"
            && vehicle.approval_state != "APPROVED")
            assignmentFindings.push_back(QStringLiteral("STANDARD_CONFIGURATION_NOT_APPROVED"));
        if (vehicle.configuration_type != "STANDARD"
            && vehicle.approval_state != "APPROVED"
            && vehicle.approval_state != "PENDING")
            assignmentFindings.push_back(QStringLiteral("CUSTOM_CONFIGURATION_APPROVAL_STATE_INVALID"));
    }

    const EquipmentAssignment *assignedCamera = nullptr;
    for (const auto &equipment : mission.equipment_assignments) {
        if (equipment.equipment_class != "CAMERA")
            continue;
        if (assignedCamera) {
            assignmentFindings.push_back(QStringLiteral("MT01_REQUIRES_EXACTLY_ONE_ASSIGNED_CAMERA"));
            break;
        }
        assignedCamera = &equipment;
    }
    if (!assignedCamera) {
        assignmentFindings.push_back(QStringLiteral("MT01_CAMERA_EQUIPMENT_ASSIGNMENT_REQUIRED"));
    } else {
        if (assignedCamera->vehicle_id.empty() || assignedCamera->equipment_id.empty()
            || assignedCamera->equipment_profile_id.empty()
            || assignedCamera->configuration_version.empty())
            assignmentFindings.push_back(QStringLiteral("CAMERA_EQUIPMENT_BINDING_REQUIRED"));
        if (mission.vehicle_assignments.size() == 1
            && assignedCamera->vehicle_id != mission.vehicle_assignments.front().vehicle_id)
            assignmentFindings.push_back(QStringLiteral("CAMERA_ASSIGNED_TO_DIFFERENT_VEHICLE"));
        if (assignedCamera->compatibility_state != "COMPATIBLE")
            assignmentFindings.push_back(QStringLiteral("CAMERA_COMPATIBILITY_NOT_CONFIRMED"));
        if (assignedCamera->verification_state != "VERIFIED")
            assignmentFindings.push_back(QStringLiteral("CAMERA_EQUIPMENT_NOT_VERIFIED"));
    }
    bool taskExecutionEstimatesReady = false;
    if (std::find(mission.selected_template_ids.begin(),
                  mission.selected_template_ids.end(), "MT-01")
        == mission.selected_template_ids.end())
        assignmentFindings.push_back(QStringLiteral("MT01_TEMPLATE_NOT_SELECTED"));
    const MissionGeometryObject *mappingAoi = nullptr;
    for (const auto &geometry : mission.geometry_objects) {
        if (geometry.geometry_type != "POLYGON_AOI"
            || std::find(geometry.template_ids.begin(), geometry.template_ids.end(), "MT-01")
                == geometry.template_ids.end())
            continue;
        if (mappingAoi) {
            assignmentFindings.push_back(QStringLiteral("MT01_REQUIRES_EXACTLY_ONE_POLYGON_AOI"));
            break;
        }
        mappingAoi = &geometry;
    }
    const MissionOperation *mappingOperation = nullptr;
    for (const auto &operation : mission.operations) {
        if (operation.template_id != "MT-01" || operation.operation_type != "MAPPING_SWATH")
            continue;
        if (mappingOperation) {
            assignmentFindings.push_back(QStringLiteral("MT01_REQUIRES_EXACTLY_ONE_MAPPING_SWATH_OPERATION"));
            break;
        }
        mappingOperation = &operation;
    }
    if (!mappingOperation) {
        assignmentFindings.push_back(QStringLiteral("MT01_MAPPING_SWATH_OPERATION_REQUIRED"));
    } else {
        if (!mappingAoi || mappingOperation->geometry_id != mappingAoi->geometry_id)
            assignmentFindings.push_back(QStringLiteral("MT01_MAPPING_OPERATION_AOI_BINDING_MISMATCH"));
        if (!assignedCamera
            || std::find(mappingOperation->equipment_ids.begin(),
                         mappingOperation->equipment_ids.end(),
                         assignedCamera->equipment_id) == mappingOperation->equipment_ids.end())
            assignmentFindings.push_back(QStringLiteral("MT01_MAPPING_OPERATION_CAMERA_BINDING_REQUIRED"));

        const auto &estimate = mappingOperation->execution_estimate;
        if (estimate.state == "UNRESOLVED") {
            if (estimate.duration_s || estimate.energy_wh || !estimate.source_type.empty()
                || !estimate.source_id.empty() || !estimate.source_version.empty()
                || !estimate.method_version.empty() || !estimate.vehicle_id.empty()
                || !estimate.vehicle_configuration_version.empty()
                || !estimate.equipment_profile_ids.empty()) {
                assignmentFindings.push_back(QStringLiteral("MT01_UNRESOLVED_OPERATION_ESTIMATE_HAS_VALUES"));
            }
        } else if (estimate.state == "VALIDATED") {
            const bool durationValid = estimate.duration_s && std::isfinite(*estimate.duration_s)
                && *estimate.duration_s >= 0.0;
            const bool energyValid = estimate.energy_wh && std::isfinite(*estimate.energy_wh)
                && *estimate.energy_wh >= 0.0;
            const bool provenanceValid = !estimate.source_id.empty()
                && !estimate.source_version.empty() && !estimate.method_version.empty()
                && !estimate.vehicle_id.empty() && !estimate.vehicle_configuration_version.empty()
                && (estimate.source_type == "VEHICLE_MODEL"
                    || estimate.source_type == "EQUIPMENT_MODEL"
                    || estimate.source_type == "MEASURED_PROFILE"
                    || estimate.source_type == "OPERATOR_APPROVED");
            if (!durationValid || !energyValid || !provenanceValid) {
                assignmentFindings.push_back(QStringLiteral("MT01_VALIDATED_OPERATION_ESTIMATE_PROVENANCE_REQUIRED"));
            } else if (mission.vehicle_assignments.size() != 1
                       || estimate.vehicle_id != mission.vehicle_assignments.front().vehicle_id
                       || estimate.vehicle_configuration_version
                            != mission.vehicle_assignments.front().configuration_version) {
                assignmentFindings.push_back(QStringLiteral("MT01_OPERATION_ESTIMATE_VEHICLE_CONFIGURATION_MISMATCH"));
            } else if (estimate.source_type == "EQUIPMENT_MODEL"
                       && (!assignedCamera
                           || std::find(estimate.equipment_profile_ids.begin(),
                                        estimate.equipment_profile_ids.end(),
                                        assignedCamera->equipment_profile_id)
                                == estimate.equipment_profile_ids.end())) {
                assignmentFindings.push_back(QStringLiteral("MT01_OPERATION_ESTIMATE_CAMERA_PROFILE_REQUIRED"));
            } else {
                taskExecutionEstimatesReady = true;
            }
        } else {
            assignmentFindings.push_back(QStringLiteral("MT01_OPERATION_ESTIMATE_STATE_INVALID"));
        }
    }
    if (!assignmentFindings.isEmpty())
        return blockedFindings(assignmentFindings);

    Mt01MappingRequest request;
    request.mission = std::move(mission);
    const auto number = [&parameters](const QString &key, double &value) {
        return readNumber(parameters, key, value);
    };

    if (!number(QStringLiteral("focalLengthMm"), request.camera.focal_length_mm)
        || !number(QStringLiteral("pixelPitchUm"), request.camera.pixel_pitch_um)
        || !number(QStringLiteral("targetGsdCmPx"), request.camera.target_gsd_cm_px)
        || !number(QStringLiteral("forwardOverlapPercent"), request.camera.forward_overlap_percent)
        || !number(QStringLiteral("sideOverlapPercent"), request.camera.side_overlap_percent)
        || !number(QStringLiteral("minimumAltitudeAglM"), request.minimum_altitude_agl_m)
        || !number(QStringLiteral("maximumAltitudeAglM"), request.maximum_altitude_agl_m)
        || !number(QStringLiteral("cruiseSpeedMps"), request.cruise_speed_mps)
        || !number(QStringLiteral("lineAngleDeg"), request.line_angle_deg))
        return blocked(QStringLiteral("MT01_NUMERIC_PARAMETERS_REQUIRED"));

    bool widthOk = false, heightOk = false;
    const uint imageWidth = parameters.value(QStringLiteral("imageWidthPx")).toUInt(&widthOk);
    const uint imageHeight = parameters.value(QStringLiteral("imageHeightPx")).toUInt(&heightOk);
    if (!widthOk || !heightOk || imageWidth == 0 || imageHeight == 0)
        return blocked(QStringLiteral("CAMERA_IMAGE_DIMENSIONS_REQUIRED"));
    request.camera.image_width_px = imageWidth;
    request.camera.image_height_px = imageHeight;
    request.camera.camera_configuration_id =
        parameters.value(QStringLiteral("cameraConfigurationId")).toString().toStdString();
    request.camera.calibration_source_reference =
        parameters.value(QStringLiteral("calibrationSourceReference")).toString().toStdString();
    request.camera.intrinsic_calibration_verified =
        parameters.value(QStringLiteral("intrinsicCalibrationVerified")).toBool();

    QStringList notamErrors;
    if (!parseNotamSnapshot(parameters, request, notamErrors))
        return blockedFindings(notamErrors);

    const QJsonDocument exclusionsDocument = QJsonDocument::fromJson(
        parameters.value(QStringLiteral("exclusionAreasJson")).toString().toUtf8());
    if (!parameters.value(QStringLiteral("exclusionAreasJson")).toString().isEmpty()) {
        if (!exclusionsDocument.isArray())
            return blocked(QStringLiteral("EXCLUSION_AREAS_JSON_INVALID"));
        for (const QJsonValue &polygonValue : exclusionsDocument.array()) {
            if (!polygonValue.isArray())
                return blocked(QStringLiteral("EXCLUSION_POLYGON_INVALID"));
            std::vector<GeoPoint> polygon;
            for (const QJsonValue &pointValue : polygonValue.toArray()) {
                if (!pointValue.isArray() || pointValue.toArray().size() < 2
                    || !pointValue.toArray().at(0).isDouble()
                    || !pointValue.toArray().at(1).isDouble())
                    return blocked(QStringLiteral("EXCLUSION_COORDINATE_INVALID"));
                polygon.push_back({pointValue.toArray().at(1).toDouble(),
                                   pointValue.toArray().at(0).toDouble()});
            }
            request.exclusion_areas.push_back(std::move(polygon));
        }
    }

    // Bind motion, safety and route lineage to explicit validated inputs from
    // the selected aircraft configuration. This bridge never synthesizes defaults.
    const auto textValue = [&parameters](const QString &key) {
        return parameters.value(key).toString().trimmed();
    };
    const auto requiredString = [&textValue](const QString &key, std::string &target) {
        const QString value = textValue(key);
        if (value.isEmpty()) return false;
        target = value.toStdString();
        return true;
    };
    const auto requiredOptional = [&parameters](const QString &key, std::optional<double> &target) {
        bool ok = false;
        const double value = parameters.value(key).toDouble(&ok);
        if (!parameters.contains(key) || !ok || !std::isfinite(value)) return false;
        target = value;
        return true;
    };
    QStringList profileErrors;
    auto &motion = request.vehicle_motion_model;
    if (!requiredString(QStringLiteral("vehicleMotionModelId"), motion.model_id)
        || !requiredString(QStringLiteral("vehicleMotionModelVersion"), motion.model_version)
        || !requiredString(QStringLiteral("vehicleMotionConfigurationId"), motion.configuration_id)
        || !requiredString(QStringLiteral("vehicleMotionSourceReference"), motion.source_reference)
        || !requiredString(QStringLiteral("vehicleMotionSpeedReference"), motion.speed_reference)
        || !number(QStringLiteral("vehicleMotionReferenceSpeedMps"), motion.reference_speed_mps)
        || !number(QStringLiteral("vehicleMotionMinimumSpeedMps"), motion.minimum_speed_mps)
        || !number(QStringLiteral("vehicleMotionMaximumSpeedMps"), motion.maximum_speed_mps)
        || !number(QStringLiteral("vehicleMotionBankAngleRad"), motion.bank_angle_rad)
        || !number(QStringLiteral("vehicleMotionLateralAccelerationMps2"), motion.lateral_acceleration_mps2))
        profileErrors.push_back(QStringLiteral("MT01_VALIDATED_VEHICLE_MOTION_PROFILE_REQUIRED"));
    const QString motionClass = textValue(QStringLiteral("vehicleMotionClass"));
    if (motionClass == QLatin1String("FIXED_WING")) motion.motion_class = MotionClass::FixedWing;
    else if (motionClass == QLatin1String("MULTIROTOR")) motion.motion_class = MotionClass::Multirotor;
    else profileErrors.push_back(QStringLiteral("MT01_VEHICLE_MOTION_CLASS_INVALID"));
    const QString motionEvidence = textValue(QStringLiteral("vehicleMotionEvidence"));
    if (motionEvidence == QLatin1String("OFFICIAL_VALIDATED")) motion.evidence = MotionEvidence::OfficialValidated;
    else if (motionEvidence == QLatin1String("FLIGHT_TEST_VALIDATED")) motion.evidence = MotionEvidence::FlightTestValidated;
    else profileErrors.push_back(QStringLiteral("MT01_VEHICLE_MOTION_EVIDENCE_NOT_VALIDATED"));

    const auto &assigned = request.mission.vehicle_assignments.front();
    if (motion.model_id != assigned.vehicle_id
        || motion.configuration_id != assigned.configuration_id
        || motion.evidence == MotionEvidence::Unknown
        || motion.evidence == MotionEvidence::DerivedScenario)
        profileErrors.push_back(QStringLiteral("MT01_VEHICLE_MOTION_PROFILE_ASSIGNMENT_MISMATCH"));

    auto &envelope = request.flight_safety_envelope;
    if (!requiredString(QStringLiteral("flightSafetyVehicleModelId"), envelope.vehicle_model_id)
        || !requiredString(QStringLiteral("flightSafetyModelVersion"), envelope.model_version)
        || !requiredString(QStringLiteral("flightSafetyConfigurationId"), envelope.configuration_id)
        || !requiredString(QStringLiteral("flightSafetySourceReference"), envelope.source_reference)
        || !requiredString(QStringLiteral("flightSafetySpeedReference"), envelope.speed_reference)
        || !requiredString(QStringLiteral("flightSafetyMinimumSpeedBasis"), envelope.minimum_speed_basis))
        profileErrors.push_back(QStringLiteral("MT01_VALIDATED_FLIGHT_SAFETY_ENVELOPE_REQUIRED"));
    const QString safetyClass = textValue(QStringLiteral("flightSafetyMotionClass"));
    if (safetyClass == QLatin1String("FIXED_WING")) envelope.motion_class = MotionClass::FixedWing;
    else if (safetyClass == QLatin1String("MULTIROTOR")) envelope.motion_class = MotionClass::Multirotor;
    else profileErrors.push_back(QStringLiteral("MT01_FLIGHT_SAFETY_MOTION_CLASS_INVALID"));
    const QString safetyEvidence = textValue(QStringLiteral("flightSafetyEvidence"));
    if (safetyEvidence == QLatin1String("OFFICIAL_VALIDATED"))
        envelope.evidence = FlightSafetyEvidence::OfficialValidated;
    else if (safetyEvidence == QLatin1String("FLIGHT_TEST_VALIDATED"))
        envelope.evidence = FlightSafetyEvidence::FlightTestValidated;
    else profileErrors.push_back(QStringLiteral("MT01_FLIGHT_SAFETY_EVIDENCE_NOT_VALIDATED"));
    if (!requiredOptional(QStringLiteral("flightSafetyMinimumSpeedMps"), envelope.minimum_speed_mps)
        || !requiredOptional(QStringLiteral("flightSafetyMaximumSpeedMps"), envelope.maximum_speed_mps))
        profileErrors.push_back(QStringLiteral("MT01_FLIGHT_SAFETY_SPEED_LIMITS_REQUIRED"));
    if (envelope.motion_class == MotionClass::FixedWing
        && !requiredOptional(QStringLiteral("flightSafetyMaximumBankAngleRad"), envelope.maximum_bank_angle_rad))
        profileErrors.push_back(QStringLiteral("MT01_FLIGHT_SAFETY_MAXIMUM_BANK_LIMIT_REQUIRED"));
    if (envelope.motion_class == MotionClass::Multirotor
        && !requiredOptional(QStringLiteral("flightSafetyMaximumLateralAccelerationMps2"), envelope.maximum_lateral_acceleration_mps2))
        profileErrors.push_back(QStringLiteral("MT01_FLIGHT_SAFETY_LATERAL_ACCELERATION_LIMIT_REQUIRED"));
    if (envelope.vehicle_model_id != motion.model_id
        || envelope.model_version != motion.model_version
        || envelope.configuration_id != motion.configuration_id
        || envelope.motion_class != motion.motion_class
        || envelope.speed_reference != motion.speed_reference)
        profileErrors.push_back(QStringLiteral("MT01_FLIGHT_SAFETY_ENVELOPE_BINDING_MISMATCH"));

    auto &metadata = request.route_candidate_metadata;
    if (!requiredString(QStringLiteral("routeId"), metadata.route_id)
        || !requiredString(QStringLiteral("routeVersion"), metadata.route_version)
        || !requiredString(QStringLiteral("routeGeneratorId"), metadata.generator_id)
        || !requiredString(QStringLiteral("routeGeneratorVersion"), metadata.generator_version)
        || !requiredString(QStringLiteral("calculationInputVersion"), metadata.calculation_input_version)
        || !requiredString(QStringLiteral("terrainSnapshotId"), metadata.environment.terrain_snapshot_id))
        profileErrors.push_back(QStringLiteral("MT01_ROUTE_LINEAGE_AND_TERRAIN_SNAPSHOT_REQUIRED"));
    metadata.environment.airspace_snapshot_id = request.notam_snapshot.snapshot_id;
    metadata.constraints.minimum_altitude_m = request.minimum_altitude_agl_m;
    metadata.constraints.maximum_altitude_m = request.maximum_altitude_agl_m;
    metadata.constraints.avoid_restricted_areas = true;
    if (!number(QStringLiteral("minimumTerrainClearanceM"), metadata.constraints.minimum_clearance_m)
        || metadata.constraints.minimum_clearance_m <= 0.0)
        profileErrors.push_back(QStringLiteral("MT01_MINIMUM_TERRAIN_CLEARANCE_REQUIRED"));

    const Mt01MappingPipelineResult calculated = Mt01MappingPipeline::calculate(request);
    QStringList findings = profileErrors;
    for (const std::string &finding : calculated.findings)
        findings.push_back(QString::fromStdString(finding));
    const bool routeCandidateExportable = calculated.route_candidate_available
        && profileErrors.isEmpty()
        && !metadata.environment.terrain_snapshot_id.empty()
        && !metadata.environment.airspace_snapshot_id.empty();
    const bool taskExecutionEstimatesAppliedToTrajectory = false;
    if (calculated.coverage_candidate_available && !taskExecutionEstimatesReady) {
        findings.push_back(QStringLiteral("MT01_OPERATION_EXECUTION_ESTIMATE_UNRESOLVED: coverage geometry is available, but operation time/energy is not yet applied to trajectory assessment."));
    } else if (calculated.coverage_candidate_available
               && taskExecutionEstimatesReady
               && !taskExecutionEstimatesAppliedToTrajectory) {
        findings.push_back(QStringLiteral("MT01_OPERATION_EXECUTION_ESTIMATE_NOT_APPLIED_TO_TRAJECTORY: the aggregate operation estimate is validated, but it has not been mapped to capture/route execution events and is excluded from flight-time and energy assessment."));
    }

    QVariantMap metrics{
        {QStringLiteral("targetGsdCmPx"), calculated.coverage.metrics.target_gsd_cm_px},
        {QStringLiteral("calculatedAltitudeAglM"), calculated.coverage.metrics.calculated_altitude_agl_m},
        {QStringLiteral("footprintWidthM"), calculated.coverage.metrics.footprint_width_m},
        {QStringLiteral("footprintHeightM"), calculated.coverage.metrics.footprint_height_m},
        {QStringLiteral("lineSpacingM"), calculated.coverage.metrics.line_spacing_m},
        {QStringLiteral("forwardTriggerSpacingM"), calculated.coverage.metrics.forward_trigger_spacing_m},
        {QStringLiteral("coverageLegLengthM"), calculated.coverage.metrics.estimated_coverage_leg_length_m},
        {QStringLiteral("coverageLegTimeS"), calculated.coverage.metrics.estimated_coverage_leg_time_s},
        {QStringLiteral("coverageLineCount"), static_cast<qulonglong>(calculated.coverage.metrics.coverage_line_count)},
        {QStringLiteral("coverageSegmentCount"), static_cast<qulonglong>(calculated.coverage.metrics.coverage_segment_count)},
        {QStringLiteral("exclusionIntersectionCount"), static_cast<qulonglong>(calculated.coverage.metrics.exclusion_intersection_count)},
        {QStringLiteral("notamApplicableComponentCount"), static_cast<qulonglong>(calculated.notam_geometry.applicable_component_ids.size())}
    };

    QVariantList waypoints;
    for (const GeoPoint &point : calculated.coverage.waypoints)
        waypoints.push_back(QVariantMap{
            {QStringLiteral("latitudeDeg"), point.latitude_deg},
            {QStringLiteral("longitudeDeg"), point.longitude_deg}
        });

    QVariantList capturePoints;
    for (const CoverageCapturePoint &point : calculated.coverage.capture_points)
        capturePoints.push_back(QVariantMap{
            {QStringLiteral("latitudeDeg"), point.position.latitude_deg},
            {QStringLiteral("longitudeDeg"), point.position.longitude_deg},
            {QStringLiteral("coverageSegmentIndex"), static_cast<qulonglong>(point.coverage_segment_index)},
            {QStringLiteral("distanceFromSegmentStartM"), point.distance_from_segment_start_m}
        });

    // Mission has already been moved into the mapping request. Read identity
    // from the request-owned canonical mission, not the moved-from local object.
    // The lifecycle record remains a derived projection of this identity.
    QVariantList lifecycleTemplateIds;
    for (const auto &templateId : request.mission.selected_template_ids)
        lifecycleTemplateIds.push_back(QString::fromStdString(templateId));
    const QVariantMap lifecycleIdentity{
        {QStringLiteral("missionId"), QString::fromStdString(request.mission.identity.mission_id)},
        {QStringLiteral("missionVersion"), QString::fromStdString(request.mission.identity.mission_version)},
        {QStringLiteral("templateIds"), lifecycleTemplateIds}
    };

    return {
        {QStringLiteral("lifecycleIdentity"), lifecycleIdentity},
        {QStringLiteral("status"), calculated.coverage_candidate_available
             ? QStringLiteral("COVERAGE_CANDIDATE_AVAILABLE") : QStringLiteral("NEEDS_INPUT")},
        {QStringLiteral("coverageCandidateAvailable"), calculated.coverage_candidate_available},
        {QStringLiteral("flightReleaseEligible"), false},
        {QStringLiteral("taskExecutionEstimatesReady"), taskExecutionEstimatesReady},
        {QStringLiteral("taskExecutionEstimatesAppliedToTrajectory"), taskExecutionEstimatesAppliedToTrajectory},
        {QStringLiteral("coverageValid"), calculated.coverage.valid},
        {QStringLiteral("coverageStatus"), QString::fromStdString(calculated.coverage.status)},
        {QStringLiteral("kinematicGeometryAvailable"), calculated.kinematic_geometry_available},
        {QStringLiteral("segmentDynamicsValidated"), calculated.segment_dynamics_validated},
        {QStringLiteral("routeCandidateAvailable"), routeCandidateExportable},
        {QStringLiteral("routeCandidate"), routeCandidateExportable
             ? QVariant(serializeRouteCandidate(calculated.route_candidate.route)) : QVariant()},
        {QStringLiteral("cameraCalibrationVerified"), calculated.coverage.intrinsic_calibration_verified},
        {QStringLiteral("findings"), findings},
        {QStringLiteral("metrics"), metrics},
        {QStringLiteral("waypoints"), waypoints},
        {QStringLiteral("capturePoints"), capturePoints},
        {QStringLiteral("notamSnapshotId"), QString::fromStdString(calculated.notam_geometry.snapshot_id)},
        {QStringLiteral("notamApplicableIds"), [&calculated]() {
            QStringList values;
            for (const auto &id : calculated.notam_geometry.applicable_notam_ids)
                values.push_back(QString::fromStdString(id));
            return values;
        }()},
        {QStringLiteral("notamComponentIds"), [&calculated]() {
            QStringList values;
            for (const auto &id : calculated.notam_geometry.applicable_component_ids)
                values.push_back(QString::fromStdString(id));
            return values;
        }()}
    };
}

QVariantMap Mt01MappingBridge::normalizeNotamSnapshot(const QVariantMap &parameters) const
{
    Mt01MappingRequest request;
    QStringList errors;
    if (!parseNotamSnapshot(parameters, request, errors))
        return {{QStringLiteral("valid"), false}, {QStringLiteral("errors"), errors}};

    QVariantList zones;
    for (const NotamZone &zone : request.notam_snapshot.zones) {
        QString datum = QStringLiteral("UNKNOWN");
        switch (zone.altitude_reference) {
        case NotamAltitudeReference::AGL: datum = QStringLiteral("AGL"); break;
        case NotamAltitudeReference::AMSL: datum = QStringLiteral("AMSL"); break;
        case NotamAltitudeReference::FlightLevel: datum = QStringLiteral("FL"); break;
        case NotamAltitudeReference::Unknown: break;
        }
        QVariantList polygon;
        for (const GeoPoint &point : zone.polygon)
            polygon.push_back(QVariantMap{
                {QStringLiteral("latitudeDeg"), point.latitude_deg},
                {QStringLiteral("longitudeDeg"), point.longitude_deg}
            });
        zones.push_back(QVariantMap{
            {QStringLiteral("notamId"), QString::fromStdString(zone.notam_id)},
            {QStringLiteral("componentId"), QString::fromStdString(zone.component_id)},
            {QStringLiteral("geometryType"), QStringLiteral("POLYGON")},
            {QStringLiteral("polygon"), polygon},
            {QStringLiteral("lowerAltitudeM"), zone.lower_altitude_m},
            {QStringLiteral("upperAltitudeM"), zone.upper_altitude_m},
            {QStringLiteral("altitudeReference"), datum},
            {QStringLiteral("validFromEpochS"), static_cast<qlonglong>(zone.valid_from_epoch_s)},
            {QStringLiteral("validToEpochS"), static_cast<qlonglong>(zone.valid_to_epoch_s)}
        });
    }
    return {
        {QStringLiteral("valid"), true},
        {QStringLiteral("snapshotId"), QString::fromStdString(request.notam_snapshot.snapshot_id)},
        {QStringLiteral("sourceId"), QString::fromStdString(request.notam_snapshot.source_id)},
        {QStringLiteral("sourceVersion"), QString::fromStdString(request.notam_snapshot.source_version)},
        {QStringLiteral("evaluationEpochS"), static_cast<qlonglong>(request.notam_snapshot.evaluation_epoch_s)},
        {QStringLiteral("coverageCenterLatitudeDeg"), request.notam_snapshot.coverage_center.latitude_deg},
        {QStringLiteral("coverageCenterLongitudeDeg"), request.notam_snapshot.coverage_center.longitude_deg},
        {QStringLiteral("coverageRadiusM"), request.notam_snapshot.coverage_radius_m},
        {QStringLiteral("plannedMissionStartEpochS"), static_cast<qlonglong>(request.planned_mission_start_epoch_s)},
        {QStringLiteral("plannedMissionEndEpochS"), static_cast<qlonglong>(request.planned_mission_end_epoch_s)},
        {QStringLiteral("safetyBufferM"), request.notam_safety_buffer_m},
        {QStringLiteral("zones"), zones}
    };
}
