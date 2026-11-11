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
        {QStringLiteral("environmentSnapshotRefs"), QVariantMap{
             {QStringLiteral("terrainSnapshotId"), QStringLiteral("terrain-snapshot-001")},
             {QStringLiteral("airspaceSnapshotId"), QStringLiteral("airspace-snapshot-001")},
             {QStringLiteral("weatherSnapshotId"), QStringLiteral("weather-snapshot-001")},
             {QStringLiteral("windSnapshotId"), QStringLiteral("wind-snapshot-001")}
         }},
        {QStringLiteral("coordinateReferenceSystem"), QStringLiteral("EPSG:4326")},
        {QStringLiteral("waypoints"), QVariantList{
             waypoint(QStringLiteral("WP-1"), 60.0, 24.0, 100.0),
             waypoint(QStringLiteral("WP-2"), 60.1, 24.1, 110.0)
         }}
    };
}

static QVariantMap weatherCandidate(const QString &routeId,
                                         const QString &candidateId,
                                         double estimatedTime)
{
    QVariantMap value = candidate();
    value.insert(QStringLiteral("routeId"), routeId);
    value.insert(QStringLiteral("routeVersion"), QStringLiteral("v1"));
    value.insert(QStringLiteral("trajectoryGeometryComplete"), true);
    QVariantMap environment = value.value(QStringLiteral("environmentSnapshotRefs")).toMap();
    environment.insert(QStringLiteral("weatherSnapshotId"), QString());
    environment.insert(QStringLiteral("windSnapshotId"), QString());
    value.insert(QStringLiteral("environmentSnapshotRefs"), environment);
    value.insert(QStringLiteral("validationState"), QStringLiteral("VALIDATED"));
    value.insert(QStringLiteral("flightSafetyValidationState"), QStringLiteral("ALLOWED"));
    value.insert(QStringLiteral("flightSafetyEnvelopeId"), QStringLiteral("UAV-ENVELOPE-01"));
    value.insert(QStringLiteral("flightSafetyVehicleId"), QStringLiteral("UAV-TEST-01"));
    value.insert(QStringLiteral("candidateId"), candidateId);
    value.insert(QStringLiteral("estimatedTimeS"), estimatedTime);
    value.insert(QStringLiteral("estimatedEnergyWh"), estimatedTime * 0.5);
    value.insert(QStringLiteral("estimatedReserveWh"), 100.0 - estimatedTime * 0.5);
    value.insert(QStringLiteral("objectiveScore"), estimatedTime);
    value.insert(QStringLiteral("segments"), QVariantList{
        QVariantMap{
            {QStringLiteral("segmentId"), routeId + QStringLiteral("-SEG-1")},
            {QStringLiteral("fromWaypointId"), QStringLiteral("WP-1")},
            {QStringLiteral("toWaypointId"), QStringLiteral("WP-2")},
            {QStringLiteral("distanceM"), 12400.0},
            {QStringLiteral("nominalSpeedMps"), 15.0},
            {QStringLiteral("bankAngleRad"), 0.2},
            {QStringLiteral("lateralAccelerationMps2"), 1.0}
        }
    });
    return value;
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
    const QVariantMap environmentSnapshotRefs = snapshot.value(QStringLiteral("environmentSnapshotRefs")).toMap();
    assert(environmentSnapshotRefs.value(QStringLiteral("terrainSnapshotId")).toString()
           == QStringLiteral("terrain-snapshot-001"));
    assert(environmentSnapshotRefs.value(QStringLiteral("windSnapshotId")).toString()
           == QStringLiteral("wind-snapshot-001"));
    const QVariantMap snapshotWaypoint = snapshot.value(QStringLiteral("waypoints")).toList()[0].toMap();
    assert(snapshotWaypoint.value(QStringLiteral("waypointId")).toString() == QStringLiteral("WP-1"));
    assert(snapshotWaypoint.value(QStringLiteral("latitudeDeg")).toDouble() == 60.0);
    assert(snapshotWaypoint.value(QStringLiteral("longitudeDeg")).toDouble() == 24.0);
    assert(!snapshot.value(QStringLiteral("trajectoryGeometryComplete")).toBool());
    assert(snapshot.value(QStringLiteral("segments")).toList().isEmpty());
    assert(controller.validationState() == QStringLiteral("CANDIDATE_UNVALIDATED"));

    // A weather-capable route must carry explicit, ordered segment geometry.
    QVariantMap completeCandidate = candidate();
    completeCandidate.insert(QStringLiteral("trajectoryGeometryComplete"), true);
    completeCandidate.insert(QStringLiteral("segments"), QVariantList{
        QVariantMap{
            {QStringLiteral("segmentId"), QStringLiteral("SEG-1")},
            {QStringLiteral("fromWaypointId"), QStringLiteral("WP-1")},
            {QStringLiteral("toWaypointId"), QStringLiteral("WP-2")},
            {QStringLiteral("distanceM"), 12400.0},
            {QStringLiteral("nominalSpeedMps"), 15.0}
        }
    });
    assert(controller.loadCandidate(completeCandidate));
    const QVariantMap completeSnapshot = controller.routeSnapshot();
    assert(completeSnapshot.value(QStringLiteral("trajectoryGeometryComplete")).toBool());
    assert(completeSnapshot.value(QStringLiteral("segments")).toList().size() == 1);

    QVariantMap malformedComplete = completeCandidate;
    malformedComplete.remove(QStringLiteral("segments"));
    assert(!controller.loadCandidate(malformedComplete));
    assert(controller.validationReason()
           == QStringLiteral("COMPLETE_ROUTE_GEOMETRY_SEGMENTS_REQUIRED"));
    assert(controller.waypoints().isEmpty());

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
    assert(!controller.routeSnapshot().value(QStringLiteral("trajectoryGeometryComplete")).toBool());
    assert(controller.routeSnapshot().value(QStringLiteral("segments")).toList().isEmpty());

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

    // A late stale result must not rewrite the status/reason of the current
    // validated route. Its route and calculation-input lineage are unchanged.
    const QString acceptedReason = controller.validationReason();
    assert(!controller.publishValidation(originalVersion, QStringLiteral("input-1"),
                                         false, QStringLiteral("late-stale-rejection")));
    assert(controller.validationState() == QStringLiteral("VALIDATED"));
    assert(controller.validationReason() == acceptedReason);
    assert(controller.routeVersion() == editedVersion);

    assert(!controller.editWaypointAltitude(QStringLiteral("WP-2"), 5001.0));
    assert(controller.validationState() == QStringLiteral("BLOCKED"));

    QVariantMap invalid = candidate();
    invalid.insert(QStringLiteral("waypoints"), QVariantList{
        waypoint(QStringLiteral("WP-1"), 91.0, 24.0, 100.0),
        waypoint(QStringLiteral("WP-2"), 60.1, 24.1, 110.0)
    });
    assert(!controller.loadCandidate(invalid));
    assert(controller.validationReason() == QStringLiteral("INVALID_WAYPOINT_POSITION"));

    // A malformed replacement must clear the previously loaded route instead of
    // leaving stale waypoints visible under a BLOCKED status.
    assert(controller.loadCandidate(candidate()));
    assert(controller.waypoints().size() == 2);
    invalid = candidate();
    invalid.insert(QStringLiteral("waypoints"), QVariantList{
        waypoint(QStringLiteral("WP-1"), 91.0, 24.0, 100.0),
        waypoint(QStringLiteral("WP-2"), 60.1, 24.1, 110.0)
    });
    assert(!controller.loadCandidate(invalid));
    assert(controller.validationState() == QStringLiteral("BLOCKED"));
    assert(controller.validationReason() == QStringLiteral("INVALID_WAYPOINT_POSITION"));
    assert(controller.waypoints().isEmpty());

    malformedComplete = completeCandidate;
    QVariantList brokenSegments = malformedComplete.value(QStringLiteral("segments")).toList();
    QVariantMap brokenSegment = brokenSegments[0].toMap();
    brokenSegment.insert(QStringLiteral("fromWaypointId"), QStringLiteral("WP-2"));
    brokenSegments[0] = brokenSegment;
    malformedComplete.insert(QStringLiteral("segments"), brokenSegments);
    assert(!controller.loadCandidate(malformedComplete));
    assert(controller.validationReason() == QStringLiteral("ROUTE_SEGMENT_CHAIN_DISCONTINUITY"));
    assert(controller.waypoints().isEmpty());

    invalid = candidate();
    invalid.remove(QStringLiteral("environmentSnapshotRefs"));
    assert(!controller.loadCandidate(invalid));
    assert(controller.validationReason() == QStringLiteral("TERRAIN_AND_AIRSPACE_SNAPSHOT_LINEAGE_REQUIRED"));
    assert(controller.waypoints().isEmpty());

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

    invalid = candidate();
    QVariantMap malformedMandatory = waypoint(QStringLiteral("WP-1"), 60.0, 24.0, 100.0);
    malformedMandatory.insert(QStringLiteral("mandatory"), QStringLiteral("true"));
    invalid.insert(QStringLiteral("waypoints"), QVariantList{
        malformedMandatory, waypoint(QStringLiteral("WP-2"), 60.1, 24.1, 110.0)
    });
    assert(!controller.loadCandidate(invalid));
    assert(controller.validationReason() == QStringLiteral("MANDATORY_FLAG_INVALID"));

    QVariantMap taskExecutionCandidate = completeCandidate;
    QVariantList taskWaypoints = taskExecutionCandidate.value(QStringLiteral("waypoints")).toList();
    QVariantMap taskWaypoint = taskWaypoints[1].toMap();
    taskWaypoint.insert(QStringLiteral("taskExecutionDurationS"), 45.0);
    taskWaypoint.insert(QStringLiteral("taskExecutionEnergyWh"), 8.5);
    taskWaypoints[1] = taskWaypoint;
    taskExecutionCandidate.insert(QStringLiteral("waypoints"), taskWaypoints);
    assert(controller.loadCandidate(taskExecutionCandidate));
    const QVariantMap taskSnapshotWaypoint =
        controller.routeSnapshot().value(QStringLiteral("waypoints")).toList()[1].toMap();
    assert(taskSnapshotWaypoint.value(QStringLiteral("taskExecutionDurationS")).toDouble() == 45.0);
    assert(taskSnapshotWaypoint.value(QStringLiteral("taskExecutionEnergyWh")).toDouble() == 8.5);

    QVariantMap incompleteTaskExecution = taskExecutionCandidate;
    taskWaypoints = incompleteTaskExecution.value(QStringLiteral("waypoints")).toList();
    taskWaypoint = taskWaypoints[1].toMap();
    taskWaypoint.remove(QStringLiteral("taskExecutionEnergyWh"));
    taskWaypoints[1] = taskWaypoint;
    incompleteTaskExecution.insert(QStringLiteral("waypoints"), taskWaypoints);
    assert(!controller.loadCandidate(incompleteTaskExecution));
    assert(controller.validationReason()
           == QStringLiteral("TASK_EXECUTION_DURATION_AND_ENERGY_MUST_BE_SUPPLIED_TOGETHER"));

    QVariantMap weatherIndependentRoute = completeCandidate;
    QVariantMap preWeatherEnvironment =
        weatherIndependentRoute.value(QStringLiteral("environmentSnapshotRefs")).toMap();
    preWeatherEnvironment.insert(QStringLiteral("weatherSnapshotId"), QString());
    preWeatherEnvironment.insert(QStringLiteral("windSnapshotId"), QString());
    weatherIndependentRoute.insert(QStringLiteral("environmentSnapshotRefs"), preWeatherEnvironment);
    assert(controller.loadCandidate(weatherIndependentRoute));
    assert(controller.routeSnapshot().value(QStringLiteral("environmentSnapshotRefs")).toMap()
               .value(QStringLiteral("windSnapshotId")).toString().isEmpty());

    const QVariantMap candidateA = weatherCandidate(
        QStringLiteral("ROUTE-CAND-A"), QStringLiteral("CAND-A"), 120.0);
    const QVariantMap candidateB = weatherCandidate(
        QStringLiteral("ROUTE-CAND-B"), QStringLiteral("CAND-B"), 100.0);
    assert(controller.loadCandidateSet(
        QVariantList{candidateA, candidateB}, QStringLiteral("CAND-B")));
    assert(controller.candidateSnapshots().size() == 2);
    assert(controller.routeId() == QStringLiteral("ROUTE-CAND-B"));
    assert(controller.validationState() == QStringLiteral("VALIDATED"));
    assert(controller.candidateSnapshots().at(0).toMap()
               .value(QStringLiteral("validationState")).toString() == QStringLiteral("VALIDATED"));
    assert(controller.selectCandidate(QStringLiteral("CAND-A")));
    assert(controller.routeId() == QStringLiteral("ROUTE-CAND-A"));
    assert(controller.candidateSnapshots().size() == 2);

    QVariantMap mismatchedCandidate = candidateB;
    mismatchedCandidate.insert(QStringLiteral("calculationInputVersion"), QStringLiteral("input-2"));
    assert(!controller.loadCandidateSet(
        QVariantList{candidateA, mismatchedCandidate}, QStringLiteral("CAND-A")));
    assert(controller.candidateSnapshots().isEmpty());
    assert(controller.validationState() == QStringLiteral("BLOCKED"));
    assert(controller.validationReason()
           == QStringLiteral("CANDIDATE_SET_MISSION_INPUT_OR_SAFETY_ENVELOPE_MISMATCH"));

    controller.clear();
    assert(controller.validationState() == QStringLiteral("EMPTY"));
    assert(controller.waypoints().isEmpty());
    return 0;
}
