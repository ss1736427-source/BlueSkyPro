"""End-to-end orchestration of the Multi-UAV planning pipeline.

The orchestrator composes domain stages; it does not replace their safety checks.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence

from conflict_4d import ConflictReport, SeparationMinimums, verify_fleet
from conflict_resolution import ResolutionPolicy, ResolutionResult, resolve_conflicts
from final_gate import FinalGateResult, GateInput, evaluate_final_gate
from route_in_zone import Route
from trajectory_4d import Trajectory4D
from wind_performance import PerformanceAdjustedRoute


@dataclass(frozen=True)
class PipelineInputs:
    zone_status: str
    assignment_status: str
    routes: tuple[Route, ...]
    performance: tuple[PerformanceAdjustedRoute, ...]
    trajectories: tuple[Trajectory4D, ...]


@dataclass(frozen=True)
class PipelineResult:
    status: str
    conflict: ConflictReport
    resolution: ResolutionResult
    final_gate: FinalGateResult


class OrchestrationError(ValueError):
    pass


def run_multi_uav_pipeline(
    inputs: PipelineInputs,
    minimums: SeparationMinimums,
    resolution_policy: ResolutionPolicy,
) -> PipelineResult:
    if inputs.zone_status != "VERIFIED":
        raise OrchestrationError("ZONE_SET_NOT_VERIFIED")
    if inputs.assignment_status != "VERIFIED":
        raise OrchestrationError("ASSIGNMENT_NOT_VERIFIED")
    if not inputs.routes:
        raise OrchestrationError("NO_ROUTES")
    if len(inputs.routes) != len(inputs.performance):
        raise OrchestrationError("ROUTE_PERFORMANCE_COUNT_MISMATCH")
    if len(inputs.performance) != len(inputs.trajectories):
        raise OrchestrationError("PERFORMANCE_TRAJECTORY_COUNT_MISMATCH")

    if not all(route.verified for route in inputs.routes):
        raise OrchestrationError("ROUTE_NOT_VERIFIED")
    if not all(item.verified for item in inputs.performance):
        raise OrchestrationError("PERFORMANCE_NOT_VERIFIED")
    if not all(item.verified for item in inputs.trajectories):
        raise OrchestrationError("TRAJECTORY_NOT_VERIFIED")

    conflict = verify_fleet(inputs.trajectories, minimums)
    resolution = resolve_conflicts(
        inputs.trajectories,
        minimums,
        resolution_policy,
    )

    final_conflict = resolution.conflict_report
    resolution_status = resolution.status

    final_gate = evaluate_final_gate(
        (
            GateInput("ZONE_SET", inputs.zone_status),
            GateInput("ASSIGNMENT", inputs.assignment_status),
            GateInput("ROUTE", "VERIFIED"),
            GateInput("PERFORMANCE", "VERIFIED"),
            GateInput("TRAJECTORY", "VERIFIED"),
            GateInput("CONFLICT", final_conflict.status),
            GateInput("RESOLUTION", resolution_status),
        ),
        unresolved_conflicts=len(final_conflict.conflicts),
    )

    return PipelineResult(
        status=final_gate.release_status,
        conflict=conflict,
        resolution=resolution,
        final_gate=final_gate,
    )
