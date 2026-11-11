"""JSONL stdio adapter for the existing BlueSky Planning Core orchestrator.

The adapter reconstructs verified planning artifacts from the machine-readable
request contract and delegates safety decisions to run_multi_uav_pipeline().
It contains no route planner or alternate mission core.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parent
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from conflict_4d import SeparationMinimums, verify_fleet
from conflict_resolution import ResolutionPolicy
from planning_bridge_contract import dumps_result_message, build_result_message
from multi_uav_orchestrator import PipelineInputs, run_multi_uav_pipeline
from route_in_zone import Route, RoutePoint
from trajectory_4d import Trajectory4D, TrajectoryPoint
from wind_performance import PerformanceAdjustedRoute, PerformanceSegment
from three_d_mapping_adapter import build_three_d_mapping_result


SCHEMA_VERSION = "1.0"
MESSAGE_TYPE = "planning.request"


class RequestError(ValueError):
    pass


def _require(obj: dict[str, Any], key: str) -> Any:
    if key not in obj:
        raise RequestError(f"REQUEST_FIELD_REQUIRED:{key}")
    return obj[key]


def _point(raw: dict[str, Any]) -> RoutePoint:
    return RoutePoint(
        point_id=str(_require(raw, "pointId")),
        position=__import__("zone_partition").Point(
            float(_require(raw, "x")), float(_require(raw, "y"))
        ),
    )


def _route(raw: dict[str, Any]) -> Route:
    return Route(
        route_id=str(_require(raw, "routeId")),
        uav_id=str(_require(raw, "uavId")),
        zone_id=str(_require(raw, "zoneId")),
        points=tuple(_point(p) for p in _require(raw, "points")),
        length=float(_require(raw, "length")),
        verified=bool(_require(raw, "verified")),
    )


def _performance(raw: dict[str, Any]) -> PerformanceAdjustedRoute:
    segments = tuple(
        PerformanceSegment(
            segment_index=int(_require(s, "segmentIndex")),
            distance_m=float(_require(s, "distanceM")),
            ground_speed_mps=float(_require(s, "groundSpeedMps")),
            energy_wh=float(_require(s, "energyWh")),
            wind_margin_mps=float(_require(s, "windMarginMps")),
        )
        for s in _require(raw, "segments")
    )
    return PerformanceAdjustedRoute(
        route_id=str(_require(raw, "routeId")),
        uav_id=str(_require(raw, "uavId")),
        segments=segments,
        total_energy_wh=float(_require(raw, "totalEnergyWh")),
        reserve_margin_wh=float(_require(raw, "reserveMarginWh")),
        verified=bool(_require(raw, "verified")),
        model_authority=str(_require(raw, "modelAuthority")),
    )


def _trajectory(raw: dict[str, Any]) -> Trajectory4D:
    points = tuple(
        TrajectoryPoint(
            point_id=str(_require(p, "pointId")),
            x=float(_require(p, "x")),
            y=float(_require(p, "y")),
            altitude_m=float(_require(p, "altitudeM")),
            timestamp_s=float(_require(p, "timestampS")),
            ground_speed_mps=float(_require(p, "groundSpeedMps")),
        )
        for p in _require(raw, "points")
    )
    return Trajectory4D(
        trajectory_id=str(_require(raw, "trajectoryId")),
        route_id=str(_require(raw, "routeId")),
        uav_id=str(_require(raw, "uavId")),
        start_time_s=float(_require(raw, "startTimeS")),
        end_time_s=float(_require(raw, "endTimeS")),
        points=points,
        verified=bool(_require(raw, "verified")),
    )



def _canonical_route_candidate(
    raw: Any, mission_id: str, *, weather_set: bool = False
) -> dict[str, Any] | None:
    """Validate a supplied WGS84 flown trajectory; never promote a graph path."""
    if raw is None:
        return None
    if not isinstance(raw, dict):
        raise RequestError("ROUTE_CANDIDATE_MUST_BE_OBJECT")
    fields = ("routeId", "routeVersion", "missionId", "missionVersion",
              "generatorId", "generatorVersion", "calculationInputVersion")
    for key in fields:
        if not isinstance(raw.get(key), str) or not raw[key].strip():
            raise RequestError(f"ROUTE_CANDIDATE_FIELD_REQUIRED:{key}")
    if raw["missionId"] != mission_id:
        raise RequestError("ROUTE_CANDIDATE_MISSION_MISMATCH")
    if raw.get("coordinateReferenceSystem") != "EPSG:4326":
        raise RequestError("ROUTE_CANDIDATE_CRS_UNSUPPORTED")
    if raw.get("trajectoryGeometryComplete") is not True:
        raise RequestError("ROUTE_CANDIDATE_COMPLETE_TRAJECTORY_REQUIRED")
    if raw.get("validationState") != "VALIDATED":
        raise RequestError("ROUTE_CANDIDATE_ROUTE_VALIDATION_REQUIRED")

    environment = raw.get("environmentSnapshotRefs")
    if not isinstance(environment, dict):
        raise RequestError("ROUTE_CANDIDATE_ENVIRONMENT_SNAPSHOT_REFS_REQUIRED")
    for key in ("terrainSnapshotId", "airspaceSnapshotId"):
        if not isinstance(environment.get(key), str) or not environment[key].strip():
            raise RequestError(f"ROUTE_CANDIDATE_ENVIRONMENT_SNAPSHOT_REQUIRED:{key}")
    weather_id = environment.get("weatherSnapshotId", "")
    wind_id = environment.get("windSnapshotId", "")
    if not isinstance(weather_id, str) or not isinstance(wind_id, str) or bool(weather_id.strip()) != bool(wind_id.strip()):
        raise RequestError("ROUTE_CANDIDATE_WEATHER_AND_WIND_SNAPSHOT_REFS_MUST_BE_PAIRED")

    import math
    points = raw.get("waypoints")
    if not isinstance(points, list) or len(points) < 2:
        raise RequestError("ROUTE_CANDIDATE_REQUIRES_TWO_WAYPOINTS")
    normalized = []
    seen = set()
    for point in points:
        if not isinstance(point, dict):
            raise RequestError("ROUTE_CANDIDATE_WAYPOINT_MUST_BE_OBJECT")
        waypoint_id = point.get("waypointId")
        if not isinstance(waypoint_id, str) or not waypoint_id.strip() or waypoint_id in seen:
            raise RequestError("ROUTE_CANDIDATE_WAYPOINT_ID_INVALID_OR_DUPLICATE")
        seen.add(waypoint_id)
        if point.get("altitudeReference") != "AGL":
            raise RequestError("ROUTE_CANDIDATE_ALTITUDE_REFERENCE_UNSUPPORTED")
        if not isinstance(point.get("mandatory"), bool):
            raise RequestError("ROUTE_CANDIDATE_MANDATORY_FLAG_INVALID")
        try:
            lat, lon, alt = (float(point[k]) for k in ("latitudeDeg", "longitudeDeg", "altitudeM"))
        except (KeyError, TypeError, ValueError) as exc:
            raise RequestError("ROUTE_CANDIDATE_WAYPOINT_NUMERIC_FIELD_INVALID") from exc
        if not all(math.isfinite(v) for v in (lat, lon, alt)):
            raise RequestError("ROUTE_CANDIDATE_WAYPOINT_NUMERIC_FIELD_INVALID")
        if not -90 <= lat <= 90 or not -180 <= lon <= 180:
            raise RequestError("ROUTE_CANDIDATE_GEODETIC_POSITION_INVALID")
        if not 0 <= alt <= 5000:
            raise RequestError("ROUTE_CANDIDATE_ALTITUDE_INVALID")
        normalized.append({
            "waypointId": waypoint_id, "latitudeDeg": lat, "longitudeDeg": lon,
            "altitudeM": alt, "mandatory": point["mandatory"], "altitudeReference": "AGL"
        })

    segments = raw.get("segments")
    if not isinstance(segments, list) or len(segments) != len(normalized) - 1:
        raise RequestError("ROUTE_CANDIDATE_COMPLETE_SEGMENT_CHAIN_REQUIRED")
    normalized_segments = []
    segment_ids = set()
    for index, segment in enumerate(segments):
        if not isinstance(segment, dict):
            raise RequestError("ROUTE_CANDIDATE_SEGMENT_MUST_BE_OBJECT")
        segment_id = segment.get("segmentId")
        if not isinstance(segment_id, str) or not segment_id.strip() or segment_id in segment_ids:
            raise RequestError("ROUTE_CANDIDATE_SEGMENT_ID_INVALID_OR_DUPLICATE")
        segment_ids.add(segment_id)
        if segment.get("fromWaypointId") != normalized[index]["waypointId"] or segment.get("toWaypointId") != normalized[index + 1]["waypointId"]:
            raise RequestError("ROUTE_CANDIDATE_SEGMENT_WAYPOINT_CHAIN_MISMATCH")
        try:
            distance = float(segment["distanceM"])
            speed = float(segment["nominalSpeedMps"])
        except (KeyError, TypeError, ValueError) as exc:
            raise RequestError("ROUTE_CANDIDATE_SEGMENT_METRIC_INVALID") from exc
        if not math.isfinite(distance) or distance <= 0 or not math.isfinite(speed) or speed <= 0:
            raise RequestError("ROUTE_CANDIDATE_SEGMENT_METRIC_INVALID")
        normalized_segments.append({
            "segmentId": segment_id,
            "fromWaypointId": segment["fromWaypointId"],
            "toWaypointId": segment["toWaypointId"],
            "distanceM": distance,
            "nominalSpeedMps": speed,
            **({"bankAngleRad": float(segment["bankAngleRad"])} if "bankAngleRad" in segment else {}),
            **({"lateralAccelerationMps2": float(segment["lateralAccelerationMps2"])} if "lateralAccelerationMps2" in segment else {}),
        })

    normalized_candidate = {
        **{key: raw[key] for key in fields},
        "environmentSnapshotRefs": {
            "terrainSnapshotId": environment["terrainSnapshotId"],
            "airspaceSnapshotId": environment["airspaceSnapshotId"],
            "weatherSnapshotId": weather_id,
            "windSnapshotId": wind_id,
        },
        "coordinateReferenceSystem": "EPSG:4326",
        "waypoints": normalized,
        "segments": normalized_segments,
        "trajectoryGeometryComplete": True,
        "validationState": "VALIDATED",
    }
    if weather_set:
        for key in ("candidateId", "flightSafetyValidationState",
                    "flightSafetyEnvelopeId", "flightSafetyVehicleId"):
            if not isinstance(raw.get(key), str) or not raw[key].strip():
                raise RequestError(f"WEATHER_CANDIDATE_FIELD_REQUIRED:{key}")
        if raw["flightSafetyValidationState"] != "ALLOWED":
            raise RequestError("WEATHER_CANDIDATE_FLIGHT_SAFETY_NOT_ALLOWED")
        for key, minimum, exclusive in (
            ("estimatedTimeS", 0.0, True),
            ("estimatedEnergyWh", 0.0, False),
            ("estimatedReserveWh", 0.0, False),
        ):
            value = raw.get(key)
            if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < minimum or (exclusive and value == minimum):
                raise RequestError(f"WEATHER_CANDIDATE_METRIC_INVALID:{key}")
        score = raw.get("objectiveScore")
        if isinstance(score, bool) or not isinstance(score, (int, float)) or not math.isfinite(score):
            raise RequestError("WEATHER_CANDIDATE_METRIC_INVALID:objectiveScore")
        normalized_candidate.update({
            key: raw[key] for key in (
                "candidateId", "estimatedTimeS", "estimatedEnergyWh",
                "estimatedReserveWh", "objectiveScore",
                "flightSafetyValidationState", "flightSafetyEnvelopeId",
                "flightSafetyVehicleId",
            )
        })
    return normalized_candidate

def _candidate_revalidator(
    trajectories: tuple[Trajectory4D, ...],
    minimums: SeparationMinimums,
) -> bool:
    if not trajectories or not all(item.verified for item in trajectories):
        return False
    report = verify_fleet(trajectories, minimums)
    return report.status == "NO_CONFLICT"


def process_request(request: dict[str, Any]) -> dict[str, Any]:
    if request.get("schemaVersion") != SCHEMA_VERSION:
        raise RequestError("UNSUPPORTED_SCHEMA_VERSION")
    if request.get("messageType") != MESSAGE_TYPE:
        raise RequestError("UNSUPPORTED_MESSAGE_TYPE")

    mission_id = str(_require(request, "missionId"))
    result_id = str(_require(request, "resultId"))
    raw = _require(request, "inputs")
    route_candidate = _canonical_route_candidate(raw.get("routeCandidate"), mission_id)
    raw_candidates = raw.get("routeCandidates")
    route_candidates = None
    if raw_candidates is not None:
        if not isinstance(raw_candidates, list) or not 1 <= len(raw_candidates) <= 64:
            raise RequestError("WEATHER_ROUTE_CANDIDATE_SET_SIZE_MUST_BE_1_TO_64")
        route_candidates = [
            _canonical_route_candidate(item, mission_id, weather_set=True)
            for item in raw_candidates
        ]
        candidate_ids = [item["candidateId"] for item in route_candidates]
        route_ids = [item["routeId"] for item in route_candidates]
        if len(set(candidate_ids)) != len(candidate_ids) or len(set(route_ids)) != len(route_ids):
            raise RequestError("WEATHER_CANDIDATE_OR_ROUTE_ID_DUPLICATE")
        lineage = {
            (item["missionVersion"], item["calculationInputVersion"],
             item["environmentSnapshotRefs"]["terrainSnapshotId"],
             item["environmentSnapshotRefs"]["airspaceSnapshotId"],
             item["flightSafetyEnvelopeId"], item["flightSafetyVehicleId"])
            for item in route_candidates
        }
        if len(lineage) != 1:
            raise RequestError("WEATHER_CANDIDATE_SET_LINEAGE_OR_SAFETY_MISMATCH")
        objective_priorities = raw.get("objectivePriorities", [])
        supported_objectives = {"completion_time", "time", "fast", "energy", "endurance", "reserve"}
        if not isinstance(objective_priorities, list) or not objective_priorities or any(
            item not in supported_objectives for item in objective_priorities
        ):
            raise RequestError("WEATHER_CANDIDATE_OBJECTIVE_PRIORITIES_REQUIRED_OR_UNSUPPORTED")
        selected_candidate_id = raw.get("selectedCandidateId", "")
        if selected_candidate_id and selected_candidate_id not in candidate_ids:
            raise RequestError("SELECTED_WEATHER_CANDIDATE_NOT_IN_SET")
    else:
        objective_priorities = []
        selected_candidate_id = ""

    routes = tuple(_route(item) for item in _require(raw, "routes"))
    performance = tuple(_performance(item) for item in _require(raw, "performance"))
    trajectories = tuple(_trajectory(item) for item in _require(raw, "trajectories"))

    minimums_raw = _require(raw, "minimums")
    minimums = SeparationMinimums(
        float(_require(minimums_raw, "horizontalM")),
        float(_require(minimums_raw, "verticalM")),
    )

    policy_raw = raw.get("resolutionPolicy", {})
    policy = ResolutionPolicy(
        max_delay_s=float(policy_raw.get("maxDelayS", 5.0)),
        allow_vertical_correction=bool(policy_raw.get("allowVerticalCorrection", False)),
        vertical_correction_m=float(policy_raw.get("verticalCorrectionM", 0.0)),
    )

    inputs = PipelineInputs(
        zone_status=str(_require(raw, "zoneStatus")),
        assignment_status=str(_require(raw, "assignmentStatus")),
        routes=routes,
        performance=performance,
        trajectories=trajectories,
    )
    result = run_multi_uav_pipeline(
        inputs,
        minimums,
        policy,
        candidate_revalidator=lambda candidate: _candidate_revalidator(candidate, minimums),
    )

    mapping = None
    mapping_raw = raw.get("threeDMapping")
    if mapping_raw is not None:
        mapping = build_three_d_mapping_result(
            routes=routes,
            performance=performance,
            trajectories=trajectories,
            release_status=result.final_gate.release_status,
            required_reserve_wh=mapping_raw.get("requiredReserveWh"),
            line_spacing_m=mapping_raw.get("lineSpacingM"),
            line_count=mapping_raw.get("lineCount"),
            expected_frames=mapping_raw.get("expectedFrames"),
            expected_coverage_percent=mapping_raw.get("expectedCoveragePercent"),
            data_volume_mb=mapping_raw.get("dataVolumeMb"),
        )

    source = request.get("source", {})
    trace = request.get("traceability", {})
    message = build_result_message(
        mission_id=mission_id,
        result_id=result_id,
        pipeline_result=result,
        three_d_mapping=mapping,
        planner_version=source.get("plannerVersion"),
        configuration_version=source.get("configurationVersion"),
        run_id=source.get("runId"),
        input_refs=tuple(trace.get("inputRefs", ())),
        artifact_refs=tuple(trace.get("artifactRefs", ())),
        evidence_refs=tuple(trace.get("evidenceRefs", ())),
        route_candidate=route_candidate,
        route_candidates=route_candidates,
        selected_candidate_id=selected_candidate_id,
        objective_priorities=objective_priorities,
    )
    return message


def main() -> int:
    for raw_line in sys.stdin:
        line = raw_line.strip()
        if not line:
            continue
        try:
            request = json.loads(line)
            response = process_request(request)
            sys.stdout.write(dumps_result_message(response) + "\n")
            sys.stdout.flush()
        except Exception as exc:
            print(f"PLANNING_CORE_ERROR:{type(exc).__name__}:{exc}", file=sys.stderr, flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
