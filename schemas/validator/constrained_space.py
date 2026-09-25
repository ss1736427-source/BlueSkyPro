"""Constrained Open Space construction using the geometry adapter."""
from __future__ import annotations
from dataclasses import dataclass
from typing import Sequence
from zone_partition import EPSILON, Polygon
from geometry_engine import GeometryEngine, ContractGeometryEngine

@dataclass(frozen=True)
class Exclusion:
    exclusion_id: str
    geometry: Polygon
    reason: str

@dataclass(frozen=True)
class ConstrainedComponent:
    component_id: str
    geometry: Polygon
    source_exclusions: tuple[str,...]=()

@dataclass(frozen=True)
class ConstrainedOpenSpace:
    source: Polygon
    exclusions: tuple[Exclusion,...]
    components: tuple[ConstrainedComponent,...]
    status: str

class ConstrainedSpaceError(ValueError):
    pass

def build_constrained_space(source: Polygon, exclusions: Sequence[Exclusion],
                            engine: GeometryEngine | None = None) -> ConstrainedOpenSpace:
    engine=engine or ContractGeometryEngine()
    for exclusion in exclusions:
        if exclusion.geometry.area<=EPSILON:
            raise ConstrainedSpaceError(f"CONSTRAINED_SPACE_INVALID: zero-area exclusion {exclusion.exclusion_id}")
        if not engine.contains(source,exclusion.geometry):
            raise ConstrainedSpaceError(f"CONSTRAINED_SPACE_INVALID: exclusion {exclusion.exclusion_id} outside source")
    try:
        raw=engine.subtract(source,[e.geometry for e in exclusions])
    except (NotImplementedError,RuntimeError,ValueError) as exc:
        raise ConstrainedSpaceError(str(exc)) from exc
    if not raw:
        raise ConstrainedSpaceError("CONSTRAINED_SPACE_INVALID: no usable operational space")
    components=tuple(ConstrainedComponent(c.component_id,c.geometry) for c in raw)
    return ConstrainedOpenSpace(source,tuple(exclusions),components,"VERIFIED")
