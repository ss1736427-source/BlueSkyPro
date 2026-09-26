#!/usr/bin/env python3

from conflict_4d import SeparationMinimums, verify_fleet
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
    minimums = SeparationMinimums(horizontal_m=10, vertical_m=10)

    separated = verify_fleet(
        (
            make_trajectory("UAV-01", 20, 0, 80),
            make_trajectory("UAV-02", 80, 0, 80),
        ),
        minimums,
    )
    assert separated.status == "NO_CONFLICT"
    assert separated.checked_pairs == 1

    temporal = verify_fleet(
        (
            make_trajectory("UAV-01", 50, 0, 80),
            make_trajectory("UAV-02", 50, 20, 80),
        ),
        minimums,
    )
    assert temporal.status == "NO_CONFLICT"

    vertical = verify_fleet(
        (
            make_trajectory("UAV-01", 50, 0, 80),
            make_trajectory("UAV-02", 50, 0, 100),
        ),
        minimums,
    )
    assert vertical.status == "NO_CONFLICT"

    conflict = verify_fleet(
        (
            make_trajectory("UAV-01", 50, 0, 80),
            make_trajectory("UAV-02", 50, 0, 80),
        ),
        minimums,
    )
    assert conflict.status == "CONFLICT"
    assert len(conflict.conflicts) >= 1

    print("4D CONFLICT TESTS: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
