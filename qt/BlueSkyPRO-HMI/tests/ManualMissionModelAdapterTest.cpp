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
        "{\"objects\":[{"
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

    // Explicit altitude and its datum must survive map JSON -> typed model -> canonical JSON.
    const QString routeWithDatum = QString::fromUtf8(R"json({"objects":[{"geometryId":"GEO-MT-05-1","geometryType":"CORRIDOR","coordinateReferenceSystem":"EPSG:4326","coordinates":[[30.0,59.0,40.0,"AGL"],[30.01,59.01,45.0,"AGL"]],"templateIds":["MT-05"],"version":1}]})json");
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
    const QString routeWithoutDatum = QString::fromUtf8(R"json({"objects":[{"geometryId":"GEO-MT-05-2","geometryType":"CORRIDOR","coordinateReferenceSystem":"EPSG:4326","coordinates":[[30.0,59.0,40.0],[30.01,59.01,45.0]],"templateIds":["MT-05"],"version":1}]})json");
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
        "{\"objects\":[{"
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

    return 0;
}
