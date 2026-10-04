"""4D conflict verification for UAV trajectories.

The verifier evaluates piecewise-linear 4D trajectories continuously over time.
No fixed sampling interval is used for the safety decision.
"""

from __future__ import annotations

from dataclasses import dataclass
from math import hypot, isfinite, sqrt
from typing import Sequence

from trajectory_4d import Trajectory4D


@dataclass(frozen=True)
class SeparationMinimums:
    horizontal_m: float
    vertical_m: float


@dataclass(frozen=True)
class Conflict:
    conflict_id: str
    uav_a: str
    uav_b: str
    time_start_s: float
    time_end_s: float
    minimum_horizontal_m: float
    minimum_vertical_m: float
    classification: str


@dataclass(frozen=True)
class ConflictReport:
    status: str
    conflicts: tuple[Conflict, ...]
    checked_pairs: int


class ConflictVerificationError(ValueError):
    pass


def _validate_minimums(minimums: SeparationMinimums) -> None:
    if minimums.horizontal_m <= 0 or minimums.vertical_m <= 0:
        raise ConflictVerificationError("INVALID_SEPARATION_MINIMUMS")


def _segment_state(trajectory: Trajectory4D, timestamp_s: float) -> tuple[float, float, float, float, float, float]:
    """Return position and velocity at the start of the containing segment."""
    points = trajectory.points
    for left, right in zip(points, points[1:]):
        if left.timestamp_s <= timestamp_s <= right.timestamp_s:
            span = right.timestamp_s - left.timestamp_s
            if span <= 0:
                raise ConflictVerificationError("NON_POSITIVE_SEGMENT_TIME")
            return (
                left.x,
                left.y,
                left.altitude_m,
                (right.x - left.x) / span,
                (right.y - left.y) / span,
                (right.altitude_m - left.altitude_m) / span,
            )
    raise ConflictVerificationError("TIME_SEGMENT_NOT_FOUND")


def _append_unique(values: list[float], value: float, lo: float, hi: float) -> None:
    if lo <= value <= hi and isfinite(value):
        values.append(value)


def _vertical_valid_interval(
    z0: float,
    vz: float,
    lo: float,
    hi: float,
    limit: float,
) -> tuple[float, float] | None:
    """Return the interval where |z0+vz*t| < limit.

    t is measured from the beginning of the overlapping segment interval.
    """
    if limit <= 0:
        return None
    if vz == 0:
        return (lo, hi) if abs(z0) < limit else None

    roots = ((-limit - z0) / vz, (limit - z0) / vz)
    left, right = sorted(roots)
    valid_lo = max(lo, left)
    valid_hi = min(hi, right)
    if valid_lo >= valid_hi:
        return None

    # Probe the open interval to respect the strict separation condition.
    probe = (valid_lo + valid_hi) / 2.0
    if abs(z0 + vz * probe) >= limit:
        return None
    return valid_lo, valid_hi


def _horizontal_conflict_interval(
    x0: float,
    y0: float,
    vx: float,
    vy: float,
    lo: float,
    hi: float,
    limit: float,
) -> tuple[float, float] | None:
    """Return the interval where horizontal distance is strictly below limit."""
    a = vx * vx + vy * vy
    b = 2.0 * (x0 * vx + y0 * vy)
    c = x0 * x0 + y0 * y0 - limit * limit

    if a == 0:
        return (lo, hi) if c < 0 else None

    vertex = -b / (2.0 * a)
    q_vertex = a * vertex * vertex + b * vertex + c
    if q_vertex >= 0:
        return None

    radius = sqrt(max(0.0, -q_vertex / a))
    return max(lo, vertex - radius), min(hi, vertex + radius)


def _distance_minima(
    x0: float,
    y0: float,
    vx: float,
    vy: float,
    z0: float,
    vz: float,
    lo: float,
    hi: float,
) -> tuple[float, float]:
    """Compute exact minima over a closed conflict interval."""
    candidates = [lo, hi]
    horizontal_speed_sq = vx * vx + vy * vy
    if horizontal_speed_sq > 0:
        vertex = -(x0 * vx + y0 * vy) / horizontal_speed_sq
        if lo <= vertex <= hi:
            candidates.append(vertex)

    vertical_zero = -z0 / vz if vz != 0 else None
    if vertical_zero is not None and lo <= vertical_zero <= hi:
        candidates.append(vertical_zero)

    min_h = float("inf")
    min_v = float("inf")
    for t in candidates:
        min_h = min(min_h, hypot(x0 + vx * t, y0 + vy * t))
        min_v = min(min_v, abs(z0 + vz * t))
    return min_h, min_v


def _continuous_pair_conflicts(
    a: Trajectory4D,
    b: Trajectory4D,
    minimums: SeparationMinimums,
) -> tuple[Conflict, ...]:
    conflicts: list[Conflict] = []

    common_start = max(a.start_time_s, b.start_time_s)
    common_end = min(a.end_time_s, b.end_time_s)
    if common_start >= common_end:
        return ()

    breakpoints = {
        common_start,
        common_end,
        *(p.timestamp_s for p in a.points if common_start < p.timestamp_s < common_end),
        *(p.timestamp_s for p in b.points if common_start < p.timestamp_s < common_end),
    }
    ordered = sorted(breakpoints)

    for interval_start, interval_end in zip(ordered, ordered[1:]):
        ax, ay, az, avx, avy, avz = _segment_state(a, interval_start)
        bx, by, bz, bvx, bvy, bvz = _segment_state(b, interval_start)

        dt = interval_end - interval_start
        x0 = (ax - bx)
        y0 = (ay - by)
        z0 = (az - bz)
        vx = avx - bvx
        vy = avy - bvy
        vz = avz - bvz

        vertical = _vertical_valid_interval(z0, vz, 0.0, dt, minimums.vertical_m)
        if vertical is None:
            continue

        horizontal = _horizontal_conflict_interval(
            x0, y0, vx, vy, vertical[0], vertical[1], minimums.horizontal_m
        )
        if horizontal is None or horizontal[0] >= horizontal[1]:
            continue

        min_h, min_v = _distance_minima(
            x0, y0, vx, vy, z0, vz, horizontal[0], horizontal[1]
        )
        start_abs = interval_start + horizontal[0]
        end_abs = interval_start + horizontal[1]
        conflicts.append(
            Conflict(
                conflict_id=f"CONFLICT:{a.uav_id}:{b.uav_id}:{start_abs:.6f}",
                uav_a=a.uav_id,
                uav_b=b.uav_id,
                time_start_s=start_abs,
                time_end_s=end_abs,
                minimum_horizontal_m=min_h,
                minimum_vertical_m=min_v,
                classification="CONFLICT",
            )
        )

    if not conflicts:
        return ()

    # Merge adjacent conflict intervals created at trajectory segment boundaries.
    merged: list[Conflict] = [conflicts[0]]
    for current in conflicts[1:]:
        previous = merged[-1]
        if current.time_start_s <= previous.time_end_s + 1e-9:
            merged[-1] = Conflict(
                conflict_id=previous.conflict_id,
                uav_a=previous.uav_a,
                uav_b=previous.uav_b,
                time_start_s=previous.time_start_s,
                time_end_s=max(previous.time_end_s, current.time_end_s),
                minimum_horizontal_m=min(previous.minimum_horizontal_m, current.minimum_horizontal_m),
                minimum_vertical_m=min(previous.minimum_vertical_m, current.minimum_vertical_m),
                classification="CONFLICT",
            )
        else:
            merged.append(current)
    return tuple(merged)


def verify_pair(
    a: Trajectory4D,
    b: Trajectory4D,
    minimums: SeparationMinimums,
) -> tuple[Conflict, ...]:
    _validate_minimums(minimums)
    if not a.verified or not b.verified:
        raise ConflictVerificationError("TRAJECTORY_NOT_VERIFIED")
    if a.uav_id == b.uav_id:
        raise ConflictVerificationError("SELF_CONFLICT_PAIR")
    return _continuous_pair_conflicts(a, b, minimums)


def verify_fleet(
    trajectories: Sequence[Trajectory4D],
    minimums: SeparationMinimums,
) -> ConflictReport:
    _validate_minimums(minimums)
    conflicts: list[Conflict] = []
    checked_pairs = 0

    for index, left in enumerate(trajectories):
        for right in trajectories[index + 1:]:
            checked_pairs += 1
            conflicts.extend(verify_pair(left, right, minimums))

    return ConflictReport(
        status="CONFLICT" if conflicts else "NO_CONFLICT",
        conflicts=tuple(conflicts),
        checked_pairs=checked_pairs,
    )
