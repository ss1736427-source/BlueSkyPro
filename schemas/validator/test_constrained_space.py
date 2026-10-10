#!/usr/bin/env python3

from __future__ import annotations

import sys

from constrained_space import (
    ConstrainedSpaceError,
    Exclusion,
    build_constrained_space,
)
from zone_partition import Point, Polygon


def main() -> int:
    source = Polygon((
        Point(0, 0), Point(100, 0), Point(100, 50), Point(0, 50)
    ))

    result = build_constrained_space(source, ())
    assert result.status == "VERIFIED"
    assert len(result.components) == 1
    assert result.components[0].component_id == "COMP-01"
    assert result.components[0].geometry.area == source.area

    exclusion = Exclusion(
        exclusion_id="NO-GO-01",
        geometry=Polygon((
            Point(20, 20), Point(30, 20), Point(30, 30), Point(20, 30)
        )),
        reason="restricted area",
    )

    try:
        build_constrained_space(source, (exclusion,))
    except ConstrainedSpaceError as exc:
        assert "polygon subtraction requires a vetted clipping backend" in str(exc)
    else:
        raise AssertionError("clipped geometry must not be fabricated")

    outside = Exclusion(
        exclusion_id="NO-GO-02",
        geometry=Polygon((
            Point(90, 40), Point(110, 40), Point(110, 45), Point(90, 45)
        )),
        reason="invalid exclusion",
    )

    try:
        build_constrained_space(source, (outside,))
    except ConstrainedSpaceError:
        pass
    else:
        raise AssertionError("outside exclusion must fail")

    print("CONSTRAINED SPACE TESTS: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
