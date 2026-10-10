#!/usr/bin/env python3

from __future__ import annotations

import sys

from constrained_partition import (
    PartitionExecutionError,
    flatten_component_partitions,
    partition_components,
)
from constrained_space import ConstrainedOpenSpace
from zone_partition import Point, Polygon


def main() -> int:
    component_a = Polygon((
        Point(0, 0), Point(40, 0), Point(40, 40), Point(0, 40)
    ))
    component_b = Polygon((
        Point(60, 0), Point(100, 0), Point(100, 40), Point(60, 40)
    ))

    constrained = ConstrainedOpenSpace(
        source=Polygon((
            Point(0, 0), Point(100, 0), Point(100, 40), Point(0, 40)
        )),
        exclusions=(),
        components=(
            type("C", (), {"component_id": "COMP-01", "geometry": component_a})(),
            type("C", (), {"component_id": "COMP-02", "geometry": component_b})(),
        ),
        status="VERIFIED",
    )

    partitions = partition_components(constrained)
    assert len(partitions) == 2
    assert partitions[0].zones[0].zone_id == "COMP-01-ZONE-01"
    assert partitions[1].zones[0].zone_id == "COMP-02-ZONE-01"

    result = flatten_component_partitions(constrained, partitions)
    assert result.status == "VERIFIED"
    assert result.coverage_area == component_a.area + component_b.area

    split_result = partition_components(constrained, zones_per_component=2)
    assert len(split_result) == 2
    assert all(len(partition.zones) == 2 for partition in split_result)
    split_flat = flatten_component_partitions(constrained, split_result)
    assert split_flat.status == "VERIFIED"
    assert split_flat.coverage_area == component_a.area + component_b.area

    print("CONSTRAINED PARTITION TESTS: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
