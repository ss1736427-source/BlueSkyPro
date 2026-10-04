#!/usr/bin/env python3

from __future__ import annotations

import sys

from polygon_splitter import PolygonSplitError, ShapelyPolygonSplitter
from zone_partition import Point, Polygon


def main() -> int:
    source = Polygon((
        Point(0, 0), Point(100, 0), Point(100, 60), Point(0, 60)
    ))

    splitter = ShapelyPolygonSplitter()
    result = splitter.split(source, 4)

    assert result.status == "VERIFIED"
    assert len(result.zones) == 4
    assert abs(sum(zone.geometry.area for zone in result.zones) - source.area) < 1e-9

    # Deterministic IDs/order and equal-area baseline.
    assert [z.zone_id for z in result.zones] == [
        "ZONE-01", "ZONE-02", "ZONE-03", "ZONE-04"
    ]
    assert all(abs(z.geometry.area - 1500.0) < 1e-9 for z in result.zones)

    one = splitter.split(source, 1)
    assert len(one.zones) == 1
    assert one.zones[0].geometry.area == source.area

    try:
        splitter.split(source, 0)
    except PolygonSplitError:
        pass
    else:
        raise AssertionError("zero split count must fail")

    print("POLYGON SPLITTER TESTS: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
