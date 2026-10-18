#include "ManualMissionModelAdapter.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

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
