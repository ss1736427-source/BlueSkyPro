#!/usr/bin/env python3

from __future__ import annotations

import sys

from geometry_engine import ContractGeometryEngine
from zone_partition import Point, Polygon


def main() -> int:
    engine = ContractGeometryEngine()

    source = Polygon((
        Point(0, 0), Point(100, 0), Point(100, 100), Point(0, 100)
    ))
    inner = Polygon((
        Point(10, 10), Point(40, 10), Point(40, 40), Point(10, 40)
    ))
    crossing = Polygon((
        Point(90, 90), Point(110, 90), Point(110, 110), Point(90, 110)
    ))

    assert engine.contains(source, inner)
    assert not engine.contains(source, crossing)
    assert not engine.interiors_overlap(inner, Polygon((
        Point(40, 10), Point(70, 10), Point(70, 40), Point(40, 40)
    )))

    components = engine.subtract(source, ())
    assert len(components) == 1
    assert components[0].component_id == "COMP-01"

    try:
        engine.subtract(source, (inner,))
    except NotImplementedError:
        pass
    else:
        raise AssertionError("unsupported clipping must fail explicitly")

    print("GEOMETRY ENGINE TESTS: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
