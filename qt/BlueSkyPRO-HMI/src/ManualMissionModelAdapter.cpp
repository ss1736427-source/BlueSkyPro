#include "ManualMissionModelAdapter.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QVariantList>
#include <QJsonValue>
#include <QSet>

#include "04_SOFTWARE/PLANNING/model/mission_model.hpp"
#include "04_SOFTWARE/PLANNING/model/mission_model_validation.hpp"

#include <cmath>
#include <string>
#include <utility>

using bluesky::planning::Mission;
using bluesky::planning::MissionGeometryObject;
using bluesky::planning::MissionGeometryVertex;
using bluesky::planning::MissionOrigin;
using bluesky::planning::validateMissionModelStructure;

namespace
{
struct SerializedOperations {
    QJsonArray json;
    QStringList errors;
    bool supplied{false};
    bool allEstimatesValidated{false};
};

SerializedOperations serializeOperations(const QVariant &value,
                                          const QStringList &selectedTemplateIds,
                                          const QJsonArray &geometryObjects,
                                          const QVariantList &vehicles,
                                          const QVariantList &equipment)
{
    SerializedOperations result;
    if (!value.isValid() || value.isNull())
        return result;
    result.supplied = true;
    if (!value.canConvert<QVariantList>()) {
        result.errors.push_back(QStringLiteral("MISSION_OPERATIONS_INVALID"));
        return result;
    }

    const QVariantList operations = value.toList();
    result.allEstimatesValidated = !operations.isEmpty();
    QSet<QString> operationIds;
    QSet<QString> assignedEquipmentIds;
    QSet<QString> assignedEquipmentProfiles;
    for (const QVariant &entry : equipment) {
        const QVariantMap item = entry.toMap();
        assignedEquipmentIds.insert(item.value(QStringLiteral("equipmentId")).toString());
        assignedEquipmentProfiles.insert(item.value(QStringLiteral("equipmentProfileId")).toString());
    }

    for (qsizetype index = 0; index < operations.size(); ++index) {
        if (!operations.at(index).canConvert<QVariantMap>()) {
            result.errors.push_back(QStringLiteral("MISSION_OPERATION_INVALID:%1").arg(index));
            result.allEstimatesValidated = false;
            continue;
        }
        const QVariantMap operation = operations.at(index).toMap();
        const QString operationId = operation.value(QStringLiteral("operationId")).toString().trimmed();
        const QString templateId = operation.value(QStringLiteral("templateId")).toString().trimmed();
        const QString operationType = operation.value(QStringLiteral("operationType")).toString().trimmed();
        const QString geometryId = operation.value(QStringLiteral("geometryId")).toString().trimmed();
        if (operationId.isEmpty() || templateId.isEmpty() || operationType.isEmpty() || geometryId.isEmpty())
            result.errors.push_back(QStringLiteral("MISSION_OPERATION_IDENTITY_INCOMPLETE:%1").arg(index));
        if (operationIds.contains(operationId))
            result.errors.push_back(QStringLiteral("MISSION_OPERATION_ID_DUPLICATE:%1").arg(operationId));
        operationIds.insert(operationId);
        if (!selectedTemplateIds.contains(templateId))
            result.errors.push_back(QStringLiteral("MISSION_OPERATION_TEMPLATE_NOT_SELECTED:%1").arg(operationId));

        bool geometryBound = false;
        for (const QJsonValue &geometryValue : geometryObjects) {
            const QJsonObject geometry = geometryValue.toObject();
            if (geometry.value(QStringLiteral("geometryId")).toString() != geometryId)
                continue;
            const QJsonArray geometryTemplates = geometry.value(QStringLiteral("templateIds")).toArray();
            for (const QJsonValue &geometryTemplate : geometryTemplates)
                geometryBound = geometryBound || geometryTemplate.toString() == templateId;
        }
        if (!geometryBound)
            result.errors.push_back(QStringLiteral("MISSION_OPERATION_GEOMETRY_TEMPLATE_MISMATCH:%1").arg(operationId));

        const QVariant equipmentIdsValue = operation.value(QStringLiteral("equipmentIds"));
        if (!equipmentIdsValue.canConvert<QVariantList>()) {
            result.errors.push_back(QStringLiteral("MISSION_OPERATION_EQUIPMENT_IDS_INVALID:%1").arg(operationId));
        } else {
            for (const QVariant &equipmentIdValue : equipmentIdsValue.toList()) {
                const QString equipmentId = equipmentIdValue.toString();
                if (equipmentId.isEmpty() || !assignedEquipmentIds.contains(equipmentId))
                    result.errors.push_back(QStringLiteral("MISSION_OPERATION_EQUIPMENT_NOT_ASSIGNED:%1:%2")
                                                .arg(operationId, equipmentId));
            }
        }

        const QVariantMap estimate = operation.value(QStringLiteral("executionEstimate")).toMap();
        const QString state = estimate.value(QStringLiteral("state")).toString();
        if (state == QStringLiteral("UNRESOLVED")) {
            result.allEstimatesValidated = false;
            const QStringList forbidden = {
                QStringLiteral("durationS"), QStringLiteral("energyWh"), QStringLiteral("sourceType"),
                QStringLiteral("sourceId"), QStringLiteral("sourceVersion"), QStringLiteral("methodVersion"),
                QStringLiteral("vehicleId"), QStringLiteral("vehicleConfigurationVersion")
            };
            for (const QString &key : forbidden) {
                if (estimate.contains(key) && !estimate.value(key).toString().isEmpty())
                    result.errors.push_back(QStringLiteral("UNRESOLVED_OPERATION_ESTIMATE_HAS_VALUES:%1").arg(operationId));
            }
        } else if (state == QStringLiteral("VALIDATED")) {
            bool durationOk = false, energyOk = false;
            const double duration = estimate.value(QStringLiteral("durationS")).toDouble(&durationOk);
            const double energyWh = estimate.value(QStringLiteral("energyWh")).toDouble(&energyOk);
            const QString sourceType = estimate.value(QStringLiteral("sourceType")).toString();
            const QString sourceId = estimate.value(QStringLiteral("sourceId")).toString();
            const QString sourceVersion = estimate.value(QStringLiteral("sourceVersion")).toString();
            const QString methodVersion = estimate.value(QStringLiteral("methodVersion")).toString();
            const QString vehicleId = estimate.value(QStringLiteral("vehicleId")).toString();
            const QString vehicleConfigurationVersion = estimate.value(QStringLiteral("vehicleConfigurationVersion")).toString();
            const QStringList sourceTypes = {"VEHICLE_MODEL", "EQUIPMENT_MODEL", "MEASURED_PROFILE", "OPERATOR_APPROVED"};
            if (!durationOk || !energyOk || !std::isfinite(duration) || !std::isfinite(energyWh)
                || duration < 0.0 || energyWh < 0.0 || !sourceTypes.contains(sourceType)
                || sourceId.isEmpty() || sourceVersion.isEmpty() || methodVersion.isEmpty()
                || vehicleId.isEmpty() || vehicleConfigurationVersion.isEmpty()) {
                result.errors.push_back(QStringLiteral("VALIDATED_OPERATION_ESTIMATE_PROVENANCE_REQUIRED:%1").arg(operationId));
            }
            bool vehicleConfigurationFound = false;
            for (const QVariant &vehicleValue : vehicles) {
                const QVariantMap vehicle = vehicleValue.toMap();
                if (vehicle.value(QStringLiteral("vehicleId")).toString() == vehicleId
                    && vehicle.value(QStringLiteral("configurationVersion")).toString() == vehicleConfigurationVersion
                    && vehicle.value(QStringLiteral("approvalState")).toString() == QStringLiteral("APPROVED")
                    && (vehicle.value(QStringLiteral("validationState")).toString() == QStringLiteral("VALIDATED")
                        || vehicle.value(QStringLiteral("validationState")).toString() == QStringLiteral("APPROVED")))
                    vehicleConfigurationFound = true;
            }
            if (!vehicleConfigurationFound)
                result.errors.push_back(QStringLiteral("MISSION_OPERATION_VEHICLE_CONFIGURATION_MISMATCH:%1").arg(operationId));

            const QVariant profilesValue = estimate.value(QStringLiteral("equipmentProfileIds"));
            if (profilesValue.isValid() && profilesValue.canConvert<QVariantList>()) {
                for (const QVariant &profileValue : profilesValue.toList()) {
                    if (!assignedEquipmentProfiles.contains(profileValue.toString()))
                        result.errors.push_back(QStringLiteral("MISSION_OPERATION_EQUIPMENT_PROFILE_NOT_ASSIGNED:%1:%2")
                                                    .arg(operationId, profileValue.toString()));
                }
            }
            if (sourceType == QStringLiteral("EQUIPMENT_MODEL")
                && (!profilesValue.canConvert<QVariantList>() || profilesValue.toList().isEmpty()))
                result.errors.push_back(QStringLiteral("MISSION_OPERATION_EQUIPMENT_PROFILE_REQUIRED:%1").arg(operationId));
        } else {
            result.errors.push_back(QStringLiteral("MISSION_OPERATION_ESTIMATE_STATE_INVALID:%1").arg(operationId));
            result.allEstimatesValidated = false;
        }
        result.json.append(QJsonObject::fromVariantMap(operation));
    }
    if (!result.errors.isEmpty())
        result.allEstimatesValidated = false;
    return result;
}

QStringList strings(const std::vector<std::string> &values)
{
    QStringList result;
    for (const auto &value : values)
        result.push_back(QString::fromStdString(value));
    return result;
}
}

QVariantMap ManualMissionModelAdapter::buildMission(const QString &geometryJson,
                                                     const QStringList &selectedTemplateIds,
                                                     const QString &missionId,
                                                     int missionVersion,
                                                     bool compositionComplete) const
{
    QVariantMap result;
    QStringList errors;

    if (missionId.trimmed().isEmpty())
        errors.push_back(QStringLiteral("MISSION_ID_REQUIRED"));
    if (missionVersion < 1)
        errors.push_back(QStringLiteral("MISSION_VERSION_INVALID"));
    if (!compositionComplete)
        errors.push_back(QStringLiteral("MANUAL_GEOMETRY_COMPOSITION_INCOMPLETE"));

    const QJsonDocument document = QJsonDocument::fromJson(geometryJson.toUtf8());
    if (!document.isObject()
        || !document.object().value(QStringLiteral("objects")).isArray()) {
        errors.push_back(QStringLiteral("MANUAL_GEOMETRY_JSON_INVALID"));
        result.insert(QStringLiteral("valid"), false);
        result.insert(QStringLiteral("errors"), errors);
        return result;
    }

    Mission mission;
    mission.identity.mission_id = missionId.toStdString();
    mission.identity.mission_version = std::to_string(missionVersion);
    mission.identity.mission_type = "manual";
    mission.identity.operational_status = "draft";
    mission.mission_mode = "MANUAL";
    for (const QString &id : selectedTemplateIds)
        mission.selected_template_ids.push_back(id.toStdString());

    const QJsonObject rootObject = document.object();
    const QJsonValue originValue = rootObject.value(QStringLiteral("origin"));
    if (originValue.isUndefined() || originValue.isNull())
        errors.push_back(QStringLiteral("MISSION_ORIGIN_REQUIRED"));
    if (!originValue.isUndefined() && !originValue.isNull()) {
        if (!originValue.isObject()) {
            errors.push_back(QStringLiteral("MISSION_ORIGIN_INVALID"));
        } else {
            const QJsonObject originObject = originValue.toObject();
            MissionOrigin origin;
            const QJsonValue longitude = originObject.value(QStringLiteral("longitudeDeg"));
            const QJsonValue latitude = originObject.value(QStringLiteral("latitudeDeg"));
            const QJsonValue crs = originObject.value(QStringLiteral("coordinateReferenceSystem"));
            const QJsonValue kind = originObject.value(QStringLiteral("originKind"));
            if (!longitude.isDouble() || !latitude.isDouble()
                || !std::isfinite(longitude.toDouble()) || !std::isfinite(latitude.toDouble())
                || longitude.toDouble() < -180.0 || longitude.toDouble() > 180.0
                || latitude.toDouble() < -90.0 || latitude.toDouble() > 90.0) {
                errors.push_back(QStringLiteral("MISSION_ORIGIN_COORDINATE_INVALID"));
            } else {
                origin.longitude_deg = longitude.toDouble();
                origin.latitude_deg = latitude.toDouble();
            }
            if (!crs.isString() || crs.toString().trimmed().isEmpty()) {
                errors.push_back(QStringLiteral("MISSION_ORIGIN_CRS_REQUIRED"));
            } else if (crs.toString() != QStringLiteral("EPSG:4326")) {
                errors.push_back(QStringLiteral("MISSION_ORIGIN_CRS_UNSUPPORTED"));
            } else {
                origin.coordinate_reference_system = crs.toString().toStdString();
            }
            if (!kind.isString() || kind.toString() != QStringLiteral("PLANNED_TAKEOFF")) {
                errors.push_back(QStringLiteral("MISSION_ORIGIN_KIND_INVALID"));
            } else {
                origin.origin_kind = kind.toString().toStdString();
            }
            const QJsonValue accuracy = originObject.value(QStringLiteral("horizontalAccuracyM"));
            if (!accuracy.isUndefined() && !accuracy.isNull()) {
                if (!accuracy.isDouble() || !std::isfinite(accuracy.toDouble())
                    || accuracy.toDouble() <= 0.0) {
                    errors.push_back(QStringLiteral("MISSION_ORIGIN_ACCURACY_INVALID"));
                } else {
                    origin.horizontal_accuracy_m = accuracy.toDouble();
                }
            }
            const QJsonValue bearing = originObject.value(QStringLiteral("trueNorthBearingDeg"));
            if (!bearing.isUndefined() && !bearing.isNull()) {
                if (!bearing.isDouble() || !std::isfinite(bearing.toDouble())
                    || bearing.toDouble() < 0.0 || bearing.toDouble() >= 360.0) {
                    errors.push_back(QStringLiteral("MISSION_ORIGIN_BEARING_INVALID"));
                } else {
                    origin.true_north_bearing_deg = bearing.toDouble();
                }
            }
            mission.origin = origin;
        }
    }

    const QJsonArray objects = document.object().value(QStringLiteral("objects")).toArray();
    for (qsizetype i = 0; i < objects.size(); ++i) {
        if (!objects.at(i).isObject()) {
            errors.push_back(QStringLiteral("GEOMETRY_OBJECT_INVALID:%1").arg(i));
            continue;
        }

        const QJsonObject object = objects.at(i).toObject();
        MissionGeometryObject geometry;
        geometry.geometry_id = object.value(QStringLiteral("geometryId")).toString().toStdString();
        geometry.geometry_type = object.value(QStringLiteral("geometryType")).toString().toStdString();
        geometry.coordinate_reference_system =
            object.value(QStringLiteral("coordinateReferenceSystem")).toString().toStdString();
        geometry.version = object.value(QStringLiteral("version")).toInt(1);

        const QJsonValue templateValue = object.value(QStringLiteral("templateIds"));
        if (!templateValue.isArray()) {
            errors.push_back(QStringLiteral("GEOMETRY_TEMPLATE_IDS_INVALID:%1").arg(i));
        } else {
            for (const QJsonValue &id : templateValue.toArray()) {
                if (!id.isString()) {
                    errors.push_back(QStringLiteral("GEOMETRY_TEMPLATE_ID_INVALID:%1").arg(i));
                    continue;
                }
                geometry.template_ids.push_back(id.toString().toStdString());
            }
        }

        const QJsonValue coordinatesValue = object.value(QStringLiteral("coordinates"));
        if (!coordinatesValue.isArray()) {
            errors.push_back(QStringLiteral("GEOMETRY_COORDINATES_INVALID:%1").arg(i));
        } else {
            const QJsonArray coordinates = coordinatesValue.toArray();
            for (qsizetype pointIndex = 0; pointIndex < coordinates.size(); ++pointIndex) {
                const QJsonValue pointValue = coordinates.at(pointIndex);
                if (!pointValue.isArray() || pointValue.toArray().size() < 2) {
                    errors.push_back(QStringLiteral("GEOMETRY_COORDINATE_INVALID:%1:%2")
                                         .arg(i).arg(pointIndex));
                    continue;
                }
                const QJsonArray point = pointValue.toArray();
                if (!point.at(0).isDouble() || !point.at(1).isDouble()) {
                    errors.push_back(QStringLiteral("GEOMETRY_COORDINATE_INVALID:%1:%2")
                                         .arg(i).arg(pointIndex));
                    continue;
                }
                MissionGeometryVertex vertex;
                vertex.longitude_deg = point.at(0).toDouble();
                vertex.latitude_deg = point.at(1).toDouble();
                if (point.size() > 2) {
                    if (point.at(2).isDouble()) {
                        vertex.altitude_m = point.at(2).toDouble();
                    } else {
                        errors.push_back(QStringLiteral("GEOMETRY_ALTITUDE_INVALID:%1:%2")
                                             .arg(i).arg(pointIndex));
                    }
                }
                if (point.size() > 3) {
                    if (point.at(3).isString()
                        && !point.at(3).toString().trimmed().isEmpty()) {
                        vertex.altitude_reference = point.at(3).toString().toStdString();
                    } else {
                        errors.push_back(QStringLiteral("GEOMETRY_ALTITUDE_REFERENCE_INVALID:%1:%2")
                                             .arg(i).arg(pointIndex));
                    }
                }
                if (vertex.altitude_reference && !vertex.altitude_m) {
                    errors.push_back(QStringLiteral("GEOMETRY_ALTITUDE_REFERENCE_WITHOUT_ALTITUDE:%1:%2")
                                         .arg(i).arg(pointIndex));
                }
                if (!std::isfinite(vertex.longitude_deg) || !std::isfinite(vertex.latitude_deg))
                    errors.push_back(QStringLiteral("GEOMETRY_COORDINATE_NONFINITE:%1:%2")
                                         .arg(i).arg(pointIndex));
                geometry.coordinates.push_back(vertex);
            }
        }

        mission.geometry_objects.push_back(std::move(geometry));
    }

    const auto structuralErrors = validateMissionModelStructure(mission);
    errors.append(strings(structuralErrors));
    errors.removeDuplicates();

    QJsonArray templateArray;
    for (const auto &id : mission.selected_template_ids)
        templateArray.append(QString::fromStdString(id));

    QJsonArray geometryArray;
    for (const auto &geometry : mission.geometry_objects) {
        QJsonArray coordinateArray;
        for (const auto &point : geometry.coordinates) {
            QJsonArray coordinate;
            coordinate.append(point.longitude_deg);
            coordinate.append(point.latitude_deg);
            if (point.altitude_m) {
                coordinate.append(*point.altitude_m);
                if (point.altitude_reference)
                    coordinate.append(QString::fromStdString(*point.altitude_reference));
            }
            coordinateArray.append(coordinate);
        }
        QJsonArray geometryTemplates;
        for (const auto &id : geometry.template_ids)
            geometryTemplates.append(QString::fromStdString(id));

        QJsonObject geometryJsonObject{
            {QStringLiteral("geometryId"), QString::fromStdString(geometry.geometry_id)},
            {QStringLiteral("geometryType"), QString::fromStdString(geometry.geometry_type)},
            {QStringLiteral("coordinateReferenceSystem"), QString::fromStdString(geometry.coordinate_reference_system)},
            {QStringLiteral("coordinates"), coordinateArray},
            {QStringLiteral("templateIds"), geometryTemplates},
            {QStringLiteral("version"), geometry.version}
        };
        geometryArray.append(geometryJsonObject);
    }

    QJsonObject canonicalOrigin;
    if (mission.origin) {
        canonicalOrigin = QJsonObject{
            {QStringLiteral("longitudeDeg"), mission.origin->longitude_deg},
            {QStringLiteral("latitudeDeg"), mission.origin->latitude_deg},
            {QStringLiteral("coordinateReferenceSystem"),
             QString::fromStdString(mission.origin->coordinate_reference_system)},
            {QStringLiteral("originKind"), QString::fromStdString(mission.origin->origin_kind)}
        };
        if (mission.origin->horizontal_accuracy_m)
            canonicalOrigin.insert(QStringLiteral("horizontalAccuracyM"),
                                   *mission.origin->horizontal_accuracy_m);
        if (mission.origin->true_north_bearing_deg)
            canonicalOrigin.insert(QStringLiteral("trueNorthBearingDeg"),
                                   *mission.origin->true_north_bearing_deg);
    }

    QJsonObject canonical{
        {QStringLiteral("missionId"), missionId},
        {QStringLiteral("missionVersion"), missionVersion},
        {QStringLiteral("missionMode"), QStringLiteral("MANUAL")},
        {QStringLiteral("selectedTemplateIds"), templateArray},
        {QStringLiteral("geometryObjects"), geometryArray}
    };

    if (mission.origin)
        canonical.insert(QStringLiteral("origin"), canonicalOrigin);

    result.insert(QStringLiteral("valid"), errors.isEmpty());
    result.insert(QStringLiteral("errors"), errors);
    result.insert(QStringLiteral("missionModelJson"),
                  QString::fromUtf8(QJsonDocument(canonical).toJson(QJsonDocument::Compact)));
    result.insert(QStringLiteral("structuralValidationOnly"), true);
    result.insert(QStringLiteral("flightReleaseEligible"), false);
    return result;
}


QVariantMap ManualMissionModelAdapter::buildMissionV2(
    const QString &geometryJson,
    const QStringList &selectedTemplateIds,
    const QString &missionId,
    int missionVersion,
    bool compositionComplete,
    const QVariantMap &missionContext) const
{
    QVariantMap result = buildMission(geometryJson, selectedTemplateIds,
                                      missionId, missionVersion, compositionComplete);
    if (!result.value(QStringLiteral("valid")).toBool())
        return result;

    QStringList errors = result.value(QStringLiteral("errors")).toStringList();
    const QString taskDescription =
        missionContext.value(QStringLiteral("taskDescription")).toString().trimmed();
    const QString objective =
        missionContext.value(QStringLiteral("objective")).toString().trimmed();
    const QString taskType =
        missionContext.value(QStringLiteral("taskType")).toString().trimmed();

    if (taskDescription.isEmpty())
        errors.push_back(QStringLiteral("TASK_DESCRIPTION_REQUIRED"));
    if (objective.isEmpty())
        errors.push_back(QStringLiteral("MISSION_OBJECTIVE_REQUIRED"));
    if (taskType.isEmpty())
        errors.push_back(QStringLiteral("MISSION_TASK_TYPE_REQUIRED"));

    const QVariant vehiclesValue = missionContext.value(QStringLiteral("vehicleAssignments"));
    const QVariant equipmentValue = missionContext.value(QStringLiteral("equipmentAssignments"));
    if (!vehiclesValue.canConvert<QVariantList>())
        errors.push_back(QStringLiteral("VEHICLE_ASSIGNMENTS_INVALID"));
    if (!equipmentValue.canConvert<QVariantList>())
        errors.push_back(QStringLiteral("EQUIPMENT_ASSIGNMENTS_INVALID"));

    QJsonParseError parseError;
    const QJsonDocument baseDocument = QJsonDocument::fromJson(
        result.value(QStringLiteral("missionModelJson")).toString().toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !baseDocument.isObject()) {
        errors.push_back(QStringLiteral("MISSION_MODEL_SERIALIZATION_INVALID"));
        result.insert(QStringLiteral("valid"), false);
        result.insert(QStringLiteral("errors"), errors);
        return result;
    }

    QJsonObject canonical = baseDocument.object();
    canonical.insert(QStringLiteral("status"), QStringLiteral("DRAFT"));
    canonical.insert(QStringLiteral("task"), QJsonObject{
        {QStringLiteral("description"), taskDescription}
    });
    canonical.insert(QStringLiteral("intent"), QJsonObject{
        {QStringLiteral("objective"), objective},
        {QStringLiteral("taskType"), taskType}
    });

    const QVariantList vehicles = vehiclesValue.canConvert<QVariantList>()
        ? vehiclesValue.toList() : QVariantList{};
    const QVariantList equipment = equipmentValue.canConvert<QVariantList>()
        ? equipmentValue.toList() : QVariantList{};
    canonical.insert(QStringLiteral("vehicleAssignments"),
                     QJsonArray::fromVariantList(vehicles));
    canonical.insert(QStringLiteral("equipmentAssignments"),
                     QJsonArray::fromVariantList(equipment));

    QVariant operationsValue = missionContext.value(QStringLiteral("operations"));
    bool operationsAutoGenerated = false;
    if (!operationsValue.isValid() || operationsValue.isNull()
        || (operationsValue.canConvert<QVariantList>() && operationsValue.toList().isEmpty())) {
        QVariantList generatedOperations;
        if (selectedTemplateIds.contains(QStringLiteral("MT-01"))) {
            const QJsonArray geometryObjects = canonical.value(QStringLiteral("geometryObjects")).toArray();
            QStringList cameraEquipmentIds;
            for (const QVariant &equipmentValue : equipment) {
                const QVariantMap assignment = equipmentValue.toMap();
                const QString equipmentClass =
                    assignment.value(QStringLiteral("equipmentClass")).toString().trimmed().toUpper();
                const QString equipmentId =
                    assignment.value(QStringLiteral("equipmentId")).toString().trimmed();
                if (equipmentClass.contains(QStringLiteral("CAMERA")) && !equipmentId.isEmpty())
                    cameraEquipmentIds.push_back(equipmentId);
            }
            for (const QJsonValue &geometryValue : geometryObjects) {
                const QJsonObject geometry = geometryValue.toObject();
                if (geometry.value(QStringLiteral("geometryType")).toString()
                    != QStringLiteral("POLYGON_AOI"))
                    continue;
                bool geometryUsesMt01 = false;
                const QJsonArray geometryTemplates =
                    geometry.value(QStringLiteral("templateIds")).toArray();
                for (const QJsonValue &templateValue : geometryTemplates)
                    geometryUsesMt01 = geometryUsesMt01
                        || templateValue.toString() == QStringLiteral("MT-01");
                const QString geometryId =
                    geometry.value(QStringLiteral("geometryId")).toString().trimmed();
                if (!geometryUsesMt01 || geometryId.isEmpty())
                    continue;

                QVariantMap unresolvedEstimate{
                    {QStringLiteral("state"), QStringLiteral("UNRESOLVED")}
                };
                QVariantMap operation{
                    {QStringLiteral("operationId"),
                     QStringLiteral("OP-%1-MT-01-%2").arg(missionId, geometryId)},
                    {QStringLiteral("templateId"), QStringLiteral("MT-01")},
                    {QStringLiteral("operationType"), QStringLiteral("MAPPING_SWATH")},
                    {QStringLiteral("geometryId"), geometryId},
                    {QStringLiteral("equipmentIds"), cameraEquipmentIds},
                    {QStringLiteral("executionEstimate"), unresolvedEstimate}
                };
                generatedOperations.push_back(operation);
            }
        }
        operationsValue = generatedOperations;
        operationsAutoGenerated = !generatedOperations.isEmpty();
    }

    const SerializedOperations serializedOperations = serializeOperations(
        operationsValue, selectedTemplateIds,
        canonical.value(QStringLiteral("geometryObjects")).toArray(), vehicles, equipment);
    errors.append(serializedOperations.errors);
    if (serializedOperations.supplied)
        canonical.insert(QStringLiteral("operations"), serializedOperations.json);

    const QVariantMap constraints = missionContext.value(QStringLiteral("constraints")).toMap();
    if (!constraints.isEmpty())
        canonical.insert(QStringLiteral("constraints"),
                         QJsonObject::fromVariantMap(constraints));

    errors.removeDuplicates();
    result.insert(QStringLiteral("valid"), errors.isEmpty());
    result.insert(QStringLiteral("errors"), errors);
    result.insert(QStringLiteral("missionModelJson"),
                  QString::fromUtf8(QJsonDocument(canonical).toJson(QJsonDocument::Compact)));
    result.insert(QStringLiteral("schemaVersion"), QStringLiteral("2"));
    result.insert(QStringLiteral("configurationComplete"),
                  !vehicles.isEmpty() && !equipment.isEmpty());
    result.insert(QStringLiteral("taskExecutionEstimatesReady"),
                  serializedOperations.supplied && serializedOperations.allEstimatesValidated);
    result.insert(QStringLiteral("flightReleaseEligible"), false);
    return result;
}


QVariantMap ManualMissionModelAdapter::buildAutoMissionV2(
    const QStringList &selectedTemplateIds,
    const QString &missionId,
    int missionVersion,
    const QVariantMap &missionContext) const
{
    QStringList errors;
    if (missionId.trimmed().isEmpty())
        errors.push_back(QStringLiteral("MISSION_ID_REQUIRED"));
    if (missionVersion < 1)
        errors.push_back(QStringLiteral("MISSION_VERSION_INVALID"));
    if (selectedTemplateIds.isEmpty())
        errors.push_back(QStringLiteral("AUTO_TEMPLATE_REQUIRED"));

    const QString taskDescription =
        missionContext.value(QStringLiteral("taskDescription")).toString().trimmed();
    const QString objective =
        missionContext.value(QStringLiteral("objective")).toString().trimmed();
    const QString taskType =
        missionContext.value(QStringLiteral("taskType")).toString().trimmed();
    if (taskDescription.isEmpty())
        errors.push_back(QStringLiteral("TASK_DESCRIPTION_REQUIRED"));
    if (objective.isEmpty())
        errors.push_back(QStringLiteral("MISSION_OBJECTIVE_REQUIRED"));
    if (taskType.isEmpty())
        errors.push_back(QStringLiteral("MISSION_TASK_TYPE_REQUIRED"));

    const QVariant vehiclesValue = missionContext.value(QStringLiteral("vehicleAssignments"));
    const QVariant equipmentValue = missionContext.value(QStringLiteral("equipmentAssignments"));
    if (!vehiclesValue.canConvert<QVariantList>())
        errors.push_back(QStringLiteral("VEHICLE_ASSIGNMENTS_INVALID"));
    if (!equipmentValue.canConvert<QVariantList>())
        errors.push_back(QStringLiteral("EQUIPMENT_ASSIGNMENTS_INVALID"));

    QJsonArray templates;
    for (const QString &id : selectedTemplateIds)
        templates.append(id);

    const QVariantList vehicles = vehiclesValue.canConvert<QVariantList>()
        ? vehiclesValue.toList() : QVariantList{};
    const QVariantList equipment = equipmentValue.canConvert<QVariantList>()
        ? equipmentValue.toList() : QVariantList{};
    QJsonObject canonical{
        {QStringLiteral("missionId"), missionId},
        {QStringLiteral("missionVersion"), missionVersion},
        {QStringLiteral("status"), QStringLiteral("DRAFT")},
        {QStringLiteral("missionMode"), QStringLiteral("AUTO")},
        {QStringLiteral("selectedTemplateIds"), templates},
        {QStringLiteral("task"), QJsonObject{
            {QStringLiteral("description"), taskDescription}
        }},
        {QStringLiteral("intent"), QJsonObject{
            {QStringLiteral("objective"), objective},
            {QStringLiteral("taskType"), taskType}
        }},
        {QStringLiteral("geometryObjects"), QJsonArray{}},
        {QStringLiteral("vehicleAssignments"), QJsonArray::fromVariantList(vehicles)},
        {QStringLiteral("equipmentAssignments"), QJsonArray::fromVariantList(equipment)}
    };

    const QVariantMap constraints = missionContext.value(QStringLiteral("constraints")).toMap();
    if (!constraints.isEmpty())
        canonical.insert(QStringLiteral("constraints"), QJsonObject::fromVariantMap(constraints));

    errors.removeDuplicates();
    return {
        {QStringLiteral("valid"), errors.isEmpty()},
        {QStringLiteral("errors"), errors},
        {QStringLiteral("schemaVersion"), QStringLiteral("2")},
        {QStringLiteral("missionModelJson"),
         QString::fromUtf8(QJsonDocument(canonical).toJson(QJsonDocument::Compact))},
        {QStringLiteral("configurationComplete"), !vehicles.isEmpty() && !equipment.isEmpty()},
        {QStringLiteral("geometryComplete"), false},
        {QStringLiteral("flightReleaseEligible"), false}
    };
}
