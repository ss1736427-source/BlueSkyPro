#!/usr/bin/env python3
"""Regression tests for contract and Shapely-backed geometry engines."""

from __future__ import annotations

import sys

from constrained_space import (
    ConstrainedSpaceError,
    Exclusion,
    build_constrained_space,
)
from geometry_engine import ContractGeometryEngine, ShapelyGeometryEngine
from route_in_zone import RouteInZoneError, build_route_in_zone
from zone_partition import Point, Polygon, Zone


def rectangle(x0: float, y0: float, x1: float, y1: float) -> Polygon:
    return Polygon((
        Point(x0, y0),
        Point(x1, y0),
        Point(x1, y1),
        Point(x0, y1),
    ))


def test_contract_engine_fails_closed_without_clipping_backend() -> None:
    engine = ContractGeometryEngine()
    source = rectangle(0, 0, 100, 100)
    inner = rectangle(10, 10, 40, 40)
    crossing = rectangle(90, 90, 110, 110)

    assert engine.contains(source, inner)
    assert not engine.contains(source, crossing)
    assert not engine.interiors_overlap(inner, rectangle(40, 10, 70, 40))

    components = engine.subtract(source, ())
    assert len(components) == 1
    assert components[0].component_id == "COMP-01"

    try:
        engine.subtract(source, (inner,))
    except NotImplementedError:
        pass
    else:
        raise AssertionError("unsupported clipping must fail explicitly")


def test_interior_exclusion_is_removed_and_area_is_preserved() -> None:
    source = rectangle(0, 0, 100, 50)
    exclusion = Exclusion(
        exclusion_id="NO-GO-INTERIOR",
        geometry=rectangle(20, 10, 40, 30),
        reason="restricted area",
    )
    engine = ShapelyGeometryEngine()

    result = build_constrained_space(source, (exclusion,), engine=engine)

    assert result.status == "VERIFIED"
    assert len(result.components) == 1
    component = result.components[0].geometry
    assert len(component.holes) == 1
    assert abs(component.area - (source.area - exclusion.geometry.area)) < 1e-9
    assert not engine.interiors_overlap(component, exclusion.geometry)

    # The clipped geometry must also reject a route segment that crosses its hole,
    # even when both segment endpoints are inside the operational polygon.
    zone = Zone(result.components[0].component_id, component)
    try:
        build_route_in_zone(
            route_id="ROUTE-CROSSES-NO-GO",
            uav_id="UAV-01",
            zone=zone,
            points=(Point(10, 20), Point(50, 20)),
        )
    except RouteInZoneError as exc:
        assert str(exc) == "ROUTE_GEOMETRY_OUTSIDE_ZONE"
    else:
        raise AssertionError("route crossing a restricted hole must be rejected")

    safe_route = build_route_in_zone(
        route_id="ROUTE-AROUND-NO-GO",
        uav_id="UAV-01",
        zone=zone,
        points=(
            Point(10, 20), Point(10, 5), Point(50, 5), Point(50, 20)
        ),
    )
    assert safe_route.verified is True


def test_boundary_to_boundary_exclusion_splits_space_deterministically() -> None:
    source = rectangle(0, 0, 100, 50)
    exclusion = Exclusion(
        exclusion_id="NO-GO-CORRIDOR",
        geometry=rectangle(45, 0, 55, 50),
        reason="restricted corridor",
    )
    engine = ShapelyGeometryEngine()

    first = build_constrained_space(source, (exclusion,), engine=engine)
    second = build_constrained_space(source, (exclusion,), engine=engine)

    assert first.status == second.status == "VERIFIED"
    assert len(first.components) == 2
    assert [item.component_id for item in first.components] == ["COMP-01", "COMP-02"]
    assert [item.component_id for item in first.components] == [
        item.component_id for item in second.components
    ]
    assert [item.geometry.area for item in first.components] == [
        item.geometry.area for item in second.components
    ]
    assert abs(
        sum(item.geometry.area for item in first.components)
        - (source.area - exclusion.geometry.area)
    ) < 1e-9
    assert not engine.interiors_overlap(
        first.components[0].geometry, first.components[1].geometry
    )


def test_exclusion_outside_source_is_rejected() -> None:
    source = rectangle(0, 0, 100, 50)
    exclusion = Exclusion(
        exclusion_id="NO-GO-OUTSIDE",
        geometry=rectangle(90, 40, 110, 45),
        reason="invalid input",
    )

    try:
        build_constrained_space(
            source, (exclusion,), engine=ShapelyGeometryEngine()
        )
    except ConstrainedSpaceError as exc:
        assert "CONSTRAINED_SPACE_INVALID" in str(exc)
        assert "outside source" in str(exc)
    else:
        raise AssertionError("exclusion outside the source area must be rejected")


def test_exclusion_that_removes_all_operational_space_is_rejected() -> None:
    source = rectangle(0, 0, 100, 50)
    exclusion = Exclusion(
        exclusion_id="NO-GO-ALL",
        geometry=rectangle(0, 0, 100, 50),
        reason="no operational area remains",
    )

    try:
        build_constrained_space(
            source, (exclusion,), engine=ShapelyGeometryEngine()
        )
    except ConstrainedSpaceError as exc:
        assert "no usable operational space" in str(exc)
    else:
        raise AssertionError("empty operational space must not be verified")


def main() -> int:
    tests = (
        test_contract_engine_fails_closed_without_clipping_backend,
        test_interior_exclusion_is_removed_and_area_is_preserved,
        test_boundary_to_boundary_exclusion_splits_space_deterministically,
        test_exclusion_outside_source_is_rejected,
        test_exclusion_that_removes_all_operational_space_is_rejected,
    )
    for test in tests:
        test()
    print(f"GEOMETRY ENGINE TESTS: {len(tests)}/{len(tests)} PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
