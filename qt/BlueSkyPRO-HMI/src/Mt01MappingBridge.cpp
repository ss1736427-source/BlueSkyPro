#include "Mt01MappingBridge.h"

#include "04_SOFTWARE/PLANNING/mt01_mapping_pipeline.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QVariantList>

#include <cmath>
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

    const Mt01MappingPipelineResult calculated = Mt01MappingPipeline::calculate(request);
    QStringList findings;
    for (const std::string &finding : calculated.findings)
        findings.push_back(QString::fromStdString(finding));

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
        {QStringLiteral("exclusionIntersectionCount"), static_cast<qulonglong>(calculated.coverage.metrics.exclusion_intersection_count)}
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

    return {
        {QStringLiteral("status"), calculated.coverage_candidate_available
             ? QStringLiteral("COVERAGE_CANDIDATE_AVAILABLE") : QStringLiteral("NEEDS_INPUT")},
        {QStringLiteral("coverageCandidateAvailable"), calculated.coverage_candidate_available},
        {QStringLiteral("flightReleaseEligible"), false},
        {QStringLiteral("coverageValid"), calculated.coverage.valid},
        {QStringLiteral("coverageStatus"), QString::fromStdString(calculated.coverage.status)},
        {QStringLiteral("cameraCalibrationVerified"), calculated.coverage.intrinsic_calibration_verified},
        {QStringLiteral("findings"), findings},
        {QStringLiteral("metrics"), metrics},
        {QStringLiteral("waypoints"), waypoints},
        {QStringLiteral("capturePoints"), capturePoints}
    };
}
