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
        "environmentSnapshotRefs": {
            "terrainSnapshotId": "terrain-snapshot-001",
            "airspaceSnapshotId": "airspace-snapshot-001",
            "weatherSnapshotId": "weather-snapshot-001",
            "windSnapshotId": "wind-snapshot-001",
        },
        "coordinateReferenceSystem": "EPSG:4326",
        "waypoints": [
            {"waypointId": "WP-1", "latitudeDeg": 60.0, "longitudeDeg": 24.0,
             "altitudeM": 100.0, "mandatory": False, "altitudeReference": "AGL"},
            {"waypointId": "WP-2", "latitudeDeg": 60.1, "longitudeDeg": 24.1,
             "altitudeM": 110.0, "mandatory": True, "altitudeReference": "AGL"},
        ],
        "segments": [
            {"segmentId": "SEG-1", "fromWaypointId": "WP-1", "toWaypointId": "WP-2",
             "distanceM": 12000.0, "nominalSpeedMps": 20.0}
        ],
        "trajectoryGeometryComplete": True,
        "validationState": "VALIDATED",
    }
    candidate_message = build_result_message(
        mission_id="MISSION-001", result_id="PLAN-002",
        pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
        route_candidate=route_candidate,
    )
    assert candidate_message["result"]["routeCandidate"]["coordinateReferenceSystem"] == "EPSG:4326"
    assert candidate_message["result"]["routeCandidate"]["environmentSnapshotRefs"]["terrainSnapshotId"] == "terrain-snapshot-001"
    assert candidate_message["result"]["routeCandidate"]["waypoints"][1]["mandatory"] is True

    candidate_a = {
        **route_candidate,
        "candidateId": "CAND-A",
        "flightSafetyValidationState": "ALLOWED",
        "flightSafetyEnvelopeId": "SAFE-ENV-1",
        "flightSafetyVehicleId": "UAV-1",
        "estimatedTimeS": 600.0,
        "estimatedEnergyWh": 45.0,
        "estimatedReserveWh": 25.0,
        "objectiveScore": 600.0,
    }
    candidate_b = {
        **candidate_a,
        "candidateId": "CAND-B",
        "routeId": "ROUTE-002",
        "routeVersion": "v2",
        "estimatedTimeS": 650.0,
        "estimatedEnergyWh": 40.0,
        "objectiveScore": 650.0,
    }
    candidate_a["environmentSnapshotRefs"] = {
        **candidate_a["environmentSnapshotRefs"],
        "weatherSnapshotId": "",
        "windSnapshotId": "",
    }
    candidate_b["environmentSnapshotRefs"] = dict(candidate_a["environmentSnapshotRefs"])
    candidate_set_message = build_result_message(
        mission_id="MISSION-001", result_id="PLAN-004",
        pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
        route_candidates=[candidate_a, candidate_b],
        selected_candidate_id="CAND-A",
        objective_priorities=["time", "energy"],
    )
    assert [x["candidateId"] for x in candidate_set_message["result"]["routeCandidates"]] == ["CAND-A", "CAND-B"]
    assert candidate_set_message["result"]["selectedCandidateId"] == "CAND-A"
    assert candidate_set_message["result"]["objectivePriorities"] == ["time", "energy"]

    broken_geometry = {**candidate_a, "segments": []}
    try:
        build_result_message(
            mission_id="MISSION-001", result_id="PLAN-008",
            pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
            route_candidates=[broken_geometry],
            objective_priorities=["time"],
        )
    except PlanningBridgeContractError as exc:
        assert str(exc) == "ROUTE_CANDIDATE_COMPLETE_SEGMENT_CHAIN_REQUIRED"
    else:
        raise AssertionError("incomplete trajectory geometry must be rejected")

    non_finite_metrics = {**candidate_a, "estimatedEnergyWh": float("nan")}
    try:
        build_result_message(
            mission_id="MISSION-001", result_id="PLAN-009",
            pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
            route_candidates=[non_finite_metrics],
            objective_priorities=["energy"],
        )
    except PlanningBridgeContractError as exc:
        assert str(exc) == "WEATHER_CANDIDATE_METRIC_INVALID:estimatedEnergyWh"
    else:
        raise AssertionError("non-finite weather ranking metrics must be rejected")

    invalid_candidate_set = [candidate_a, {**candidate_b, "flightSafetyVehicleId": "UAV-2"}]
    try:
        build_result_message(
            mission_id="MISSION-001", result_id="PLAN-005",
            pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
            route_candidates=invalid_candidate_set,
            objective_priorities=["time"],
        )
    except PlanningBridgeContractError as exc:
        assert str(exc) == "WEATHER_CANDIDATE_SET_LINEAGE_OR_SAFETY_MISMATCH"
    else:
        raise AssertionError("mixed UAV safety lineage must be rejected")

    mismatched_environment = {
        **candidate_b,
        "environmentSnapshotRefs": {
            **candidate_b["environmentSnapshotRefs"],
            "airspaceSnapshotId": "airspace-snapshot-other",
        },
    }
    try:
        build_result_message(
            mission_id="MISSION-001", result_id="PLAN-010",
            pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
            route_candidates=[candidate_a, mismatched_environment],
            objective_priorities=["time"],
        )
    except PlanningBridgeContractError as exc:
        assert str(exc) == "WEATHER_CANDIDATE_SET_LINEAGE_OR_SAFETY_MISMATCH"
    else:
        raise AssertionError("candidate routes must use the same airspace snapshot")

    try:
        build_result_message(
            mission_id="MISSION-001", result_id="PLAN-006",
            pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
            route_candidates=[candidate_a],
            selected_candidate_id="CAND-MISSING",
            objective_priorities=["time"],
        )
    except PlanningBridgeContractError as exc:
        assert str(exc) == "SELECTED_WEATHER_CANDIDATE_NOT_IN_SET"
    else:
        raise AssertionError("selected candidate outside the set must be rejected")

    try:
        build_result_message(
            mission_id="MISSION-001", result_id="PLAN-007",
            pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
            route_candidates=[candidate_a],
            objective_priorities=["minimum_cost"],
        )
    except PlanningBridgeContractError as exc:
        assert str(exc) == "WEATHER_CANDIDATE_OBJECTIVE_PRIORITIES_REQUIRED_OR_UNSUPPORTED"
    else:
        raise AssertionError("weather reranking must reject objectives without corrected metrics")
    assert '"missionId":"MISSION-001"' in dumps_result_message(message)

    incomplete_candidate = dict(route_candidate)
    incomplete_candidate.pop("environmentSnapshotRefs")
    try:
        build_result_message(
            mission_id="MISSION-001", result_id="PLAN-003",
            pipeline_result=Pipeline(Gate("PASS", "RELEASE_ELIGIBLE")),
            route_candidate=incomplete_candidate,
        )
    except PlanningBridgeContractError as exc:
        assert str(exc) == "ROUTE_CANDIDATE_FIELD_REQUIRED:environmentSnapshotRefs"
    else:
        raise AssertionError("route candidate without environment lineage must be rejected")

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
