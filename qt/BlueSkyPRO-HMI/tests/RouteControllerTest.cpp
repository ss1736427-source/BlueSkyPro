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
    assert(controller.validationState() == QStringLiteral("CANDIDATE_UNVALIDATED"));

    const QString originalVersion = controller.routeVersion();
    assert(controller.publishValidation(originalVersion, QStringLiteral("input-1"),
                                        true, QStringLiteral("validator-pass")));
    assert(controller.validationState() == QStringLiteral("VALIDATED"));

    assert(controller.editWaypointAltitude(QStringLiteral("WP-2"), 125.0));
    assert(controller.routeVersion() != originalVersion);
    assert(controller.validationState() == QStringLiteral("RECALCULATION_REQUIRED"));
    assert(!controller.publishValidation(originalVersion, QStringLiteral("input-1"),
                                         true, QStringLiteral("stale-pass")));
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
