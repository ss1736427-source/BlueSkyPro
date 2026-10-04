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
class RevalidationReport:
    status: str
    failed_checks: tuple[str, ...] = ()
    model_ids: tuple[str, ...] = ()
    input_snapshot_id: str = ""
    candidate_snapshot_id: str = ""
    provenance: str = ""

    @property
    def accepted(self) -> bool:
        return self.status == "PASS" and not self.failed_checks


@dataclass(frozen=True)
class ResolutionResult:
    status: str
    method: str | None
    affected_uav_id: str | None
    delay_s: float
    vertical_correction_m: float
    rejected_candidates: tuple[str, ...]
    conflict_report: ConflictReport
    revalidation: RevalidationReport | None
    resolved_trajectories: tuple[Trajectory4D, ...] = ()


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
    candidate_validator: Callable[[Sequence[Trajectory4D]], RevalidationReport] | None = None,
) -> ResolutionResult:
    initial = verify_fleet(trajectories, minimums)
    if initial.status == "NO_CONFLICT":
        return ResolutionResult(
            "NO_ACTION", None, None, 0.0, 0.0, (), initial, None, tuple(trajectories)
        )

    rejected: list[str] = []

    def accept_candidate(
        method: str,
        affected_uav_id: str,
        candidate_trajectories: tuple[Trajectory4D, ...],
        delay_s: float = 0.0,
        vertical_correction_m: float = 0.0,
    ) -> ResolutionResult | None:
        report = verify_fleet(candidate_trajectories, minimums)
        if report.status != "NO_CONFLICT":
            return None
        if candidate_validator is None:
            rejected.append(f"{method}:AUTHORITATIVE_REVALIDATION_REQUIRED")
            return None
        revalidation = candidate_validator(candidate_trajectories)
        if not revalidation.accepted:
            rejected.append(f"{method}:AUTHORITATIVE_REVALIDATION_FAILED")
            return None
        return ResolutionResult(
            "RESOLVED",
            method,
            affected_uav_id,
            delay_s,
            vertical_correction_m,
            tuple(rejected),
            report,
            revalidation,
            candidate_trajectories,
        )

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
                    updated = list(trajectories)
                    for replacement in candidate:
                        updated = [
                            replacement if item.uav_id == replacement.uav_id else item
                            for item in updated
                        ]
                    result = accept_candidate(
                        "SPATIAL_REGENERATION",
                        conflict.uav_b,
                        tuple(updated),
                    )
                    if result is not None:
                        return result
                rejected.append(f"SPATIAL_REGENERATION:{conflict.conflict_id}")

    # Priority 2: deterministic delay candidates 1..max_delay_s.
    for delay in range(1, int(policy.max_delay_s) + 1):
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
            result = accept_candidate(
                "TEMPORAL_DELAY",
                target.uav_id,
                candidate_trajectories,
                delay_s=float(delay),
            )
            if result is not None:
                return result
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
            result = accept_candidate(
                "VERTICAL_CORRECTION",
                target.uav_id,
                candidate_trajectories,
                vertical_correction_m=policy.vertical_correction_m,
            )
            if result is not None:
                return result

    return ResolutionResult(
        "UNRESOLVED",
        None,
        None,
        0.0,
        0.0,
        tuple(rejected),
        initial,
        None,
        (),
    )
