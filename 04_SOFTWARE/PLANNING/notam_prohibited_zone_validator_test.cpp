#include "notam_prohibited_zone_validator.hpp"
#include <cassert>

using namespace bluesky::planning;

static Route route() {
    Route r;
    r.lineage.route_id = "R-NOTAM-1";
    r.lineage.route_version = "1";
    r.environment.airspace_snapshot_id = "NOTAM-SNAP-1";
    r.constraints.avoid_restricted_areas = true;
    r.waypoints = {{"WP-1", {60.0, 25.0}, 100.0, true, AltitudeReference::AGL},
                   {"WP-2", {60.01, 25.01}, 100.0, false, AltitudeReference::AGL}};
    return r;
}

static NotamSnapshot snapshot() {
    NotamSnapshot s;
    s.snapshot_id = "NOTAM-SNAP-1";
    s.source_id = "AUTH-NOTAM";
    s.source_version = "2026-09-24T12:00Z";
    s.evaluation_epoch_s = 1000;
    s.planned_mission_start_epoch_s = 1000;
    s.planned_mission_end_epoch_s = 2000;
     s.zones.push_back({"N-001", NotamRestrictionType::Prohibited, NotamGeometryType::Circle,
                       {60.0, 25.0}, 1000.0, {}, 0.0, 500.0, 900, 1100,
                       NotamAltitudeReference::AGL, "N-001-COMP-01"});
    return s;
}

int main() {
    auto r = route();
    auto s = snapshot();
    auto blocked = NotamProhibitedZoneValidator::validate(r, s);
    assert(blocked.status == RouteValidationStatus::Rejected);
    assert(blocked.findings.size() >= 1);
    assert(blocked.findings.front().waypoint_id == "WP-1");
    assert(blocked.findings.front().notam_id == "N-001");

    // NOTAM validity is inclusive at both endpoints; outside its interval the zone is not active.
    s.evaluation_epoch_s = 899;
    auto before_validity = NotamProhibitedZoneValidator::validate(r, s);
    assert(before_validity.status == RouteValidationStatus::Allowed);
    s.evaluation_epoch_s = 900;
    auto at_validity_start = NotamProhibitedZoneValidator::validate(r, s);
    assert(at_validity_start.status == RouteValidationStatus::Rejected);
    s.evaluation_epoch_s = 1100;
    auto at_validity_end = NotamProhibitedZoneValidator::validate(r, s);
    assert(at_validity_end.status == RouteValidationStatus::Rejected);
    s.evaluation_epoch_s = 1101;
    auto after_validity = NotamProhibitedZoneValidator::validate(r, s);
    assert(after_validity.status == RouteValidationStatus::Allowed);
    s.evaluation_epoch_s = 1000;

    // A segment crossing a circle must be rejected even when both endpoints are outside.
    r.waypoints = {{"A", {60.0, 24.98}, 100.0, true, AltitudeReference::AGL},
                   {"B", {60.0, 25.02}, 100.0, false, AltitudeReference::AGL}};
    s.zones.front().center = {60.0, 25.0};
    s.zones.front().radius_m = 150.0;
    auto circle_crossing = NotamProhibitedZoneValidator::validate(r, s);
    assert(circle_crossing.status == RouteValidationStatus::Rejected);
    assert(circle_crossing.findings.front().code == NotamValidationCode::SegmentIntersectsProhibitedZone);
    assert(circle_crossing.findings.front().segment_start_waypoint_id == "A");
    assert(circle_crossing.findings.front().segment_end_waypoint_id == "B");

    // A segment crossing a polygon must be rejected when endpoints are outside.
    s.zones.front() = {"N-POLY", NotamRestrictionType::Prohibited, NotamGeometryType::Polygon,
                       {}, 0.0, {{59.999, 24.999}, {60.001, 24.999},
                                 {60.001, 25.001}, {59.999, 25.001}},
                       0.0, 500.0, 900, 1100, NotamAltitudeReference::AGL, "N-POLY-COMP-01"};
    auto polygon_crossing = NotamProhibitedZoneValidator::validate(r, s);
    assert(polygon_crossing.status == RouteValidationStatus::Rejected);
    bool polygon_finding = false;
    for (const auto& finding : polygon_crossing.findings)
        if (finding.code == NotamValidationCode::SegmentIntersectsProhibitedZone &&
            finding.notam_id == "N-POLY") polygon_finding = true;
    assert(polygon_finding);

    // Altitude outside the restriction means the horizontal crossing is allowed.
    r.waypoints[0].altitude_m = 700.0;
    r.waypoints[1].altitude_m = 700.0;
    auto altitude_clear = NotamProhibitedZoneValidator::validate(r, s);
    assert(altitude_clear.status == RouteValidationStatus::Allowed);

    // Existing snapshot, geometry, and coordinate validation remains enforced.
    r = route();
    auto mismatched = s;
    mismatched.snapshot_id = "NOTAM-SNAP-OTHER";
    auto mismatch_result = NotamProhibitedZoneValidator::validate(r, mismatched);
    assert(mismatch_result.status == RouteValidationStatus::Rejected);
    assert(mismatch_result.findings.front().code == NotamValidationCode::SnapshotNotValid);

    auto invalid_zone = snapshot();
    invalid_zone.zones.front().radius_m = -1.0;
    auto invalid_result = NotamProhibitedZoneValidator::validate(r, invalid_zone);
    assert(invalid_result.status == RouteValidationStatus::Rejected);
    assert(invalid_result.findings.front().code == NotamValidationCode::SnapshotNotValid);

    r.waypoints.front().position = {91.0, 25.0};
    auto invalid_waypoint = NotamProhibitedZoneValidator::validate(r, snapshot());
    assert(invalid_waypoint.status == RouteValidationStatus::Rejected);
    assert(invalid_waypoint.findings.front().code == NotamValidationCode::SnapshotNotValid);

    // Unknown and non-AGL vertical references must fail closed rather than compare raw numbers.
    r = route();
    r.waypoints.front().altitude_reference = AltitudeReference::Unknown;
    auto unknown_route_datum = NotamProhibitedZoneValidator::validate(r, snapshot());
    assert(unknown_route_datum.status == RouteValidationStatus::Rejected);
    assert(unknown_route_datum.findings.front().code == NotamValidationCode::UnsupportedAltitudeReference);

    r = route();
    r.waypoints.front().altitude_reference = AltitudeReference::AMSL;
    auto amsl_route_datum = NotamProhibitedZoneValidator::validate(r, snapshot());
    assert(amsl_route_datum.status == RouteValidationStatus::Rejected);
    assert(amsl_route_datum.findings.front().code == NotamValidationCode::UnsupportedAltitudeReference);

    auto unknown_notam_datum = snapshot();
    unknown_notam_datum.zones.front().altitude_reference = NotamAltitudeReference::Unknown;
    auto unknown_notam_result = NotamProhibitedZoneValidator::validate(route(), unknown_notam_datum);
    assert(unknown_notam_result.status == RouteValidationStatus::Rejected);
    assert(unknown_notam_result.findings.front().code == NotamValidationCode::UnsupportedAltitudeReference);

    auto amsl_notam_datum = snapshot();
    amsl_notam_datum.zones.front().altitude_reference = NotamAltitudeReference::AMSL;
    auto amsl_notam_result = NotamProhibitedZoneValidator::validate(route(), amsl_notam_datum);
    assert(amsl_notam_result.status == RouteValidationStatus::Rejected);
    assert(amsl_notam_result.findings.front().code == NotamValidationCode::UnsupportedAltitudeReference);


    // Time-dependent validation uses actual segment occupancy after wind/performance.
    auto timedRoute = route();
    timedRoute.waypoints = {
        {"A", {60.0, 24.98}, 100.0, true, AltitudeReference::AGL},
        {"B", {60.0, 25.02}, 100.0, false, AltitudeReference::AGL}
    };
    timedRoute.segments = {{"S-TIMED", "A", "B", 2200.0, 10.0}};
    TrajectoryResult timed;
    timed.status = TrajectoryStatus::Feasible;
    timed.route_id = timedRoute.lineage.route_id;
    timed.route_version = timedRoute.lineage.route_version;
    timed.mission_start_epoch_s = 1000;
    timed.total_time_s = 200.0;
    timed.points = {
        {"A", timedRoute.waypoints[0].position, 100.0, 0.0, 0.0},
        {"B", timedRoute.waypoints[1].position, 100.0, 200.0, 0.0}
    };
    timed.segments = {{"S-TIMED", 11.0, 90.0, 200.0, 0.0}};

    auto timedSnapshot = snapshot();
    timedSnapshot.evaluation_epoch_s = 1000;
    timedSnapshot.planned_mission_end_epoch_s = 1200;
    timedSnapshot.zones.front().center = {60.0, 25.0};
    timedSnapshot.zones.front().radius_m = 150.0;
    timedSnapshot.zones.front().valid_from_epoch_s = 1100;
    timedSnapshot.zones.front().valid_to_epoch_s = 1200;
    // The static snapshot-instant check sees the zone as inactive; the 4D
    // validator must still reject because the segment crosses it during validity.
    assert(NotamProhibitedZoneValidator::validate(timedRoute, timedSnapshot).status ==
           RouteValidationStatus::Allowed);
    const auto timedConflict =
        NotamProhibitedZoneValidator::validate(timedRoute, timedSnapshot, timed);
    assert(timedConflict.status == RouteValidationStatus::Rejected);
    bool timedSegmentFinding = false;
    for (const auto& finding : timedConflict.findings)
        if (finding.code == NotamValidationCode::SegmentOccupancyOverlapsNotamValidity &&
            finding.notam_id == "N-001" &&
            finding.component_id == "N-001-COMP-01") timedSegmentFinding = true;
    assert(timedSegmentFinding);

    timedSnapshot.zones.front().valid_from_epoch_s = 1201;
    timedSnapshot.zones.front().valid_to_epoch_s = 1300;
    assert(NotamProhibitedZoneValidator::validate(timedRoute, timedSnapshot, timed).status ==
           RouteValidationStatus::Allowed);
    timedSnapshot.zones.front().valid_from_epoch_s = 1200;
    timedSnapshot.zones.front().valid_to_epoch_s = 1300;
    assert(NotamProhibitedZoneValidator::validate(timedRoute, timedSnapshot, timed).status ==
           RouteValidationStatus::Rejected);

    auto dwellRoute = timedRoute;
    dwellRoute.waypoints.front().task_execution_duration_s = 60.0;
    dwellRoute.waypoints.front().task_execution_energy_wh = 1.0;
    auto dwellTiming = timed;
    dwellTiming.points.front().elapsed_time_s = 60.0;
    dwellTiming.points.back().elapsed_time_s = 260.0;
    dwellTiming.total_time_s = 260.0;
    timedSnapshot.planned_mission_end_epoch_s = 1300;
    timedSnapshot.zones.front().center = dwellRoute.waypoints.front().position;
    timedSnapshot.zones.front().radius_m = 50.0;
    timedSnapshot.zones.front().valid_from_epoch_s = 1000;
    timedSnapshot.zones.front().valid_to_epoch_s = 1050;
    const auto dwellConflict =
        NotamProhibitedZoneValidator::validate(dwellRoute, timedSnapshot, dwellTiming);
    assert(dwellConflict.status == RouteValidationStatus::Rejected);
    bool dwellFinding = false;
    for (const auto& finding : dwellConflict.findings)
        if (finding.code == NotamValidationCode::WaypointOccupancyOverlapsNotamValidity)
            dwellFinding = true;
    assert(dwellFinding);

    auto missingEpoch = timed;
    missingEpoch.mission_start_epoch_s = 0;
    assert(NotamProhibitedZoneValidator::validate(timedRoute, timedSnapshot, missingEpoch).status ==
           RouteValidationStatus::Rejected);
    auto wrongRouteVersion = timed;
    wrongRouteVersion.route_version = "stale";
    assert(NotamProhibitedZoneValidator::validate(timedRoute, timedSnapshot, wrongRouteVersion).status ==
           RouteValidationStatus::Rejected);

    r = route();
    r.environment.airspace_snapshot_id.clear();
    auto missing = NotamProhibitedZoneValidator::validate(r, snapshot());
    assert(missing.status == RouteValidationStatus::Rejected);
    assert(missing.findings.front().code == NotamValidationCode::MissingSnapshotReference);
}
