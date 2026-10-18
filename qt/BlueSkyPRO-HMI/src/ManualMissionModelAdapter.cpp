#include "ManualMissionModelAdapter.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "04_SOFTWARE/PLANNING/model/mission_model.hpp"
#include "04_SOFTWARE/PLANNING/model/mission_model_validation.hpp"

#include <cmath>
#include <string>
#include <utility>

using bluesky::planning::Mission;
using bluesky::planning::MissionGeometryObject;
using bluesky::planning::MissionGeometryVertex;
using bluesky::planning::validateMissionModelStructure;

namespace
{
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
                if (point.size() > 2 && point.at(2).isDouble())
                    vertex.altitude_m = point.at(2).toDouble();
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
            if (point.altitude_m)
                coordinate.append(*point.altitude_m);
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

    QJsonObject canonical{
        {QStringLiteral("missionId"), missionId},
        {QStringLiteral("missionVersion"), missionVersion},
        {QStringLiteral("missionMode"), QStringLiteral("MANUAL")},
        {QStringLiteral("selectedTemplateIds"), templateArray},
        {QStringLiteral("geometryObjects"), geometryArray}
    };

    result.insert(QStringLiteral("valid"), errors.isEmpty());
    result.insert(QStringLiteral("errors"), errors);
    result.insert(QStringLiteral("missionModelJson"),
                  QString::fromUtf8(QJsonDocument(canonical).toJson(QJsonDocument::Compact)));
    result.insert(QStringLiteral("structuralValidationOnly"), true);
    result.insert(QStringLiteral("flightReleaseEligible"), false);
    return result;
}
