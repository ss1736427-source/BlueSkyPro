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
    route_geometry: dict[str, Any] | None = None,
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
    if route_geometry is not None:
        route_id = route_geometry.get("routeId")
        route_version = route_geometry.get("routeVersion")
        coordinate_reference = route_geometry.get("coordinateReference")
        points = route_geometry.get("points")
        if not isinstance(route_id, str) or not route_id.strip():
            raise PlanningBridgeContractError("ROUTE_ID_REQUIRED")
        if not isinstance(route_version, str) or not route_version.strip():
            raise PlanningBridgeContractError("ROUTE_VERSION_REQUIRED")
        if coordinate_reference != "WGS84":
            raise PlanningBridgeContractError("ROUTE_COORDINATE_REFERENCE_MUST_BE_WGS84")
        if not isinstance(points, list) or len(points) < 2:
            raise PlanningBridgeContractError("ROUTE_REQUIRES_AT_LEAST_TWO_POINTS")
        normalized_points = []
        for point in points:
            if not isinstance(point, dict):
                raise PlanningBridgeContractError("INVALID_ROUTE_POINT")
            waypoint_id = point.get("waypointId")
            latitude = point.get("latitude")
            longitude = point.get("longitude")
            altitude = point.get("altitudeM")
            mandatory = point.get("mandatory")
            if not isinstance(waypoint_id, str) or not waypoint_id.strip():
                raise PlanningBridgeContractError("ROUTE_WAYPOINT_ID_REQUIRED")
            if isinstance(latitude, bool) or not isinstance(latitude, (int, float)) or not -90 <= latitude <= 90:
                raise PlanningBridgeContractError("INVALID_ROUTE_LATITUDE")
            if isinstance(longitude, bool) or not isinstance(longitude, (int, float)) or not -180 <= longitude <= 180:
                raise PlanningBridgeContractError("INVALID_ROUTE_LONGITUDE")
            if isinstance(altitude, bool) or not isinstance(altitude, (int, float)):
                raise PlanningBridgeContractError("INVALID_ROUTE_ALTITUDE")
            if not isinstance(mandatory, bool):
                raise PlanningBridgeContractError("INVALID_ROUTE_MANDATORY_FLAG")
            normalized_points.append({
                "waypointId": waypoint_id,
                "latitude": float(latitude),
                "longitude": float(longitude),
                "altitudeM": float(altitude),
                "mandatory": mandatory,
            })
        result["routeGeometry"] = {
            "routeId": route_id,
            "routeVersion": route_version,
            "coordinateReference": "WGS84",
            "points": normalized_points,
        }

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
