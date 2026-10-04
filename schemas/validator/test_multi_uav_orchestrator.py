#!/usr/bin/env python3

from conflict_4d import SeparationMinimums
from conflict_resolution import ResolutionPolicy
from multi_uav_orchestrator import PipelineInputs, run_multi_uav_pipeline
from route_in_zone import build_route_in_zone
from trajectory_4d import build_trajectory_4d
from wind_performance import PerformanceProfile, WindSample, adjust_route_for_wind
from zone_partition import Point, Polygon, Zone


def make_plan(uav_id: str, y: float, start: float = 0):
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
    trajectory = build_trajectory_4d(
        trajectory_id=f"TRAJ-{uav_id}",
        route_id=route.route_id,
        uav_id=uav_id,
        route_points=route.points,
        performance=performance,
        start_time_s=start,
        altitude_m=80,
    )
    return route, performance, trajectory


def main() -> int:
    first = make_plan("UAV-01", 20)
    second = make_plan("UAV-02", 80)

    success = run_multi_uav_pipeline(
        PipelineInputs(
            "VERIFIED",
            "VERIFIED",
            (first[0], second[0]),
            (first[1], second[1]),
            (first[2], second[2]),
        ),
        SeparationMinimums(10, 10),
        ResolutionPolicy(max_delay_s=5),
    )
    assert success.status == "RELEASE_ELIGIBLE"
    assert success.final_gate.status == "PASS"
    assert success.conflict.status == "NO_CONFLICT"

    conflict_first = make_plan("UAV-01", 50)
    conflict_second = make_plan("UAV-02", 50)

    resolved = run_multi_uav_pipeline(
        PipelineInputs(
            "VERIFIED",
            "VERIFIED",
            (conflict_first[0], conflict_second[0]),
            (conflict_first[1], conflict_second[1]),
            (conflict_first[2], conflict_second[2]),
        ),
        SeparationMinimums(10, 10),
        ResolutionPolicy(max_delay_s=5),
    )
    assert resolved.status == "RELEASE_ELIGIBLE"
    assert resolved.resolution.status == "RESOLVED"
    assert resolved.resolution.delay_s <= 5
    assert resolved.final_gate.status == "PASS"

    blocked = run_multi_uav_pipeline(
        PipelineInputs(
            "VERIFIED",
            "VERIFIED",
            (conflict_first[0], conflict_second[0]),
            (conflict_first[1], conflict_second[1]),
            (conflict_first[2], conflict_second[2]),
        ),
        SeparationMinimums(10, 10),
        ResolutionPolicy(max_delay_s=0),
    )
    assert blocked.status == "BLOCKED"
    assert blocked.final_gate.status == "FAIL"

    print("MULTI-UAV ORCHESTRATOR TESTS: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
