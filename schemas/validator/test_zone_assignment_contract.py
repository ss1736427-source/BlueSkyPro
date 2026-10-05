#!/usr/bin/env python3

from uav_assignment import UAVCapability, assign_zones
from zone_assignment_contract import build_assignment_artifact
from zone_partition import Point, Polygon, Zone


def make_zone(zone_id: str, x0: float, x1: float) -> Zone:
    return Zone(zone_id, Polygon((
        Point(x0, 0), Point(x1, 0), Point(x1, 10), Point(x0, 10)
    )))


def main() -> int:
    zones = (make_zone("ZONE-01", 0, 10), make_zone("ZONE-02", 10, 20))
    fleet = (
        UAVCapability("UAV-01", True, True, 1200, 600, 300, True, True, 5, 2),
        UAVCapability("UAV-02", True, True, 1200, 600, 300, True, True, 4, 1),
    )

    result = assign_zones(zones, fleet)
    artifact = build_assignment_artifact(
        mission_id="MISSION-001",
        zone_set_id="ZONESET-001",
        zone_set_version="zone-v3",
        assignment_version="assignment-v1",
        result=result,
    )

    assert artifact.artifact_id == "MISSION-001:zone-assignment:assignment-v1"
    assert artifact.source_artifact_ids == ("ZONESET-001",)
    assert artifact.source_versions == ("zone-v3",)
    assert artifact.status == "VERIFIED"
    assert len(artifact.assignments) == 2
    assert len(artifact.candidate_matrix) == 4

    for candidate in artifact.candidate_matrix:
        assert "failedConstraints" in candidate
        assert "feasible" in candidate

    print("ZONE ASSIGNMENT CONTRACT TEST: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
