#!/usr/bin/env python3

from route_in_zone import build_route_in_zone
from wind_performance import (
    PerformanceError,
    PerformanceProfile,
    WindSample,
    adjust_route_for_wind,
)
from zone_partition import Point, Polygon, Zone


def make_route():
    zone = Zone(
        "ZONE-01",
        Polygon((Point(0, 0), Point(100, 0), Point(100, 50), Point(0, 50))),
    )
    return build_route_in_zone(
        route_id="ROUTE-01",
        uav_id="UAV-01",
        zone=zone,
        points=(Point(10, 25), Point(90, 25)),
    )


def main() -> int:
    route = make_route()
    profile = PerformanceProfile(
        cruise_speed_mps=20,
        energy_per_meter_wh=0.05,
        reserve_wh=10,
        max_wind_mps=12,
    )

    headwind = adjust_route_for_wind(
        route,
        WindSample(wind_speed_mps=5, wind_from_deg=180),
        profile,
    )
    tailwind = adjust_route_for_wind(
        route,
        WindSample(wind_speed_mps=5, wind_from_deg=0),
        profile,
    )

    assert headwind.verified is True
    assert headwind.total_energy_wh == tailwind.total_energy_wh
    assert headwind.segments[0].ground_speed_mps < tailwind.segments[0].ground_speed_mps

    low_reserve = adjust_route_for_wind(
        route,
        WindSample(wind_speed_mps=0, wind_from_deg=0),
        PerformanceProfile(20, 0.2, 5, 12),
    )
    assert low_reserve.verified is False
    assert low_reserve.reserve_margin_wh < 0

    try:
        adjust_route_for_wind(
            route,
            WindSample(13, 0),
            profile,
        )
    except PerformanceError as exc:
        assert str(exc) == "WIND_LIMIT_EXCEEDED"
    else:
        raise AssertionError("wind limit must block performance calculation")

    print("WIND/PERFORMANCE TESTS: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
