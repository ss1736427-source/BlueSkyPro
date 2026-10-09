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
    assert '"missionId":"MISSION-001"' in dumps_result_message(message)

    route_geometry = {
        "routeId": "ROUTE-001",
        "routeVersion": "3",
        "coordinateReference": "WGS84",
        "points": [
            {"waypointId": "WP-START", "latitude": 55.75, "longitude": 37.61, "altitudeM": 120.0, "mandatory": True},
            {"waypointId": "WP-FINISH", "latitude": 55.76, "longitude": 37.63, "altitudeM": 150.0, "mandatory": True},
        ],
    }
    with_route = build_result_message(
        mission_id="MISSION-001",
        result_id="PLAN-002",
        pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
        route_geometry=route_geometry,
    )
    published_route = with_route["result"]["routeGeometry"]
    assert published_route["coordinateReference"] == "WGS84"
    assert published_route["points"][1]["latitude"] == 55.76
    assert published_route["points"][1]["altitudeM"] == 150.0

    invalid_route = dict(route_geometry, coordinateReference="LOCAL_XY")
    try:
        build_result_message(
            mission_id="MISSION-001",
            result_id="PLAN-003",
            pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
            route_geometry=invalid_route,
        )
    except PlanningBridgeContractError as exc:
        assert str(exc) == "ROUTE_COORDINATE_REFERENCE_MUST_BE_WGS84"
    else:
        raise AssertionError("non-geodetic route coordinates must be rejected")

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
