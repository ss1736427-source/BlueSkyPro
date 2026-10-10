"""Deterministic polygon decomposition for connected components."""

from __future__ import annotations

from dataclasses import dataclass

from zone_partition import Point, Polygon, Zone, ZonePartitionError, verify_zone_set


@dataclass(frozen=True)
class SplitResult:
    zones: tuple[Zone, ...]
    status: str


class PolygonSplitError(ValueError):
    pass


class ShapelyPolygonSplitter:
    """Split one connected polygon into deterministic contiguous sectors."""

    def __init__(self) -> None:
        try:
            from shapely.ops import split
            from shapely.geometry import LineString, Polygon as SPolygon
        except ImportError as exc:
            raise RuntimeError("Shapely is required for polygon splitting") from exc
        self._split = split
        self._LineString = LineString
        self._SPolygon = SPolygon

    @staticmethod
    def _to_shapely(polygon: Polygon):
        return ShapelyPolygonSplitter._polygon_from_contract(polygon)

    @staticmethod
    def _polygon_from_contract(polygon: Polygon):
        from shapely.geometry import Polygon as SPolygon
        return SPolygon(
            [(p.x, p.y) for p in polygon.points],
            [[(p.x, p.y) for p in h] for h in polygon.holes],
        )

    @staticmethod
    def _from_shapely(geometry) -> Polygon:
        if geometry.geom_type != "Polygon":
            raise PolygonSplitError(f"unexpected split result: {geometry.geom_type}")
        holes = tuple(
            tuple(Point(x, y) for x, y in ring.coords[:-1])
            for ring in geometry.interiors
        )
        return Polygon(
            tuple(Point(x, y) for x, y in geometry.exterior.coords[:-1]),
            holes,
        )

    def split(self, source: Polygon, count: int) -> SplitResult:
        if count < 1:
            raise PolygonSplitError("POLYGON_SPLIT_FAILED: count must be positive")
        if count == 1:
            return SplitResult((Zone("ZONE-01", source),), "VERIFIED")

        geometry = self._to_shapely(source)
        if not geometry.is_valid:
            raise PolygonSplitError("POLYGON_SPLIT_FAILED: invalid source geometry")

        min_x, min_y, max_x, max_y = geometry.bounds
        horizontal = (max_x - min_x) >= (max_y - min_y)

        pieces = [geometry]
        while len(pieces) < count:
            pieces.sort(key=lambda g: (-g.area, g.bounds[0], g.bounds[1]))
            target = pieces.pop(0)
            x0, y0, x1, y1 = target.bounds

            if horizontal:
                mid = (x0 + x1) / 2.0
                blade = self._LineString([(mid, y0 - 1.0), (mid, y1 + 1.0)])
            else:
                mid = (y0 + y1) / 2.0
                blade = self._LineString([(x0 - 1.0, mid), (x1 + 1.0, mid)])

            result = self._split(target, blade)
            new_pieces = list(result.geoms)

            if len(new_pieces) < 2:
                # Try the opposite axis before failing.
                if horizontal:
                    mid = (y0 + y1) / 2.0
                    blade = self._LineString([(x0 - 1.0, mid), (x1 + 1.0, mid)])
                else:
                    mid = (x0 + x1) / 2.0
                    blade = self._LineString([(mid, y0 - 1.0), (mid, y1 + 1.0)])
                result = self._split(target, blade)
                new_pieces = list(result.geoms)

            if len(new_pieces) < 2:
                raise PolygonSplitError(
                    "POLYGON_SPLIT_FAILED: component cannot be deterministically split"
                )

            pieces.extend(new_pieces)

        pieces.sort(key=lambda g: (-g.area, g.bounds[0], g.bounds[1], g.bounds[2], g.bounds[3]))
        zones = tuple(
            Zone(f"ZONE-{index + 1:02d}", self._from_shapely(piece))
            for index, piece in enumerate(pieces)
        )

        try:
            verify_zone_set(source, zones)
        except ZonePartitionError as exc:
            raise PolygonSplitError(str(exc)) from exc

        return SplitResult(zones, "VERIFIED")
