"""Pre-execution ground conflict resolution for 4D UAV plans."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, Sequence

from conflict_4d import Conflict, ConflictReport, SeparationMinimums, verify_fleet
from trajectory_4d import Trajectory4D


@dataclass(frozen=True)
class ResolutionPolicy:
    max_delay_s: float = 5.0
    allow_vertical_correction: bool = False
    vertical_correction_m: float = 0.0


@dataclass(frozen=True)
class ResolutionResult:
    status: str
    method: str | None
    affected_uav_id: str | None
    delay_s: float
    vertical_correction_m: float
    rejected_candidates: tuple[str, ...]
    conflict_report: ConflictReport


class ResolutionError(ValueError):
    pass


def _delay_trajectory(trajectory: Trajectory4D, delay_s: float) -> Trajectory4D:
    if delay_s < 0:
        raise ResolutionError("NEGATIVE_DELAY")
    return Trajectory4D(
        trajectory_id=trajectory.trajectory_id,
        route_id=trajectory.route_id,
        uav_id=trajectory.uav_id,
        start_time_s=trajectory.start_time_s + delay_s,
        end_time_s=trajectory.end_time_s + delay_s,
        points=tuple(
            type(point)(
                point_id=point.point_id,
                x=point.x,
                y=point.y,
                altitude_m=point.altitude_m,
                timestamp_s=point.timestamp_s + delay_s,
                ground_speed_mps=point.ground_speed_mps,
            )
            for point in trajectory.points
        ),
        verified=trajectory.verified,
    )


def _vertical_trajectory(
    trajectory: Trajectory4D,
    correction_m: float,
) -> Trajectory4D:
    if correction_m == 0:
        raise ResolutionError("ZERO_VERTICAL_CORRECTION")
    return Trajectory4D(
        trajectory_id=trajectory.trajectory_id,
        route_id=trajectory.route_id,
        uav_id=trajectory.uav_id,
        start_time_s=trajectory.start_time_s,
        end_time_s=trajectory.end_time_s,
        points=tuple(
            type(point)(
                point_id=point.point_id,
                x=point.x,
                y=point.y,
                altitude_m=point.altitude_m + correction_m,
                timestamp_s=point.timestamp_s,
                ground_speed_mps=point.ground_speed_mps,
            )
            for point in trajectory.points
        ),
        verified=trajectory.verified,
    )


def _replace_trajectory(
    trajectories: Sequence[Trajectory4D],
    replacement: Trajectory4D,
) -> tuple[Trajectory4D, ...]:
    return tuple(
        replacement if item.uav_id == replacement.uav_id else item
        for item in trajectories
    )


def resolve_conflicts(
    trajectories: Sequence[Trajectory4D],
    minimums: SeparationMinimums,
    policy: ResolutionPolicy,
    *,
    spatial_regenerator: Callable[[Conflict], Sequence[Trajectory4D]] | None = None,
) -> ResolutionResult:
    initial = verify_fleet(trajectories, minimums)
    if initial.status == "NO_CONFLICT":
        return ResolutionResult(
            "NO_ACTION",
            None,
            None,
            0.0,
            0.0,
            (),
            initial,
        )

    rejected: list[str] = []

    # Priority 1: regenerate affected trajectories spatially.
    if spatial_regenerator is not None:
        for conflict in initial.conflicts:
            regenerated = tuple(spatial_regenerator(conflict))
            if regenerated:
                candidate = tuple(
                    item for item in regenerated
                    if item.uav_id in {conflict.uav_a, conflict.uav_b}
                )
                if candidate:
                    combined = tuple(
                        candidate_item
                        if any(
                            existing.uav_id == candidate_item.uav_id
                            for existing in trajectories
                        )
                        else existing
                        for existing in trajectories
                        for candidate_item in ()
                    )
                    # Explicit replacement without implicit list mutation.
                    updated = list(trajectories)
                    for replacement in candidate:
                        updated = [
                            replacement if item.uav_id == replacement.uav_id else item
                            for item in updated
                        ]
                    report = verify_fleet(tuple(updated), minimums)
                    if report.status == "NO_CONFLICT":
                        return ResolutionResult(
                            "RESOLVED",
                            "SPATIAL_REGENERATION",
                            conflict.uav_b,
                            0.0,
                            0.0,
                            tuple(rejected),
                            report,
                        )
                rejected.append(f"SPATIAL_REGENERATION:{conflict.conflict_id}")

    # Priority 2: deterministic delay candidates 0..5 s.
    for delay in range(0, int(policy.max_delay_s) + 1):
        if delay == 0:
            continue
        for conflict in initial.conflicts:
            target = next(
                (t for t in trajectories if t.uav_id == conflict.uav_b),
                None,
            )
            if target is None:
                continue
            candidate_trajectories = _replace_trajectory(
                trajectories,
                _delay_trajectory(target, float(delay)),
            )
            report = verify_fleet(candidate_trajectories, minimums)
            if report.status == "NO_CONFLICT":
                return ResolutionResult(
                    "RESOLVED",
                    "TEMPORAL_DELAY",
                    target.uav_id,
                    float(delay),
                    0.0,
                    tuple(rejected),
                    report,
                )
        rejected.append(f"TEMPORAL_DELAY:{delay}s")

    # Priority 3: vertical correction only when explicitly permitted.
    if policy.allow_vertical_correction:
        if policy.vertical_correction_m == 0:
            raise ResolutionError("VERTICAL_CORRECTION_NOT_DEFINED")
        for conflict in initial.conflicts:
            target = next(
                (t for t in trajectories if t.uav_id == conflict.uav_b),
                None,
            )
            if target is None:
                continue
            candidate_trajectories = _replace_trajectory(
                trajectories,
                _vertical_trajectory(target, policy.vertical_correction_m),
            )
            report = verify_fleet(candidate_trajectories, minimums)
            if report.status == "NO_CONFLICT":
                return ResolutionResult(
                    "RESOLVED",
                    "VERTICAL_CORRECTION",
                    target.uav_id,
                    0.0,
                    policy.vertical_correction_m,
                    tuple(rejected),
                    report,
                )

    return ResolutionResult(
        "UNRESOLVED",
        None,
        None,
        0.0,
        0.0,
        tuple(rejected),
        initial,
    )
