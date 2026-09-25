"""Deterministic baseline Zone Partition Engine.

The service provides a small, dependency-free spatial contract layer for
rectangular operational areas. It is deliberately not a general polygon
optimizer. Its purpose is to establish executable zone-partition semantics
before adding production geometry libraries and optimization strategies.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence


@dataclass(frozen=True)
class Point:
    x: float
    y: float


@dataclass(frozen=True)
class Rectangle:
    min_x: float
    min_y: float
    max_x: float
    max_y: float

    def __post_init__(self) -> None:
        if self.max_x <= self.min_x or self.max_y <= self.min_y:
            raise ValueError("invalid rectangle")

    @property
    def area(self) -> float:
        return (self.max_x - self.min_x) * (self.max_y - self.min_y)


@dataclass(frozen=True)
class Zone:
    zone_id: str
    geometry: Rectangle
    uav_id: str | None = None


@dataclass(frozen=True)
class ZoneSet:
    zones: tuple[Zone, ...]
    coverage_area: float
    source_area: float
    status: str


class ZonePartitionError(ValueError):
    pass


def _overlap_area(a: Rectangle, b: Rectangle) -> float:
    width = max(0.0, min(a.max_x, b.max_x) - max(a.min_x, b.min_x))
    height = max(0.0, min(a.max_y, b.max_y) - max(a.min_y, b.min_y))
    return width * height


def verify_zone_set(source: Rectangle, zones: Sequence[Zone], tolerance: float = 1e-9) -> ZoneSet:
    if not zones:
        raise ZonePartitionError("PARTITION_FAILED: no zones")

    for zone in zones:
        g = zone.geometry
        if (
            g.min_x < source.min_x - tolerance
            or g.min_y < source.min_y - tolerance
            or g.max_x > source.max_x + tolerance
            or g.max_y > source.max_y + tolerance
        ):
            raise ZonePartitionError(
                f"PARTITION_FAILED: {zone.zone_id} outside constrained open space"
            )

    overlap = 0.0
    for i, left in enumerate(zones):
        for right in zones[i + 1 :]:
            overlap += _overlap_area(left.geometry, right.geometry)

    total = sum(z.geometry.area for z in zones)
    if overlap > tolerance:
        raise ZonePartitionError("PARTITION_FAILED: zone overlap is non-zero")

    if abs(total - source.area) > tolerance:
        raise ZonePartitionError(
            f"PARTITION_FAILED: incomplete coverage: zones={total}, source={source.area}"
        )

    return ZoneSet(
        zones=tuple(zones),
        coverage_area=total,
        source_area=source.area,
        status="VERIFIED",
    )


def partition_rectangle(source: Rectangle, zone_count: int) -> ZoneSet:
    """Split a rectangle into deterministic contiguous vertical sectors."""
    if zone_count < 1:
        raise ZonePartitionError("PARTITION_FAILED: zone_count must be positive")

    width = (source.max_x - source.min_x) / zone_count
    zones: list[Zone] = []

    for index in range(zone_count):
        min_x = source.min_x + index * width
        max_x = source.max_x if index == zone_count - 1 else source.min_x + (index + 1) * width
        zones.append(
            Zone(
                zone_id=f"ZONE-{index + 1:02d}",
                geometry=Rectangle(min_x, source.min_y, max_x, source.max_y),
            )
        )

    return verify_zone_set(source, zones)
