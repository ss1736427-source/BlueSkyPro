#!/usr/bin/env python3

from route_in_zone import build_route_in_zone
from trajectory_4d import TrajectoryError, build_trajectory_4d
from wind_performance import PerformanceProfile, WindSample, adjust_route_for_wind
from zone_partition import Point, Polygon, Zone


def main() -> int:
    zone = Zone(
        "ZONE-01",
        Polygon((Point(0, 0), Point(100, 0), Point(100, 50), Point(0, 50))),
    )
    route = build_route_in_zone(
        route_id="ROUTE-01",
        uav_id="UAV-01",
        zone=zone,
        points=(Point(10, 25), Point(60, 25), Point(90, 25)),
    )
    performance = adjust_route_for_wind(
        route,
        WindSample(0, 0),
        PerformanceProfile(10, 0.01, 100, 12),
    )

    trajectory = build_trajectory_4d(
        trajectory_id="TRAJ-01",
        route_id=route.route_id,
        uav_id=route.uav_id,
        route_points=route.points,
        performance=performance,
        start_time_s=100,
        altitude_m=80,
    )

    assert trajectory.verified is True
    assert trajectory.start_time_s == 100
    assert trajectory.end_time_s > 100
    assert all(
        left.timestamp_s < right.timestamp_s
        for left, right in zip(trajectory.points, trajectory.points[1:])
    )
    assert all(point.altitude_m == 80 for point in trajectory.points)

    try:
        build_trajectory_4d(
            trajectory_id="TRAJ-02",
            route_id=route.route_id,
            uav_id=route.uav_id,
            route_points=route.points,
            performance=performance,
            start_time_s=-1,
            altitude_m=80,
        )
    except TrajectoryError as exc:
        assert str(exc) == "INVALID_START_TIME"
    else:
        raise AssertionError("negative start time must be rejected")

    print("4D TRAJECTORY TESTS: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
