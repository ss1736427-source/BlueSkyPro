"""Component-aware deterministic zone partitioning."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence

from constrained_space import ConstrainedOpenSpace, ConstrainedComponent
from zone_partition import Polygon, Zone, ZoneSet, verify_zone_set


@dataclass(frozen=True)
class ComponentPartition:
    component_id: str
    zones: tuple[Zone, ...]
    status: str


class PartitionExecutionError(ValueError):
    pass


def partition_components(
    constrained: ConstrainedOpenSpace,
    zones_per_component: int = 1,
) -> tuple[ComponentPartition, ...]:
    if zones_per_component < 1:
        raise PartitionExecutionError("PARTITION_FAILED: zones_per_component must be positive")

    result: list[ComponentPartition] = []

    for component in constrained.components:
        if zones_per_component == 1:
            zones = (Zone(
                zone_id=f"{component.component_id}-ZONE-01",
                geometry=component.geometry,
            ),)
        else:
            raise PartitionExecutionError(
                "PARTITION_REQUIRES_COMPONENT_SPLITTER: "
                f"{component.component_id} requires polygon decomposition"
            )

        verify_zone_set(component.geometry, zones)
        result.append(ComponentPartition(component.component_id, zones, "VERIFIED"))

    return tuple(result)


def flatten_component_partitions(
    constrained: ConstrainedOpenSpace,
    partitions: Sequence[ComponentPartition],
) -> ZoneSet:
    if not partitions:
        raise PartitionExecutionError("PARTITION_FAILED: no component partitions")

    expected = {c.component_id for c in constrained.components}
    actual = {p.component_id for p in partitions}

    if actual != expected:
        raise PartitionExecutionError("PARTITION_FAILED: component coverage mismatch")

    all_zones = tuple(zone for partition in partitions for zone in partition.zones)

    # Each connected component is independently verified. A global area sum
    # is safe because connected components are disjoint by construction.
    total = sum(zone.geometry.area for zone in all_zones)
    source_area = sum(component.geometry.area for component in constrained.components)

    if abs(total - source_area) > 1e-9:
        raise PartitionExecutionError("PARTITION_FAILED: incomplete component coverage")

    return ZoneSet(
        zones=all_zones,
        coverage_area=total,
        source_area=source_area,
        status="VERIFIED",
    )
