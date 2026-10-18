#!/usr/bin/env python3
from dataclasses import dataclass

from planning_bridge_contract import (
    MESSAGE_TYPE,
    SCHEMA_VERSION,
    PlanningBridgeContractError,
    build_result_message,
    dumps_result_message,
)

@dataclass(frozen=True)
class Gate:
    status: str
    release_status: str
    failed_checks: tuple[str, ...] = ()
    upstream_checks: tuple[str, ...] = ()
    authorization_status: str = "NOT_EVALUATED"

@dataclass(frozen=True)
class Pipeline:
    final_gate: Gate

@dataclass(frozen=True)
class Mapping:
    line_spacing_m: float | None
    line_count: int | None
    expected_frames: int | None
    expected_coverage_percent: float | None
    route_length_m: float
    expected_duration_s: float
    expected_energy_wh: float
    required_reserve_wh: float | None
    data_volume_mb: float | None
    release_status: str
    verified: bool

def main() -> int:
    message = build_result_message(
        mission_id="MISSION-001",
        result_id="PLAN-001",
        pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
        three_d_mapping=Mapping(2.5, 10, 1000, 98.0, 2500.0, 900.0, 120.0, 30.0, 512.0, "RELEASE_ELIGIBLE", True),
        run_id="RUN-001",
    )
    assert message["schemaVersion"] == SCHEMA_VERSION
    assert message["messageType"] == MESSAGE_TYPE
    assert message["verification"]["verified"] is True
    assert message["result"]["threeDMapping"]["routeLengthM"] == 2500.0
    route_candidate = {
        "routeId": "ROUTE-001", "routeVersion": "v1",
        "missionId": "MISSION-001", "missionVersion": "m1",
        "generatorId": "planner", "generatorVersion": "1",
        "calculationInputVersion": "input-1",
        "coordinateReferenceSystem": "EPSG:4326",
        "waypoints": [
            {"waypointId": "WP-1", "latitudeDeg": 60.0, "longitudeDeg": 24.0,
             "altitudeM": 100.0, "mandatory": False, "altitudeReference": "AGL"},
            {"waypointId": "WP-2", "latitudeDeg": 60.1, "longitudeDeg": 24.1,
             "altitudeM": 110.0, "mandatory": True, "altitudeReference": "AGL"},
        ],
    }
    candidate_message = build_result_message(
        mission_id="MISSION-001", result_id="PLAN-002",
        pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
        route_candidate=route_candidate,
    )
    assert candidate_message["result"]["routeCandidate"]["coordinateReferenceSystem"] == "EPSG:4326"
    assert candidate_message["result"]["routeCandidate"]["waypoints"][1]["mandatory"] is True
    assert '"missionId":"MISSION-001"' in dumps_result_message(message)

    try:
        build_result_message(mission_id="", result_id="PLAN-001", pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")))
    except PlanningBridgeContractError as exc:
        assert str(exc) == "MISSION_AND_RESULT_IDS_REQUIRED"
    else:
        raise AssertionError("missing mission id must be rejected")

    blocked = build_result_message(
        mission_id="MISSION-002",
        result_id="PLAN-002",
        pipeline_result=Pipeline(Gate("FAIL", "BLOCKED", ("CONFLICT:CONFLICT",))),
    )
    assert blocked["verification"]["verified"] is False
    assert blocked["verification"]["releaseStatus"] == "BLOCKED"

    print("PLANNING BRIDGE CONTRACT TESTS: PASS")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
