"""Pre-execution ground conflict resolution for 4D UAV plans.

Conflict detection remains authoritative and continuous-time. Resolution searches
a finite, deterministic temporal-delay lattice, evaluates combinations across all
UAVs participating in the initial conflict graph, and accepts only candidates
that pass both 4D separation verification and authoritative revalidation.
"""

from __future__ import annotations

from dataclasses import dataclass
from heapq import heappop, heappush
from typing import Callable, Iterator, Sequence

from conflict_4d import Conflict, ConflictReport, SeparationMinimums, verify_fleet
from trajectory_4d import Trajectory4D


@dataclass(frozen=True)
class ResolutionPolicy:
    max_delay_s: float = 5.0
    delay_step_s: float = 0.5
    max_delay_search_states: int = 100_000
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
        return (
            self.status == "PASS"
            and not self.failed_checks
            and bool(self.model_ids)
            and bool(self.input_snapshot_id)
            and bool(self.candidate_snapshot_id)
            and bool(self.provenance)
        )


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
    delay_assignments: tuple[tuple[str, float], ...] = ()


class ResolutionError(ValueError):
    pass


def _validate_policy(policy: ResolutionPolicy) -> None:
    if policy.max_delay_s < 0:
        raise ResolutionError("NEGATIVE_MAX_DELAY")
    if policy.delay_step_s <= 0:
        raise ResolutionError("NON_POSITIVE_DELAY_STEP")
    if policy.max_delay_search_states <= 0:
        raise ResolutionError("NON_POSITIVE_SEARCH_STATE_LIMIT")

    steps = policy.max_delay_s / policy.delay_step_s
    if abs(steps - round(steps)) > 1e-9:
        raise ResolutionError("DELAY_GRID_NOT_ALIGNED")


def _delay_values(policy: ResolutionPolicy) -> tuple[float, ...]:
    count = int(round(policy.max_delay_s / policy.delay_step_s))
    return tuple(
        round(index * policy.delay_step_s, 10)
        for index in range(count + 1)
    )


def _iter_delay_assignments(
    uav_ids: Sequence[str],
    values: Sequence[float],
) -> Iterator[tuple[float, tuple[float, ...]]]:
    """Enumerate the finite delay lattice in deterministic cost order.

    Cost is total delay. Ties are resolved lexicographically by the stable,
    sorted UAV ID order. The all-zero assignment is omitted.
    """
    if not uav_ids:
        return

    dimensions = len(uav_ids)
    last_index = len(values) - 1
    initial = (0,) * dimensions
    queue: list[tuple[int, tuple[int, ...]]] = [(0, initial)]
    visited = {initial}

    while queue:
        _, state = heappop(queue)
        delays = tuple(values[index] for index in state)
        total = sum(delays)
        if total > 0:
            yield total, delays

        for dimension in range(dimensions):
            if state[dimension] >= last_index:
                continue
            next_state = list(state)
            next_state[dimension] += 1
            next_tuple = tuple(next_state)
            if next_tuple in visited:
                continue
            visited.add(next_tuple)
            heappush(
                queue,
                (sum(next_tuple), next_tuple),
            )


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


def _replace_trajectories(
    trajectories: Sequence[Trajectory4D],
    replacements: Sequence[Trajectory4D],
) -> tuple[Trajectory4D, ...]:
    replacement_by_uav = {item.uav_id: item for item in replacements}
    return tuple(
        replacement_by_uav.get(item.uav_id, item)
        for item in trajectories
    )


def _changed_uavs(
    uav_ids: Sequence[str],
    delays: Sequence[float],
) -> tuple[str, ...]:
    return tuple(
        uav_id
        for uav_id, delay in zip(uav_ids, delays)
        if delay > 0
    )


def resolve_conflicts(
    trajectories: Sequence[Trajectory4D],
    minimums: SeparationMinimums,
    policy: ResolutionPolicy,
    *,
    spatial_regenerator: Callable[[Conflict], Sequence[Trajectory4D]] | None = None,
    candidate_validator: Callable[[Sequence[Trajectory4D]], RevalidationReport] | None = None,
) -> ResolutionResult:
    _validate_policy(policy)

    initial = verify_fleet(trajectories, minimums)
    if initial.status == "NO_CONFLICT":
        return ResolutionResult(
            "NO_ACTION", None, None, 0.0, 0.0, (), initial, None,
            tuple(trajectories), ()
        )

    rejected: list[str] = []

    def accept_candidate(
        method: str,
        candidate_trajectories: tuple[Trajectory4D, ...],
        *,
        delay_s: float = 0.0,
        delay_assignments: tuple[tuple[str, float], ...] = (),
        affected_uav_id: str | None = None,
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
            delay_assignments,
        )

    # Priority 1: regenerate affected trajectories spatially.
    if spatial_regenerator is not None:
        for conflict in initial.conflicts:
            regenerated = tuple(spatial_regenerator(conflict))
            if regenerated:
                candidate = tuple(
                    item
                    for item in regenerated
                    if item.uav_id in {conflict.uav_a, conflict.uav_b}
                )
                if candidate:
                    updated = list(trajectories)
                    for replacement in candidate:
                        updated = [
                            replacement
                            if item.uav_id == replacement.uav_id
                            else item
                            for item in updated
                        ]
                    result = accept_candidate(
                        "SPATIAL_REGENERATION",
                        tuple(updated),
                        affected_uav_id=conflict.uav_b,
                    )
                    if result is not None:
                        return result
                rejected.append(f"SPATIAL_REGENERATION:{conflict.conflict_id}")

    # Priority 2: deterministic exhaustive search over the configured finite
    # delay lattice. All UAVs in the initial conflict graph are decision
    # variables; this prevents the resolver from assuming that only UAV-B may
    # move. A feasible candidate is still accepted only after full revalidation.
    affected_uav_ids = tuple(
        sorted({uav_id for conflict in initial.conflicts for uav_id in (conflict.uav_a, conflict.uav_b)})
    )
    values = _delay_values(policy)
    total_states = len(values) ** len(affected_uav_ids) - 1
    search_limit = policy.max_delay_search_states

    if total_states > search_limit:
        rejected.append(
            f"TEMPORAL_DELAY:SEARCH_SPACE_EXCEEDS_LIMIT:{total_states}>{search_limit}"
        )

    searched = 0
    for total_delay, delays in _iter_delay_assignments(affected_uav_ids, values):
        searched += 1
        if searched > search_limit:
            rejected.append("TEMPORAL_DELAY:SEARCH_LIMIT_REACHED")
            break

        assignments = tuple(
            (uav_id, delay)
            for uav_id, delay in zip(affected_uav_ids, delays)
            if delay > 0
        )
        replacements = tuple(
            _delay_trajectory(
                next(item for item in trajectories if item.uav_id == uav_id),
                delay,
            )
            for uav_id, delay in assignments
        )
        candidate_trajectories = _replace_trajectories(trajectories, replacements)

        affected_id = assignments[0][0] if len(assignments) == 1 else None
        result = accept_candidate(
            "TEMPORAL_DELAY" if len(assignments) == 1 else "TEMPORAL_DELAY_COMBINATION",
            candidate_trajectories,
            delay_s=max(delay for _, delay in assignments),
            delay_assignments=assignments,
            affected_uav_id=affected_id,
        )
        if result is not None:
            return result

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
            candidate_trajectories = _replace_trajectories(
                trajectories,
                (_vertical_trajectory(target, policy.vertical_correction_m),),
            )
            result = accept_candidate(
                "VERTICAL_CORRECTION",
                candidate_trajectories,
                affected_uav_id=target.uav_id,
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
        (),
    )
