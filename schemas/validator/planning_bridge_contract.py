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
