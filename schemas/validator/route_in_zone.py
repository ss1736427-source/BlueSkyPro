"""Route generation and containment verification for assigned UAV zones."""

from __future__ import annotations

from dataclasses import dataclass
from math import hypot
from typing import Sequence

from zone_partition import Point, Zone, Polygon, Rectangle


@dataclass(frozen=True)
class RoutePoint:
    point_id: str
    position: Point


@dataclass(frozen=True)
class Route:
    route_id: str
    uav_id: str
    zone_id: str
    points: tuple[RoutePoint, ...]
    length: float
    verified: bool


class RouteInZoneError(ValueError):
    pass


def _polygon(geometry: Polygon | Rectangle) -> Polygon:
    return geometry if isinstance(geometry, Polygon) else geometry.polygon()


def _route_length(points: Sequence[RoutePoint]) -> float:
    return sum(
        hypot(
            points[i + 1].position.x - points[i].position.x,
            points[i + 1].position.y - points[i].position.y,
        )
        for i in range(len(points) - 1)
    )


def _segments_inside_zone(route: Route, zone: Zone) -> bool:
    """Verify every route segment is contained, not only its vertices."""
    polygon = _polygon(zone.geometry)
    try:
        from shapely.geometry import LineString, Polygon as SPolygon

        area = SPolygon(
            [(p.x, p.y) for p in polygon.points],
            [[(p.x, p.y) for p in h] for h in polygon.holes],
        )
        return all(
            area.covers(LineString((
                (left.position.x, left.position.y),
                (right.position.x, right.position.y),
            )))
            for left, right in zip(route.points, route.points[1:])
        )
    except ImportError:
        return all(_point_in_polygon(p.position, polygon) for p in route.points)


def _point_in_polygon(point: Point, polygon: Polygon) -> bool:
    ring = polygon.points
    inside = False
    for i, start in enumerate(ring):
        end = ring[(i + 1) % len(ring)]
        if (start.y > point.y) != (end.y > point.y):
            x = (end.x - start.x) * (point.y - start.y) / (end.y - start.y) + start.x
            if point.x < x:
                inside = not inside
    return inside


def verify_route_in_zone(route: Route, zone: Zone) -> None:
    if route.zone_id != zone.zone_id:
        raise RouteInZoneError("ROUTE_ZONE_MISMATCH")
    if len(route.points) < 2:
        raise RouteInZoneError("ROUTE_TOO_SHORT")
    if not _segments_inside_zone(route, zone):
        raise RouteInZoneError("ROUTE_GEOMETRY_OUTSIDE_ZONE")
    if route.length <= 0:
        raise RouteInZoneError("ROUTE_ZERO_LENGTH")


def build_route_in_zone(
    *,
    route_id: str,
    uav_id: str,
    zone: Zone,
    points: Sequence[Point],
) -> Route:
    route_points = tuple(
        RoutePoint(f"{route_id}:P{i + 1:03d}", point)
        for i, point in enumerate(points)
    )
    route = Route(
        route_id=route_id,
        uav_id=uav_id,
        zone_id=zone.zone_id,
        points=route_points,
        length=_route_length(route_points),
        verified=False,
    )
    verify_route_in_zone(route, zone)
    return Route(
        route_id=route.route_id,
        uav_id=route.uav_id,
        zone_id=route.zone_id,
        points=route.points,
        length=route.length,
        verified=True,
    )


def build_boundary_lawnmower(
    *,
    route_id: str,
    uav_id: str,
    zone: Zone,
    passes: int = 2,
) -> Route:
    """Build a deterministic baseline route; not the production coverage planner."""
    if passes < 1:
        raise RouteInZoneError("PASSES_MUST_BE_POSITIVE")

    polygon = _polygon(zone.geometry)
    xs = [p.x for p in polygon.points]
    ys = [p.y for p in polygon.points]
    xmin, xmax = min(xs), max(xs)
    ymin, ymax = min(ys), max(ys)

    if passes == 1:
        path = [Point(xmin, ymin), Point(xmax, ymax)]
    else:
        path = []
        for i in range(passes):
            y = ymin + (ymax - ymin) * i / (passes - 1)
            path.extend(
                (Point(xmin, y), Point(xmax, y))
                if i % 2 == 0
                else (Point(xmax, y), Point(xmin, y))
            )

    return build_route_in_zone(
        route_id=route_id,
        uav_id=uav_id,
        zone=zone,
        points=path,
    )
