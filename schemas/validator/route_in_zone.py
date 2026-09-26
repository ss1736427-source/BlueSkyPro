"""Route generation and containment verification for assigned UAV zones."""

from __future__ import annotations

from dataclasses import dataclass
from math import hypot
from typing import Sequence

from zone_partition import Point, Zone, polygon_contains_polygon


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


def _route_length(points: Sequence[RoutePoint]) -> float:
    return sum(
        hypot(
            points[i + 1].position.x - points[i].position.x,
            points[i + 1].position.y - points[i].position.y,
        )
        for i in range(len(points) - 1)
    )


def verify_route_in_zone(route: Route, zone: Zone) -> None:
    if route.zone_id != zone.zone_id:
        raise RouteInZoneError("ROUTE_ZONE_MISMATCH")
    if len(route.points) < 2:
        raise RouteInZoneError("ROUTE_TOO_SHORT")

    for point in route.points:
        if not zone.geometry.contains(point.position):
            raise RouteInZoneError(f"ROUTE_POINT_OUTSIDE_ZONE:{point.point_id}")

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
    """Build a deterministic simple route from the zone bounding box.

    This is a contract-level baseline, not the production coverage planner.
    """
    if passes < 1:
        raise RouteInZoneError("PASSES_MUST_BE_POSITIVE")

    xs = [p.x for p in zone.geometry.points]
    ys = [p.y for p in zone.geometry.points]
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
