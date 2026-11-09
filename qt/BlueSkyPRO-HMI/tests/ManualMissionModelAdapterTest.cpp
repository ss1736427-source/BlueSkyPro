#include "ManualMissionModelAdapter.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QString>

#include <cassert>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    ManualMissionModelAdapter adapter;

    const QString geometry = QStringLiteral(
        "{\"origin\":{\"longitudeDeg\":30.2,\"latitudeDeg\":59.3,\"coordinateReferenceSystem\":\"EPSG:4326\",\"originKind\":\"PLANNED_TAKEOFF\"},\"objects\":[{"
        "\"geometryId\":\"GEO-MT-01-1\","
        "\"geometryType\":\"POLYGON_AOI\","
        "\"coordinateReferenceSystem\":\"EPSG:4326\","
        "\"coordinates\":[[30.0,59.0],[30.01,59.0],[30.01,59.01],[30.0,59.0]],"
        "\"templateIds\":[\"MT-01\"],\"version\":1}]}");

    const QVariantMap valid = adapter.buildMission(
        geometry, {QStringLiteral("MT-01")}, QStringLiteral("MISSION-1"), 3, true);
    assert(valid.value(QStringLiteral("valid")).toBool());
    assert(valid.value(QStringLiteral("structuralValidationOnly")).toBool());
    assert(!valid.value(QStringLiteral("flightReleaseEligible")).toBool());

    const QJsonDocument canonical = QJsonDocument::fromJson(
        valid.value(QStringLiteral("missionModelJson")).toString().toUtf8());
    assert(canonical.isObject());
    assert(canonical.object().value(QStringLiteral("missionMode")).toString() == QStringLiteral("MANUAL"));
    assert(canonical.object().value(QStringLiteral("geometryObjects")).toArray().size() == 1);

    // A planned takeoff origin is explicit and remains separate from work geometry.
    const QString withOrigin = QString::fromUtf8(
        R"json({"origin":{"longitudeDeg":30.2,"latitudeDeg":59.3,"coordinateReferenceSystem":"EPSG:4326","originKind":"PLANNED_TAKEOFF","horizontalAccuracyM":1.5,"trueNorthBearingDeg":12.0},"objects":[{"geometryId":"GEO-ORIGIN-TEST","geometryType":"POLYGON_AOI","coordinateReferenceSystem":"EPSG:4326","coordinates":[[30.0,59.0],[30.01,59.0],[30.01,59.01],[30.0,59.0]],"templateIds":["MT-01"],"version":1}]})json");
    const QVariantMap anchored = adapter.buildMission(
        withOrigin, {QStringLiteral("MT-01")}, QStringLiteral("MISSION-ORIGIN"), 1, true);
    assert(anchored.value(QStringLiteral("valid")).toBool());
    const QJsonDocument anchoredCanonical = QJsonDocument::fromJson(
        anchored.value(QStringLiteral("missionModelJson")).toString().toUtf8());
    const QJsonObject origin = anchoredCanonical.object().value(QStringLiteral("origin")).toObject();
    assert(origin.value(QStringLiteral("originKind")).toString() == QStringLiteral("PLANNED_TAKEOFF"));
    assert(origin.value(QStringLiteral("longitudeDeg")).toDouble() == 30.2);
    assert(origin.value(QStringLiteral("horizontalAccuracyM")).toDouble() == 1.5);
    assert(origin.value(QStringLiteral("trueNorthBearingDeg")).toDouble() == 12.0);

    const QString invalidOrigin = QString::fromUtf8(
        R"json({"origin":{"longitudeDeg":181.0,"latitudeDeg":59.3,"coordinateReferenceSystem":"EPSG:4326","originKind":"PLANNED_TAKEOFF"},"objects":[]})json");
    const QVariantMap rejectedOrigin = adapter.buildMission(
        invalidOrigin, {}, QStringLiteral("MISSION-ORIGIN-BAD"), 1, true);
    assert(!rejectedOrigin.value(QStringLiteral("valid")).toBool());
    assert(rejectedOrigin.value(QStringLiteral("errors")).toStringList().contains(
        QStringLiteral("MISSION_ORIGIN_COORDINATE_INVALID")));

    const QString unsupportedOriginCrs = QString::fromUtf8(
        R"json({"origin":{"longitudeDeg":30.2,"latitudeDeg":59.3,"coordinateReferenceSystem":"EPSG:3857","originKind":"PLANNED_TAKEOFF"},"objects":[{"geometryId":"GEO-ORIGIN-CRS","geometryType":"POLYGON_AOI","coordinateReferenceSystem":"EPSG:4326","coordinates":[[30.0,59.0],[30.01,59.0],[30.01,59.01],[30.0,59.0]],"templateIds":["MT-01"],"version":1}]})json");
    const QVariantMap rejectedOriginCrs = adapter.buildMission(
        unsupportedOriginCrs, {QStringLiteral("MT-01")},
        QStringLiteral("MISSION-ORIGIN-CRS"), 1, true);
    assert(!rejectedOriginCrs.value(QStringLiteral("valid")).toBool());
    assert(rejectedOriginCrs.value(QStringLiteral("errors")).toStringList().contains(
        QStringLiteral("MISSION_ORIGIN_CRS_UNSUPPORTED")));

    // Explicit altitude and its datum must survive map JSON -> typed model -> canonical JSON.
    const QString routeWithDatum = QString::fromUtf8(R"json({"origin":{"longitudeDeg":30.2,"latitudeDeg":59.3,"coordinateReferenceSystem":"EPSG:4326","originKind":"PLANNED_TAKEOFF"},"objects":[{"geometryId":"GEO-MT-05-1","geometryType":"CORRIDOR","coordinateReferenceSystem":"EPSG:4326","coordinates":[[30.0,59.0,40.0,"AGL"],[30.01,59.01,45.0,"AGL"]],"templateIds":["MT-05"],"version":1}]})json");
    const QVariantMap route = adapter.buildMission(
        routeWithDatum, {QStringLiteral("MT-05")}, QStringLiteral("MISSION-ROUTE"), 1, true);
    assert(route.value(QStringLiteral("valid")).toBool());
    const QJsonDocument routeCanonical = QJsonDocument::fromJson(
        route.value(QStringLiteral("missionModelJson")).toString().toUtf8());
    const QJsonArray routeObjects = routeCanonical.object()
        .value(QStringLiteral("geometryObjects")).toArray();
    const QJsonArray routeCoordinates = routeObjects.at(0).toObject()
        .value(QStringLiteral("coordinates")).toArray();
    assert(routeCoordinates.at(0).toArray().at(2).toDouble() == 40.0);
    assert(routeCoordinates.at(0).toArray().at(3).toString() == QStringLiteral("AGL"));

    // Missing datum is preserved as missing, never silently inferred by the adapter.
    const QString routeWithoutDatum = QString::fromUtf8(R"json({"origin":{"longitudeDeg":30.2,"latitudeDeg":59.3,"coordinateReferenceSystem":"EPSG:4326","originKind":"PLANNED_TAKEOFF"},"objects":[{"geometryId":"GEO-MT-05-2","geometryType":"CORRIDOR","coordinateReferenceSystem":"EPSG:4326","coordinates":[[30.0,59.0,40.0],[30.01,59.01,45.0]],"templateIds":["MT-05"],"version":1}]})json");
    const QVariantMap routeNoDatum = adapter.buildMission(
        routeWithoutDatum, {QStringLiteral("MT-05")}, QStringLiteral("MISSION-ROUTE"), 2, true);
    assert(routeNoDatum.value(QStringLiteral("valid")).toBool());
    const QJsonDocument noDatumCanonical = QJsonDocument::fromJson(
        routeNoDatum.value(QStringLiteral("missionModelJson")).toString().toUtf8());
    const QJsonArray noDatumCoordinates = noDatumCanonical.object()
        .value(QStringLiteral("geometryObjects")).toArray().at(0).toObject()
        .value(QStringLiteral("coordinates")).toArray();
    assert(noDatumCoordinates.at(0).toArray().size() == 3);

    const QVariantMap incomplete = adapter.buildMission(
        geometry, {QStringLiteral("MT-01")}, QStringLiteral("MISSION-1"), 4, false);
    assert(!incomplete.value(QStringLiteral("valid")).toBool());
    assert(incomplete.value(QStringLiteral("errors")).toStringList().contains(
        QStringLiteral("MANUAL_GEOMETRY_COMPOSITION_INCOMPLETE")));

    const QString invalidCoordinate = QStringLiteral(
        "{\"origin\":{\"longitudeDeg\":30.2,\"latitudeDeg\":59.3,\"coordinateReferenceSystem\":\"EPSG:4326\",\"originKind\":\"PLANNED_TAKEOFF\"},\"objects\":[{"
        "\"geometryId\":\"GEO-BAD\",\"geometryType\":\"WAYPOINT_ROUTE\","
        "\"coordinateReferenceSystem\":\"EPSG:4326\","
        "\"coordinates\":[[181.0,59.0],[30.0,59.01]],"
        "\"templateIds\":[\"MT-03\"],\"version\":1}]}");
    const QVariantMap invalid = adapter.buildMission(
        invalidCoordinate, {QStringLiteral("MT-03")}, QStringLiteral("MISSION-1"), 5, true);
    assert(!invalid.value(QStringLiteral("valid")).toBool());
    assert(invalid.value(QStringLiteral("errors")).toStringList().contains(
        QStringLiteral("GEOMETRY_COORDINATE_INVALID:GEO-BAD")));

    const QVariantMap malformed = adapter.buildMission(
        QStringLiteral("{\"objects\":{}}"), {QStringLiteral("MT-01")},
        QStringLiteral("MISSION-1"), 6, true);
    assert(!malformed.value(QStringLiteral("valid")).toBool());
    assert(malformed.value(QStringLiteral("errors")).toStringList().contains(
        QStringLiteral("MANUAL_GEOMETRY_JSON_INVALID")));


    // v2 serializes the full mission context and preserves an unassigned draft
    // as DRAFT; route calculation itself must reject missing vehicle/equipment.
    QVariantMap mt01Context{
        {QStringLiteral("taskDescription"), QStringLiteral("Картографировать AOI")},
        {QStringLiteral("objective"), QStringLiteral("Получить ортофотоплан")},
        {QStringLiteral("taskType"), QStringLiteral("MAPPING")},
        {QStringLiteral("vehicleAssignments"), QVariantList{}},
        {QStringLiteral("equipmentAssignments"), QVariantList{}}
    };
    const QVariantMap v2 = adapter.buildMissionV2(
        geometry, {QStringLiteral("MT-01")}, QStringLiteral("MISSION-V2"), 7, true, mt01Context);
    assert(v2.value(QStringLiteral("valid")).toBool());
    assert(v2.value(QStringLiteral("schemaVersion")).toString() == QStringLiteral("2"));
    assert(!v2.value(QStringLiteral("configurationComplete")).toBool());
    const QJsonDocument v2Document = QJsonDocument::fromJson(
        v2.value(QStringLiteral("missionModelJson")).toString().toUtf8());
    assert(v2Document.isObject());
    const QJsonObject v2Object = v2Document.object();
    assert(v2Object.value(QStringLiteral("status")).toString() == QStringLiteral("DRAFT"));
    assert(v2Object.value(QStringLiteral("task")).toObject()
               .value(QStringLiteral("description")).toString() == QStringLiteral("Картографировать AOI"));
    assert(v2Object.value(QStringLiteral("vehicleAssignments")).toArray().isEmpty());
    assert(v2Object.value(QStringLiteral("equipmentAssignments")).toArray().isEmpty());

    mt01Context.remove(QStringLiteral("objective"));
    const QVariantMap missingIntent = adapter.buildMissionV2(
        geometry, {QStringLiteral("MT-01")}, QStringLiteral("MISSION-V2"), 8, true, mt01Context);
    assert(!missingIntent.value(QStringLiteral("valid")).toBool());
    assert(missingIntent.value(QStringLiteral("errors")).toStringList().contains(
        QStringLiteral("MISSION_OBJECTIVE_REQUIRED")));


    QVariantMap autoContext{
        {QStringLiteral("taskDescription"), QStringLiteral("Картографирование участка")},
        {QStringLiteral("objective"), QStringLiteral("Создать ортофотоплан")},
        {QStringLiteral("taskType"), QStringLiteral("MAPPING")},
        {QStringLiteral("vehicleAssignments"), QVariantList{}},
        {QStringLiteral("equipmentAssignments"), QVariantList{}}
    };
    const QVariantMap autoMission = adapter.buildAutoMissionV2(
        {QStringLiteral("MT-01")}, QStringLiteral("MISSION-AUTO-V2"), 1, autoContext);
    assert(autoMission.value(QStringLiteral("valid")).toBool());
    const QJsonDocument autoDocument = QJsonDocument::fromJson(
        autoMission.value(QStringLiteral("missionModelJson")).toString().toUtf8());
    assert(autoDocument.isObject());
    assert(autoDocument.object().value(QStringLiteral("missionMode")).toString() == QStringLiteral("AUTO"));
    assert(autoDocument.object().value(QStringLiteral("status")).toString() == QStringLiteral("DRAFT"));
    assert(autoDocument.object().value(QStringLiteral("geometryObjects")).toArray().isEmpty());
    assert(!autoMission.value(QStringLiteral("flightReleaseEligible")).toBool());

    return 0;
}
