#!/usr/bin/env python3

from __future__ import annotations

import sys

from uav_assignment import AssignmentError, UAVCapability, assign_zones
from zone_partition import Point, Polygon, Zone


def zone(zone_id: str, x0: float, x1: float) -> Zone:
    return Zone(zone_id, Polygon((
        Point(x0, 0), Point(x1, 0), Point(x1, 10), Point(x0, 10)
    )))


def main() -> int:
    zones = (zone("ZONE-01", 0, 10), zone("ZONE-02", 10, 20))
    fleet = (
        UAVCapability("UAV-01", True, True, 1200, 600, 300, True, True, 5, 2),
        UAVCapability("UAV-02", True, True, 1500, 600, 300, True, True, 3, 1),
    )

    result = assign_zones(zones, fleet)
    assert result.status == "VERIFIED"
    assert len(result.assignments) == 2
    assert len({a.uav_id for a in result.assignments}) == 2

    blocked = (
        UAVCapability("UAV-01", True, True, 700, 600, 200, True, True),
    )
    try:
        assign_zones((zones[0],), blocked)
    except AssignmentError as exc:
        assert "no feasible UAV" in str(exc)
    else:
        raise AssertionError("insufficient reserve must block assignment")

    unauthorized = (
        UAVCapability("UAV-01", True, True, 1200, 600, 300, True, False),
    )
    try:
        assign_zones((zones[0],), unauthorized)
    except AssignmentError:
        pass
    else:
        raise AssertionError("unauthorized UAV must not be assigned")

    print("UAV ASSIGNMENT TESTS: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
