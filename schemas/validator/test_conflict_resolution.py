#!/usr/bin/env python3

from conflict_4d import SeparationMinimums, verify_fleet
from conflict_resolution import ResolutionPolicy, resolve_conflicts
from route_in_zone import build_route_in_zone
from trajectory_4d import build_trajectory_4d
from wind_performance import PerformanceProfile, WindSample, adjust_route_for_wind
from zone_partition import Point, Polygon, Zone


def make_trajectory(uav_id: str, y: float, start: float, altitude: float):
    zone = Zone(
        f"ZONE-{uav_id}",
        Polygon((Point(0, 0), Point(100, 0), Point(100, 0), Point(0, 100))),
    )
    # Use a valid rectangle after constructing the test geometry.
    zone = Zone(
        f"ZONE-{uav_id}",
        Polygon((Point(0, 0), Point(100, 0), Point(100, 100), Point(0, 100))),
    )
    route = build_route_in_zone(
        route_id=f"ROUTE-{uav_id}",
        uav_id=uav_id,
        zone=zone,
        points=(Point(10, y), Point(90, y)),
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


def main() -> int:
    minimums = SeparationMinimums(10, 10)
    trajectories = (
        make_trajectory("UAV-01", 50, 0, 80),
        make_trajectory("UAV-02", 50, 0, 80),
    )

    result = resolve_conflicts(
        trajectories,
        minimums,
        ResolutionPolicy(max_delay_s=5),
    )
    assert result.status == "RESOLVED"
    assert result.method == "TEMPORAL_DELAY"
    assert 0 < result.delay_s <= 5
    assert result.conflict_report.status == "NO_CONFLICT"

    blocked = resolve_conflicts(
        trajectories,
        minimums,
        ResolutionPolicy(max_delay_s=0),
    )
    assert blocked.status == "UNRESOLVED"
    assert blocked.conflict_report.status == "CONFLICT"

    vertical = resolve_conflicts(
        trajectories,
        minimums,
        ResolutionPolicy(
            max_delay_s=0,
            allow_vertical_correction=True,
            vertical_correction_m=20,
        ),
    )
    assert vertical.status == "RESOLVED"
    assert vertical.method == "VERTICAL_CORRECTION"
    assert vertical.conflict_report.status == "NO_CONFLICT"

    no_action = resolve_conflicts(
        (
            make_trajectory("UAV-01", 20, 0, 80),
            make_trajectory("UAV-02", 80, 0, 80),
        ),
        minimums,
        ResolutionPolicy(),
    )
    assert no_action.status == "NO_ACTION"

    print("CONFLICT RESOLUTION TESTS: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
