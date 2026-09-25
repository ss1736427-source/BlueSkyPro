"""Constrained Open Space geometry contracts.

This module models usable operational space as an operational polygon minus
excluded polygons and exposes deterministic connected components for later
zone partitioning. It does not perform full GIS polygon clipping.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence

from zone_partition import EPSILON, Point, Polygon, _point_in_polygon


@dataclass(frozen=True)
class Exclusion:
    exclusion_id: str
    geometry: Polygon
    reason: str


@dataclass(frozen=True)
class ConstrainedComponent:
    component_id: str
    geometry: Polygon
    source_exclusions: tuple[str, ...] = ()


@dataclass(frozen=True)
class ConstrainedOpenSpace:
    source: Polygon
    exclusions: tuple[Exclusion, ...]
    components: tuple[ConstrainedComponent, ...]
    status: str


class ConstrainedSpaceError(ValueError):
    pass


def _bounds(polygon: Polygon) -> tuple[float, float, float, float]:
    xs = [p.x for p in polygon.points]
    ys = [p.y for p in polygon.points]
    return min(xs), min(ys), max(xs), max(ys)


def _strictly_inside(point: Point, polygon: Polygon) -> bool:
    return _point_in_polygon(point, polygon)


def build_constrained_space(
    source: Polygon,
    exclusions: Sequence[Exclusion],
) -> ConstrainedOpenSpace:
    """Validate exclusions and build deterministic component metadata.

    Polygon clipping is intentionally not attempted here. If exclusions
    fragment the space, the caller must provide/derive component geometry
    using a future production geometry engine. For the current contract layer,
    the no-exclusion case is fully executable.
    """
    for exclusion in exclusions:
        if exclusion.geometry.area <= EPSILON:
            raise ConstrainedSpaceError(
                f"CONSTRAINED_SPACE_INVALID: zero-area exclusion {exclusion.exclusion_id}"
            )
        if not all(_strictly_inside(p, source) for p in exclusion.geometry.points):
            raise ConstrainedSpaceError(
                f"CONSTRAINED_SPACE_INVALID: exclusion {exclusion.exclusion_id} outside source"
            )

    if not exclusions:
        return ConstrainedOpenSpace(
            source=source,
            exclusions=(),
            components=(
                ConstrainedComponent(
                    component_id="COMP-01",
                    geometry=source,
                ),
            ),
            status="VERIFIED",
        )

    # Contract-safe state: exclusions are recorded, but resulting clipped
    # components are not fabricated. A production clipping engine is required.
    raise ConstrainedSpaceError(
        "CONSTRAINED_SPACE_REQUIRES_GEOMETRY_ENGINE: exclusions present"
    )
