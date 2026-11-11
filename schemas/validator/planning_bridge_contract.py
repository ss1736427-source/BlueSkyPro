"""Serialization contract between the Python Planning Core and the Qt HMI bridge."""
from __future__ import annotations

from dataclasses import asdict, is_dataclass
from datetime import datetime, timezone
import json
from typing import Any

SCHEMA_VERSION = "1.0"
MESSAGE_TYPE = "planning.result"


class PlanningBridgeContractError(ValueError):
    pass


_WEATHER_OBJECTIVES = {
    "completion_time", "time", "fast", "energy", "endurance", "reserve"
}


def _validate_route_candidate(candidate: dict[str, Any], mission_id: str, *, weather_set: bool) -> None:
    if not isinstance(candidate, dict):
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_MUST_BE_OBJECT")
    required = (
        "routeId", "routeVersion", "missionId", "missionVersion",
        "generatorId", "generatorVersion", "calculationInputVersion",
        "environmentSnapshotRefs", "coordinateReferenceSystem", "waypoints",
        "segments", "trajectoryGeometryComplete", "validationState",
    )
    for key in required:
        if key not in candidate:
            raise PlanningBridgeContractError(f"ROUTE_CANDIDATE_FIELD_REQUIRED:{key}")
    for key in ("routeId", "routeVersion", "missionId", "missionVersion",
                "generatorId", "generatorVersion", "calculationInputVersion"):
        if not isinstance(candidate[key], str) or not candidate[key].strip():
            raise PlanningBridgeContractError(f"ROUTE_CANDIDATE_FIELD_REQUIRED:{key}")
    if candidate["missionId"] != mission_id:
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_MISSION_MISMATCH")
    if candidate["coordinateReferenceSystem"] != "EPSG:4326":
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_CRS_MUST_BE_EPSG_4326")
    if candidate["trajectoryGeometryComplete"] is not True:
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_COMPLETE_TRAJECTORY_REQUIRED")
    if candidate["validationState"] != "VALIDATED":
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_ROUTE_VALIDATION_REQUIRED")

    waypoints = candidate["waypoints"]
    segments = candidate["segments"]
    if not isinstance(waypoints, list) or len(waypoints) < 2:
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_AT_LEAST_TWO_WAYPOINTS_REQUIRED")
    if not isinstance(segments, list) or len(segments) != len(waypoints) - 1:
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_COMPLETE_SEGMENT_CHAIN_REQUIRED")

    environment = candidate["environmentSnapshotRefs"]
    if not isinstance(environment, dict):
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_ENVIRONMENT_SNAPSHOT_REFS_REQUIRED")
    for key in ("terrainSnapshotId", "airspaceSnapshotId"):
        if not isinstance(environment.get(key), str) or not environment[key].strip():
            raise PlanningBridgeContractError(f"ROUTE_CANDIDATE_ENVIRONMENT_SNAPSHOT_REQUIRED:{key}")
    weather_id = environment.get("weatherSnapshotId", "")
    wind_id = environment.get("windSnapshotId", "")
    if not isinstance(weather_id, str) or not isinstance(wind_id, str) or bool(weather_id.strip()) != bool(wind_id.strip()):
        raise PlanningBridgeContractError("ROUTE_CANDIDATE_WEATHER_AND_WIND_SNAPSHOT_REFS_MUST_BE_PAIRED")

    if weather_set:
        required_weather = (
            "candidateId", "estimatedTimeS", "estimatedEnergyWh",
            "estimatedReserveWh", "objectiveScore", "flightSafetyValidationState",
            "flightSafetyEnvelopeId", "flightSafetyVehicleId",
        )
        for key in required_weather:
            if key not in candidate:
                raise PlanningBridgeContractError(f"WEATHER_CANDIDATE_FIELD_REQUIRED:{key}")
        if not isinstance(candidate["candidateId"], str) or not candidate["candidateId"].strip():
            raise PlanningBridgeContractError("WEATHER_CANDIDATE_ID_REQUIRED")
        if candidate["flightSafetyValidationState"] != "ALLOWED":
            raise PlanningBridgeContractError("WEATHER_CANDIDATE_FLIGHT_SAFETY_NOT_ALLOWED")
        for key in ("flightSafetyEnvelopeId", "flightSafetyVehicleId"):
            if not isinstance(candidate[key], str) or not candidate[key].strip():
                raise PlanningBridgeContractError(f"WEATHER_CANDIDATE_FIELD_REQUIRED:{key}")
        for key, minimum, exclusive in (
            ("estimatedTimeS", 0.0, True),
            ("estimatedEnergyWh", 0.0, False),
            ("estimatedReserveWh", 0.0, False),
        ):
            value = candidate[key]
            if isinstance(value, bool) or not isinstance(value, (int, float)) or value < minimum or (exclusive and value == minimum):
                raise PlanningBridgeContractError(f"WEATHER_CANDIDATE_METRIC_INVALID:{key}")
        score = candidate["objectiveScore"]
        if isinstance(score, bool) or not isinstance(score, (int, float)):
            raise PlanningBridgeContractError("WEATHER_CANDIDATE_METRIC_INVALID:objectiveScore")


def build_result_message(
    *,
    mission_id: str,
    result_id: str,
    pipeline_result: object,
    three_d_mapping: object | None = None,
    contract_version: str = SCHEMA_VERSION,
    planner_version: str | None = None,
    configuration_version: str | None = None,
    run_id: str | None = None,
    input_refs: tuple[str, ...] = (),
    artifact_refs: tuple[str, ...] = (),
    evidence_refs: tuple[str, ...] = (),
    route_candidate: dict[str, Any] | None = None,
    route_candidates: tuple[dict[str, Any], ...] | list[dict[str, Any]] | None = None,
    selected_candidate_id: str | None = None,
    objective_priorities: tuple[str, ...] | list[str] = (),
) -> dict[str, Any]:
    if not mission_id or not result_id:
        raise PlanningBridgeContractError("MISSION_AND_RESULT_IDS_REQUIRED")

    gate = getattr(pipeline_result, "final_gate", None)
    if gate is None:
        raise PlanningBridgeContractError("FINAL_GATE_REQUIRED")

    release_status = str(getattr(gate, "release_status"))
    final_gate_status = str(getattr(gate, "status"))
    if release_status not in {"RELEASE_ELIGIBLE", "BLOCKED"}:
        raise PlanningBridgeContractError("INVALID_RELEASE_STATUS")
    if final_gate_status not in {"PASS", "FAIL"}:
        raise PlanningBridgeContractError("INVALID_FINAL_GATE_STATUS")

    source = {"runtime": "python-planning-core", "contractVersion": contract_version}
    if planner_version is not None:
        source["plannerVersion"] = planner_version
    if configuration_version is not None:
        source["configurationVersion"] = configuration_version
    if run_id is not None:
        source["runId"] = run_id

    result: dict[str, Any] = {}
    if route_candidate is not None and route_candidates is not None:
        raise PlanningBridgeContractError("SINGLE_AND_MULTI_ROUTE_CANDIDATE_PAYLOADS_ARE_MUTUALLY_EXCLUSIVE")
    if route_candidate is not None:
        _validate_route_candidate(route_candidate, mission_id, weather_set=False)
        result["routeCandidate"] = route_candidate
    if route_candidates is not None:
        if not isinstance(route_candidates, (tuple, list)) or not 1 <= len(route_candidates) <= 64:
            raise PlanningBridgeContractError("WEATHER_ROUTE_CANDIDATE_SET_SIZE_MUST_BE_1_TO_64")
        if not objective_priorities or any(item not in _WEATHER_OBJECTIVES for item in objective_priorities):
            raise PlanningBridgeContractError("WEATHER_CANDIDATE_OBJECTIVE_PRIORITIES_REQUIRED_OR_UNSUPPORTED")
        seen_candidates: set[str] = set()
        seen_routes: set[str] = set()
        common_lineage: tuple[str, str, str, str] | None = None
        normalized_selected = selected_candidate_id or ""
        for candidate in route_candidates:
            _validate_route_candidate(candidate, mission_id, weather_set=True)
            candidate_id = candidate["candidateId"]
            route_id = candidate["routeId"]
            if candidate_id in seen_candidates or route_id in seen_routes:
                raise PlanningBridgeContractError("WEATHER_CANDIDATE_OR_ROUTE_ID_DUPLICATE")
            seen_candidates.add(candidate_id)
            seen_routes.add(route_id)
            lineage = (
                candidate["missionVersion"], candidate["calculationInputVersion"],
                candidate["flightSafetyEnvelopeId"], candidate["flightSafetyVehicleId"],
            )
            if common_lineage is None:
                common_lineage = lineage
            elif common_lineage != lineage:
                raise PlanningBridgeContractError("WEATHER_CANDIDATE_SET_LINEAGE_OR_SAFETY_MISMATCH")
        if normalized_selected and normalized_selected not in seen_candidates:
            raise PlanningBridgeContractError("SELECTED_WEATHER_CANDIDATE_NOT_IN_SET")
        result["routeCandidates"] = list(route_candidates)
        result["selectedCandidateId"] = normalized_selected
        result["objectivePriorities"] = list(objective_priorities)
    if three_d_mapping is not None:
        if not is_dataclass(three_d_mapping):
            raise PlanningBridgeContractError("3D_MAPPING_RESULT_MUST_BE_DATACLASS")
        raw = asdict(three_d_mapping)
        result["threeDMapping"] = {
            "lineSpacingM": raw["line_spacing_m"],
            "lineCount": raw["line_count"],
            "expectedFrames": raw["expected_frames"],
            "expectedCoveragePercent": raw["expected_coverage_percent"],
            "routeLengthM": raw["route_length_m"],
            "expectedDurationS": raw["expected_duration_s"],
            "expectedEnergyWh": raw["expected_energy_wh"],
            "requiredReserveWh": raw["required_reserve_wh"],
            "dataVolumeMb": raw["data_volume_mb"],
            "releaseStatus": raw["release_status"],
            "verified": raw["verified"],
        }

    release_verified = release_status == "RELEASE_ELIGIBLE"
    message = {
        "schemaVersion": SCHEMA_VERSION,
        "messageType": MESSAGE_TYPE,
        "missionId": mission_id,
        "resultId": result_id,
        "generatedAt": datetime.now(timezone.utc).isoformat(),
        "source": source,
        "verification": {
            "finalGateStatus": final_gate_status,
            "releaseStatus": release_status,
            "verified": release_verified,
            "failedChecks": list(getattr(gate, "failed_checks", ())),
            "upstreamChecks": list(getattr(gate, "upstream_checks", ())),
            "authorizationStatus": str(getattr(gate, "authorization_status", "NOT_EVALUATED")),
        },
        "traceability": {
            "inputRefs": list(input_refs),
            "artifactRefs": list(artifact_refs),
            "evidenceRefs": list(evidence_refs),
        },
        "result": result,
    }
    return message


def dumps_result_message(message: dict[str, Any]) -> str:
    if message.get("schemaVersion") != SCHEMA_VERSION:
        raise PlanningBridgeContractError("UNSUPPORTED_SCHEMA_VERSION")
    if message.get("messageType") != MESSAGE_TYPE:
        raise PlanningBridgeContractError("UNSUPPORTED_MESSAGE_TYPE")
    return json.dumps(message, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
