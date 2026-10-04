"""Geometry-engine adapter boundary and Shapely backend."""
from __future__ import annotations
from dataclasses import dataclass
from typing import Protocol, Sequence
from zone_partition import Point, Polygon

@dataclass(frozen=True)
class GeometryComponent:
    component_id: str
    geometry: Polygon

class GeometryEngine(Protocol):
    def subtract(self, source: Polygon, exclusions: Sequence[Polygon]) -> tuple[GeometryComponent,...]: ...
    def contains(self, outer: Polygon, inner: Polygon) -> bool: ...
    def interiors_overlap(self, left: Polygon, right: Polygon) -> bool: ...

class ContractGeometryEngine:
    def contains(self, outer: Polygon, inner: Polygon) -> bool:
        from zone_partition import _contains
        return _contains(outer,inner)
    def interiors_overlap(self,left: Polygon,right: Polygon)->bool:
        from zone_partition import _interior_overlap
        return _interior_overlap(left,right)
    def subtract(self,source: Polygon,exclusions: Sequence[Polygon])->tuple[GeometryComponent,...]:
        if not exclusions: return (GeometryComponent("COMP-01",source),)
        raise NotImplementedError("polygon subtraction requires a vetted clipping backend")

class ShapelyGeometryEngine:
    """GEOS-backed adapter using Shapely 2.1.x; not the domain model itself."""
    def __init__(self)->None:
        try:
            from shapely.ops import unary_union
        except ImportError as exc:
            raise RuntimeError("Shapely is required for ShapelyGeometryEngine") from exc
        self._unary_union=unary_union
    @staticmethod
    def _to_shapely(polygon:Polygon):
        from shapely.geometry import Polygon as SPolygon
        return SPolygon([(p.x,p.y) for p in polygon.points],
                        [[(p.x,p.y) for p in h] for h in polygon.holes])
    @staticmethod
    def _from_shapely(geometry):
        if geometry.geom_type!="Polygon":
            raise ValueError(f"unsupported component geometry: {geometry.geom_type}")
        holes=tuple(tuple(Point(x,y) for x,y in ring.coords[:-1]) for ring in geometry.interiors)
        return Polygon(tuple(Point(x,y) for x,y in geometry.exterior.coords[:-1]),holes)
    def contains(self,outer:Polygon,inner:Polygon)->bool:
        return self._to_shapely(outer).covers(self._to_shapely(inner))
    def interiors_overlap(self,left:Polygon,right:Polygon)->bool:
        return self._to_shapely(left).intersection(self._to_shapely(right)).area>1e-9
    def subtract(self,source:Polygon,exclusions:Sequence[Polygon])->tuple[GeometryComponent,...]:
        result=self._to_shapely(source)
        if not result.is_valid:
            raise ValueError("source geometry is invalid")

        exclusion_geometries=[]
        for exclusion in exclusions:
            geometry=self._to_shapely(exclusion)
            if not geometry.is_valid:
                raise ValueError("exclusion geometry is invalid")
            if not result.covers(geometry):
                raise ValueError("exclusion geometry is outside source")
            exclusion_geometries.append(geometry)

        if exclusion_geometries:
            result=result.difference(self._unary_union(exclusion_geometries))

        if result.is_empty:
            return ()
        if not result.is_valid:
            raise ValueError("clipped constrained geometry is invalid")
        if result.geom_type not in {"Polygon","MultiPolygon"}:
            raise ValueError(f"unsupported clipped geometry: {result.geom_type}")

        geoms=list(result.geoms) if result.geom_type=="MultiPolygon" else [result]
        geoms.sort(key=lambda g:(-g.area,g.bounds[0],g.bounds[1],g.bounds[2],g.bounds[3]))

        components=tuple(
            GeometryComponent(f"COMP-{i+1:02d}",self._from_shapely(g))
            for i,g in enumerate(geoms)
        )
        if abs(sum(c.geometry.area for c in components)-result.area)>1e-9:
            raise ValueError("clipped component area mismatch")
        return components
