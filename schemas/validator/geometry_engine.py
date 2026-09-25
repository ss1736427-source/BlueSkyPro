"""Geometry-engine adapter boundary for BlueSky PRO.

The planning contracts depend on this interface, not on a concrete GIS
library. A backend may later be implemented with a vetted geometry library
without changing Zone Partition or Constrained Open Space contracts.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Protocol, Sequence

from zone_partition import Point, Polygon


@dataclass(frozen=True)
class GeometryComponent:
    component_id: str
    geometry: Polygon


class GeometryEngine(Protocol):
    def subtract(
        self,
        source: Polygon,
        exclusions: Sequence[Polygon],
    ) -> tuple[GeometryComponent, ...]:
        """Return deterministic connected usable components."""

    def contains(self, outer: Polygon, inner: Polygon) -> bool:
        """Return whether inner is fully contained by outer."""

    def interiors_overlap(self, left: Polygon, right: Polygon) -> bool:
        """Return whether polygon interiors overlap."""


class ContractGeometryEngine:
    """Dependency-free backend for contract-safe basic operations.

    Polygon subtraction is intentionally unsupported until a vetted clipping
    backend is selected. This prevents silent geometric approximation.
    """

    def contains(self, outer: Polygon, inner: Polygon) -> bool:
        from zone_partition import _contains
        return _contains(outer, inner)

    def interiors_overlap(self, left: Polygon, right: Polygon) -> bool:
        from zone_partition import _interior_overlap
        return _interior_overlap(left, right)

    def subtract(
        self,
        source: Polygon,
        exclusions: Sequence[Polygon],
    ) -> tuple[GeometryComponent, ...]:
        if not exclusions:
            return (GeometryComponent("COMP-01", source),)
        raise NotImplementedError(
            "polygon subtraction requires a vetted clipping backend"
        )
