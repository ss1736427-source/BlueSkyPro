"""Component-aware deterministic zone partitioning."""
from __future__ import annotations
from dataclasses import dataclass
from typing import Sequence

from constrained_space import ConstrainedOpenSpace
from polygon_splitter import ShapelyPolygonSplitter
from zone_partition import Zone, ZoneSet


@dataclass(frozen=True)
class ComponentPartition:
    component_id: str
    zones: tuple[Zone,...]
    status: str


class PartitionExecutionError(ValueError):
    pass


def partition_components(constrained: ConstrainedOpenSpace,zones_per_component:int=1)->tuple[ComponentPartition,...]:
    if zones_per_component<1:
        raise PartitionExecutionError("PARTITION_FAILED: zones_per_component must be positive")
    splitter=ShapelyPolygonSplitter()
    result=[]
    for component in constrained.components:
        try:
            split=splitter.split(component.geometry,zones_per_component)
        except Exception as exc:
            raise PartitionExecutionError(f"PARTITION_FAILED: {component.component_id}: {exc}") from exc
        zones=tuple(
            Zone(f"{component.component_id}-{zone.zone_id}",zone.geometry)
            for zone in split.zones
        )
        result.append(ComponentPartition(component.component_id,zones,"VERIFIED"))
    return tuple(result)


def flatten_component_partitions(constrained:ConstrainedOpenSpace,partitions:Sequence[ComponentPartition])->ZoneSet:
    if not partitions:
        raise PartitionExecutionError("PARTITION_FAILED: no component partitions")
    expected={c.component_id for c in constrained.components}
    actual={p.component_id for p in partitions}
    if actual!=expected:
        raise PartitionExecutionError("PARTITION_FAILED: component coverage mismatch")
    all_zones=tuple(z for p in partitions for z in p.zones)
    total=sum(z.geometry.area for z in all_zones)
    source_area=sum(c.geometry.area for c in constrained.components)
    if abs(total-source_area)>1e-9:
        raise PartitionExecutionError("PARTITION_FAILED: incomplete component coverage")
    return ZoneSet(all_zones,total,source_area,"VERIFIED")
