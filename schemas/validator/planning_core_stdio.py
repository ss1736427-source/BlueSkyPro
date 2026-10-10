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
