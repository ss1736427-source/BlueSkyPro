#!/usr/bin/env python3
"""Tests for the deterministic baseline Zone Partition Engine."""

from __future__ import annotations

import sys

from zone_partition import Rectangle, Zone, ZonePartitionError, partition_rectangle, verify_zone_set


def main() -> int:
    source = Rectangle(0, 0, 100, 40)

    result = partition_rectangle(source, 2)
    assert result.status == "VERIFIED"
    assert len(result.zones) == 2
    assert result.coverage_area == source.area

    left, right = result.zones
    assert left.geometry.max_x == right.geometry.min_x

    try:
        verify_zone_set(
            source,
            (
                Zone("ZONE-01", Rectangle(0, 0, 60, 40)),
                Zone("ZONE-02", Rectangle(50, 0, 100, 40)),
            ),
        )
    except ZonePartitionError:
        pass
    else:
        raise AssertionError("overlapping zones must fail")

    try:
        verify_zone_set(
            source,
            (
                Zone("ZONE-01", Rectangle(0, 0, 40, 40)),
                Zone("ZONE-02", Rectangle(40, 0, 90, 40)),
            ),
        )
    except ZonePartitionError:
        pass
    else:
        raise AssertionError("incomplete coverage must fail")

    try:
        verify_zone_set(
            source,
            (Zone("ZONE-01", Rectangle(-1, 0, 50, 40)), Zone("ZONE-02", Rectangle(50, 0, 100, 40))),
        )
    except ZonePartitionError:
        pass
    else:
        raise AssertionError("zone outside constrained space must fail")

    print("ZONE PARTITION TESTS: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
