#include "notam_prohibited_zone_validator.hpp"
#include <cassert>

using namespace bluesky::planning;

static Route route() {
    Route r;
    r.lineage.route_id = "R-NOTAM-1";
    r.lineage.route_version = "1";
    r.environment.airspace_snapshot_id = "NOTAM-SNAP-1";
    r.constraints.avoid_restricted_areas = true;
    r.waypoints = {{"WP-1", {60.0, 25.0}, 100.0, true},
                   {"WP-2", {60.01, 25.01}, 100.0, false}};
    return r;
}

static NotamSnapshot snapshot() {
    NotamSnapshot s;
    s.snapshot_id = "NOTAM-SNAP-1";
    s.source_id = "AUTH-NOTAM";
    s.source_version = "2026-09-24T12:00Z";
    s.evaluation_epoch_s = 1000;
    s.zones.push_back({"N-001", NotamRestrictionType::Prohibited, NotamGeometryType::Circle,
                       {60.0, 25.0}, 1000.0, {}, 0.0, 500.0, 900, 1100});
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

    // A segment crossing a circle must be rejected even when both endpoints are outside.
    r.waypoints = {{"A", {60.0, 24.98}, 100.0, true},
                   {"B", {60.0, 25.02}, 100.0, false}};
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
                       0.0, 500.0, 900, 1100};
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

    r = route();
    r.environment.airspace_snapshot_id.clear();
    auto missing = NotamProhibitedZoneValidator::validate(r, snapshot());
    assert(missing.status == RouteValidationStatus::Rejected);
    assert(missing.findings.front().code == NotamValidationCode::MissingSnapshotReference);
}
