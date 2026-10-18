#include "../src/RouteController.h"

#include <QCoreApplication>

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

static QVariantMap waypoint(const QString &id, double lat, double lon, double altitude)
{
    return {
        {QStringLiteral("waypointId"), id},
        {QStringLiteral("latitudeDeg"), lat},
        {QStringLiteral("longitudeDeg"), lon},
        {QStringLiteral("altitudeM"), altitude},
        {QStringLiteral("mandatory"), false},
        {QStringLiteral("altitudeReference"), QStringLiteral("AGL")}
    };
}

static QVariantMap candidate()
{
    return {
        {QStringLiteral("routeId"), QStringLiteral("ROUTE-TEST-01")},
        {QStringLiteral("routeVersion"), QStringLiteral("v1")},
        {QStringLiteral("missionId"), QStringLiteral("MISSION-TEST-01")},
        {QStringLiteral("missionVersion"), QStringLiteral("m1")},
        {QStringLiteral("generatorId"), QStringLiteral("planner-test")},
        {QStringLiteral("generatorVersion"), QStringLiteral("1")},
        {QStringLiteral("calculationInputVersion"), QStringLiteral("input-1")},
        {QStringLiteral("coordinateReferenceSystem"), QStringLiteral("EPSG:4326")},
        {QStringLiteral("waypoints"), QVariantList{
             waypoint(QStringLiteral("WP-1"), 60.0, 24.0, 100.0),
             waypoint(QStringLiteral("WP-2"), 60.1, 24.1, 110.0)
         }}
    };
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    RouteController controller;

    assert(controller.validationState() == QStringLiteral("EMPTY"));
    assert(!controller.publishValidation(QStringLiteral("v1"), QStringLiteral("input-1"),
                                         true, QString()));
    assert(controller.loadCandidate(candidate()));
    assert(controller.waypoints().size() == 2);
    const QVariantMap snapshot = controller.routeSnapshot();
    assert(snapshot.value(QStringLiteral("routeId")).toString() == QStringLiteral("ROUTE-TEST-01"));
    assert(snapshot.value(QStringLiteral("missionVersion")).toString() == QStringLiteral("m1"));
    const QVariantMap snapshotWaypoint = snapshot.value(QStringLiteral("waypoints")).toList()[0].toMap();
    assert(snapshotWaypoint.value(QStringLiteral("waypointId")).toString() == QStringLiteral("WP-1"));
    assert(snapshotWaypoint.value(QStringLiteral("latitudeDeg")).toDouble() == 60.0);
    assert(snapshotWaypoint.value(QStringLiteral("longitudeDeg")).toDouble() == 24.0);
    assert(controller.validationState() == QStringLiteral("CANDIDATE_UNVALIDATED"));

    // Mandatory status is route data and must survive normalization unchanged.
    QVariantMap mandatoryCandidate = candidate();
    QVariantList mandatoryPoints = mandatoryCandidate.value(QStringLiteral("waypoints")).toList();
    QVariantMap mandatoryWaypoint = mandatoryPoints[0].toMap();
    mandatoryWaypoint.insert(QStringLiteral("mandatory"), true);
    mandatoryPoints[0] = mandatoryWaypoint;
    mandatoryCandidate.insert(QStringLiteral("waypoints"), mandatoryPoints);
    assert(controller.loadCandidate(mandatoryCandidate));
    assert(controller.waypoints()[0].toMap().value(QStringLiteral("mandatory")).toBool());

    const QString originalVersion = controller.routeVersion();
    assert(controller.publishValidation(originalVersion, QStringLiteral("input-1"),
                                        true, QStringLiteral("validator-pass")));
    assert(controller.validationState() == QStringLiteral("VALIDATED"));

    const QVariantMap untouchedWaypoint = controller.waypoints()[0].toMap();
    assert(controller.editWaypointAltitude(QStringLiteral("WP-2"), 125.0));
    assert(controller.routeVersion() != originalVersion);
    assert(controller.validationState() == QStringLiteral("RECALCULATION_REQUIRED"));
    assert(controller.waypoints()[0].toMap() == untouchedWaypoint);
    assert(controller.waypoints()[1].toMap().value(QStringLiteral("altitudeM")).toDouble() == 125.0);

    // Neither an older route version nor a mismatched calculation-input version may validate edits.
    assert(!controller.publishValidation(originalVersion, QStringLiteral("input-1"),
                                         true, QStringLiteral("stale-pass")));
    assert(controller.validationState() == QStringLiteral("RECALCULATION_REQUIRED"));
    assert(!controller.publishValidation(controller.routeVersion(), QStringLiteral("input-stale"),
                                         true, QStringLiteral("stale-input-pass")));
    assert(controller.validationState() == QStringLiteral("RECALCULATION_REQUIRED"));

    const QString editedVersion = controller.routeVersion();
    assert(controller.publishValidation(editedVersion, QStringLiteral("input-1"),
                                        true, QStringLiteral("revalidated")));
    assert(controller.validationState() == QStringLiteral("VALIDATED"));

    assert(!controller.editWaypointAltitude(QStringLiteral("WP-2"), 5001.0));
    assert(controller.validationState() == QStringLiteral("BLOCKED"));

    QVariantMap invalid = candidate();
    invalid.insert(QStringLiteral("waypoints"), QVariantList{
        waypoint(QStringLiteral("WP-1"), 91.0, 24.0, 100.0),
        waypoint(QStringLiteral("WP-2"), 60.1, 24.1, 110.0)
    });
    assert(!controller.loadCandidate(invalid));
    assert(controller.validationReason() == QStringLiteral("INVALID_WAYPOINT_POSITION"));

    invalid = candidate();
    invalid.insert(QStringLiteral("coordinateReferenceSystem"), QStringLiteral("LOCAL_XY"));
    assert(!controller.loadCandidate(invalid));
    assert(controller.validationReason() == QStringLiteral("UNSUPPORTED_COORDINATE_REFERENCE_SYSTEM"));

    invalid = candidate();
    QVariantMap unsupported = waypoint(QStringLiteral("WP-1"), 60.0, 24.0, 100.0);
    unsupported.insert(QStringLiteral("altitudeReference"), QStringLiteral("AMSL"));
    invalid.insert(QStringLiteral("waypoints"), QVariantList{
        unsupported, waypoint(QStringLiteral("WP-2"), 60.1, 24.1, 110.0)
    });
    assert(!controller.loadCandidate(invalid));
    assert(controller.validationReason() == QStringLiteral("UNSUPPORTED_ALTITUDE_REFERENCE"));

    controller.clear();
    assert(controller.validationState() == QStringLiteral("EMPTY"));
    assert(controller.waypoints().isEmpty());
    return 0;
}
