#!/usr/bin/env python3

from conflict_4d import SeparationMinimums
from conflict_resolution import RevalidationReport, ResolutionPolicy, resolve_conflicts as resolve
from route_in_zone import build_route_in_zone
from trajectory_4d import build_trajectory_4d
from wind_performance import PerformanceProfile, WindSample, adjust_route_for_wind
from zone_partition import Point, Polygon, Zone


def make_trajectory(uav_id: str, y: float, start: float, altitude: float):
    zone = Zone(
        f"ZONE-{uav_id}",
        Polygon((Point(0, 0), Point(100, 0), Point(100, 100), Point(0, 100))),
    )
    route = build_route_in_zone(
        route_id=f"ROUTE-{uav_id}",
        uav_id=uav_id,
        zone=zone,
        points=(Point(30, y), Point(70, y)),
    )
    performance = adjust_route_for_wind(
        route,
        WindSample(0, 0),
        PerformanceProfile(10, 0.01, 100, 12),
    )
    return build_trajectory_4d(
        trajectory_id=f"TRAJ-{uav_id}",
        route_id=route.route_id,
        uav_id=uav_id,
        route_points=route.points,
        performance=performance,
        start_time_s=start,
        altitude_m=altitude,
    )


def make_custom_trajectory(
    uav_id: str,
    points: tuple[Point, ...],
    speed_mps: float = 15,
):
    zone = Zone(
        f"ZONE-{uav_id}",
        Polygon((Point(-20, -20), Point(50, -20), Point(50, 50), Point(-20, 50))),
    )
    route = build_route_in_zone(
        route_id=f"ROUTE-{uav_id}",
        uav_id=uav_id,
        zone=zone,
        points=points,
    )
    performance = adjust_route_for_wind(
        route,
        WindSample(0, 0),
        PerformanceProfile(speed_mps, 0.01, 100, 12),
    )
    return build_trajectory_4d(
        trajectory_id=f"TRAJ-{uav_id}",
        route_id=route.route_id,
        uav_id=uav_id,
        route_points=route.points,
        performance=performance,
        start_time_s=0,
        altitude_m=80,
    )

def main() -> int:
    minimums = SeparationMinimums(10, 10)
    # Multi-conflict case: UAV-01 crosses UAV-02 and UAV-03 at different
    # points. A 0.5 s delay of either crossing UAV is sufficient for that
    # pair, but resolving both conflicts requires a combination affecting
    # UAV-02 and UAV-03. The search must not assume only conflict.uav_b moves.
    multi = (
        make_custom_trajectory(
            "UAV-01",
            (Point(0, 20), Point(20, 20), Point(40, 20)),
        ),
        make_custom_trajectory(
            "UAV-02",
            (Point(10, 7), Point(10, 47)),
        ),
        make_custom_trajectory(
            "UAV-03",
            (Point(30, -13), Point(30, 27)),
        ),
    )
    combination = resolve(
        multi,
        minimums,
        ResolutionPolicy(max_delay_s=0.5, delay_step_s=0.5),
        candidate_validator=lambda candidate: RevalidationReport(
            "PASS",
            passed_checks=(
                "ALTITUDE_LIMITS","AIRSPACE_RESTRICTIONS","TERRAIN_CLEARANCE",
                "OBSTACLE_CLEARANCE","UAV_CAPABILITY","CLIMB_DESCENT_RATE",
                "PERFORMANCE_ENERGY","MANDATORY_POINTS","GEOFENCE",
                "MISSION_GEOMETRY","MULTI_UAV_CONFLICT"
            ),
            model_ids=("MODEL-001",),
            input_snapshot_id="IN-001",
            candidate_snapshot_id="CAND-001",
            provenance="TEST",
        ),
    )
    assert combination.status == "RESOLVED"
    assert combination.method == "TEMPORAL_DELAY_COMBINATION"
    assert combination.delay_assignments == (
        ("UAV-01", 0.5),
        ("UAV-02", 0.5),
    )
    assert combination.conflict_report.status == "NO_CONFLICT"


    trajectories = (
        make_trajectory("UAV-01", 50, 0, 80),
        make_trajectory("UAV-02", 50, 0, 80),
    )

    result = resolve(
        trajectories,
        minimums,
        ResolutionPolicy(max_delay_s=5),
        candidate_validator=lambda candidate: RevalidationReport("PASS", passed_checks=(
                "ALTITUDE_LIMITS","AIRSPACE_RESTRICTIONS","TERRAIN_CLEARANCE",
                "OBSTACLE_CLEARANCE","UAV_CAPABILITY","CLIMB_DESCENT_RATE",
                "PERFORMANCE_ENERGY","MANDATORY_POINTS","GEOFENCE",
                "MISSION_GEOMETRY","MULTI_UAV_CONFLICT"
            ), model_ids=("MODEL-001",), input_snapshot_id="IN-001", candidate_snapshot_id="CAND-001", provenance="TEST"),
    )
    assert result.status == "RESOLVED"
    assert result.method == "TEMPORAL_DELAY"
    assert 0 < result.delay_s <= 5
    assert result.conflict_report.status == "NO_CONFLICT"

    incomplete_evidence = resolve(
        trajectories,
        minimums,
        ResolutionPolicy(max_delay_s=5),
        candidate_validator=lambda candidate: RevalidationReport("PASS"),
    )
    assert incomplete_evidence.status == "UNRESOLVED"
    assert "TEMPORAL_DELAY:AUTHORITATIVE_REVALIDATION_FAILED" in incomplete_evidence.rejected_candidates

    blocked = resolve(
        trajectories,
        minimums,
        ResolutionPolicy(max_delay_s=0),
        candidate_validator=lambda candidate: RevalidationReport("PASS", passed_checks=(
                "ALTITUDE_LIMITS","AIRSPACE_RESTRICTIONS","TERRAIN_CLEARANCE",
                "OBSTACLE_CLEARANCE","UAV_CAPABILITY","CLIMB_DESCENT_RATE",
                "PERFORMANCE_ENERGY","MANDATORY_POINTS","GEOFENCE",
                "MISSION_GEOMETRY","MULTI_UAV_CONFLICT"
            ), model_ids=("MODEL-001",), input_snapshot_id="IN-001", candidate_snapshot_id="CAND-001", provenance="TEST"),
    )
    assert blocked.status == "UNRESOLVED"
    assert blocked.conflict_report.status == "CONFLICT"

    vertical = resolve(
        trajectories,
        minimums,
        ResolutionPolicy(
            max_delay_s=0,
            allow_vertical_correction=True,
            vertical_correction_m=20,
        ),
        candidate_validator=lambda candidate: RevalidationReport("PASS", passed_checks=(
                "ALTITUDE_LIMITS","AIRSPACE_RESTRICTIONS","TERRAIN_CLEARANCE",
                "OBSTACLE_CLEARANCE","UAV_CAPABILITY","CLIMB_DESCENT_RATE",
                "PERFORMANCE_ENERGY","MANDATORY_POINTS","GEOFENCE",
                "MISSION_GEOMETRY","MULTI_UAV_CONFLICT"
            ), model_ids=("MODEL-001",), input_snapshot_id="IN-001", candidate_snapshot_id="CAND-001", provenance="TEST"),
    )
    assert vertical.status == "RESOLVED"
    assert vertical.method == "VERTICAL_CORRECTION"
    assert vertical.conflict_report.status == "NO_CONFLICT"

    no_action = resolve(
        (
            make_trajectory("UAV-01", 20, 0, 80),
            make_trajectory("UAV-02", 80, 0, 80),
        ),
        minimums,
        ResolutionPolicy(),
        candidate_validator=lambda candidate: RevalidationReport("PASS", passed_checks=(
                "ALTITUDE_LIMITS","AIRSPACE_RESTRICTIONS","TERRAIN_CLEARANCE",
                "OBSTACLE_CLEARANCE","UAV_CAPABILITY","CLIMB_DESCENT_RATE",
                "PERFORMANCE_ENERGY","MANDATORY_POINTS","GEOFENCE",
                "MISSION_GEOMETRY","MULTI_UAV_CONFLICT"
            ), model_ids=("MODEL-001",), input_snapshot_id="IN-001", candidate_snapshot_id="CAND-001", provenance="TEST"),
    )
    assert no_action.status == "NO_ACTION"

    print("CONFLICT RESOLUTION TESTS: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
