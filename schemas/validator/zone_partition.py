"""Deterministic Zone Partition geometry contracts."""

from __future__ import annotations
from dataclasses import dataclass
from typing import Sequence

EPSILON = 1e-9

@dataclass(frozen=True)
class Point:
    x: float
    y: float

@dataclass(frozen=True)
class Polygon:
    points: tuple[Point, ...]

    def __post_init__(self) -> None:
        if len(self.points) < 3:
            raise ValueError("polygon requires at least three points")

    @property
    def area(self) -> float:
        return abs(sum(
            p.x * self.points[(i + 1) % len(self.points)].y
            - self.points[(i + 1) % len(self.points)].x * p.y
            for i, p in enumerate(self.points)
        )) / 2.0

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

    def polygon(self) -> Polygon:
        return Polygon((
            Point(self.min_x, self.min_y), Point(self.max_x, self.min_y),
            Point(self.max_x, self.max_y), Point(self.min_x, self.max_y),
        ))

@dataclass(frozen=True)
class Zone:
    zone_id: str
    geometry: Polygon | Rectangle
    uav_id: str | None = None

@dataclass(frozen=True)
class ZoneSet:
    zones: tuple[Zone, ...]
    coverage_area: float
    source_area: float
    status: str

class ZonePartitionError(ValueError):
    pass

def _polygon(g: Polygon | Rectangle) -> Polygon:
    return g if isinstance(g, Polygon) else g.polygon()

def _cross(a: Point, b: Point, c: Point) -> float:
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x)

def _on_segment(a: Point, b: Point, p: Point) -> bool:
    return (abs(_cross(a,b,p)) <= EPSILON and
            min(a.x,b.x)-EPSILON <= p.x <= max(a.x,b.x)+EPSILON and
            min(a.y,b.y)-EPSILON <= p.y <= max(a.y,b.y)+EPSILON)

def _segments_intersect(a: Point,b: Point,c: Point,d: Point) -> bool:
    c1,c2,c3,c4 = _cross(a,b,c),_cross(a,b,d),_cross(c,d,a),_cross(c,d,b)
    proper = ((c1 > EPSILON and c2 < -EPSILON) or (c1 < -EPSILON and c2 > EPSILON)) and ((c3 > EPSILON and c4 < -EPSILON) or (c3 < -EPSILON and c4 > EPSILON))
    return proper or _on_segment(a,b,c) or _on_segment(a,b,d) or _on_segment(c,d,a) or _on_segment(c,d,b)

def _point_in_polygon(point: Point, polygon: Polygon) -> bool:
    inside = False
    for i, start in enumerate(polygon.points):
        end = polygon.points[(i+1) % len(polygon.points)]
        if _on_segment(start,end,point):
            return True
        if (start.y > point.y) != (end.y > point.y):
            x = (end.x-start.x)*(point.y-start.y)/(end.y-start.y)+start.x
            if point.x < x:
                inside = not inside
    return inside

def _contains(outer: Polygon, inner: Polygon) -> bool:
    return all(_point_in_polygon(p, outer) for p in inner.points)

def _interior_overlap(a: Polygon, b: Polygon) -> bool:
    for i, a0 in enumerate(a.points):
        a1 = a.points[(i+1) % len(a.points)]
        for j, b0 in enumerate(b.points):
            b1 = b.points[(j+1) % len(b.points)]
            if _segments_intersect(a0,a1,b0,b1):
                # Boundary-only contact is allowed. A proper crossing is overlap.
                c1,c2,c3,c4 = _cross(a0,a1,b0),_cross(a0,a1,b1),_cross(b0,b1,a0),_cross(b0,b1,a1)
                if ((c1 > EPSILON and c2 < -EPSILON) or (c1 < -EPSILON and c2 > EPSILON)) and ((c3 > EPSILON and c4 < -EPSILON) or (c3 < -EPSILON and c4 > EPSILON)):
                    return True
    return _point_in_polygon(a.points[0],b) or _point_in_polygon(b.points[0],a)

def verify_zone_set(source: Polygon | Rectangle, zones: Sequence[Zone], tolerance: float = EPSILON) -> ZoneSet:
    outer = _polygon(source)
    if not zones:
        raise ZonePartitionError("PARTITION_FAILED: no zones")
    for zone in zones:
        inner = _polygon(zone.geometry)
        if inner.area <= tolerance:
            raise ZonePartitionError(f"PARTITION_FAILED: zero-area zone {zone.zone_id}")
        if not _contains(outer, inner):
            raise ZonePartitionError(f"PARTITION_FAILED: {zone.zone_id} outside constrained open space")
    for i, left in enumerate(zones):
        for right in zones[i+1:]:
            if _interior_overlap(_polygon(left.geometry), _polygon(right.geometry)):
                raise ZonePartitionError("PARTITION_FAILED: zone interior overlap")
    total = sum(_polygon(z.geometry).area for z in zones)
    if abs(total - outer.area) > tolerance:
        raise ZonePartitionError(f"PARTITION_FAILED: incomplete coverage: zones={total}, source={outer.area}")
    return ZoneSet(tuple(zones), total, outer.area, "VERIFIED")

def partition_rectangle(source: Rectangle, zone_count: int) -> ZoneSet:
    if zone_count < 1:
        raise ZonePartitionError("PARTITION_FAILED: zone_count must be positive")
    width = (source.max_x-source.min_x)/zone_count
    zones = []
    for i in range(zone_count):
        x0 = source.min_x+i*width
        x1 = source.max_x if i == zone_count-1 else source.min_x+(i+1)*width
        zones.append(Zone(f"ZONE-{i+1:02d}", Rectangle(x0,source.min_y,x1,source.max_y)))
    return verify_zone_set(source,zones)
