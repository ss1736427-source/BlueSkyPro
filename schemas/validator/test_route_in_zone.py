#!/usr/bin/env python3

from route_in_zone import RouteInZoneError, build_route_in_zone
from zone_partition import Point, Polygon, Zone


def main() -> int:
    zone = Zone(
        "ZONE-01",
        Polygon((
            Point(0, 0), Point(100, 0), Point(100, 100), Point(0, 100)
        )),
    )

    route = build_route_in_zone(
        route_id="ROUTE-01",
        uav_id="UAV-01",
        zone=zone,
        points=(Point(10, 10), Point(90, 10), Point(90, 90)),
    )
    assert route.verified is True
    assert route.length > 0

    try:
        build_route_in_zone(
            route_id="ROUTE-02",
            uav_id="UAV-01",
            zone=zone,
            points=(Point(10, 10), Point(110, 10)),
        )
    except RouteInZoneError as exc:
        assert "OUTSIDE_ZONE" in str(exc)
    else:
        raise AssertionError("route outside zone must be rejected")

    concave = Zone(
        "ZONE-02",
        Polygon((
            Point(0, 0), Point(100, 0), Point(100, 40),
            Point(40, 40), Point(40, 100), Point(0, 100)
        )),
    )
    try:
        build_route_in_zone(
            route_id="ROUTE-03",
            uav_id="UAV-02",
            zone=concave,
            points=(Point(10, 10), Point(90, 90)),
        )
    except RouteInZoneError:
        pass
    else:
        raise AssertionError("segment crossing outside a concave zone must be rejected")

    print("ROUTE-IN-ZONE TESTS: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
