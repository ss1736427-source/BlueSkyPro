"""4D conflict verification for UAV trajectories."""

from __future__ import annotations

from dataclasses import dataclass
from math import hypot
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


def _sample_times(a: Trajectory4D, b: Trajectory4D) -> tuple[float, ...]:
    times = {
        p.timestamp_s
        for p in a.points
        if b.start_time_s <= p.timestamp_s <= b.end_time_s
    }
    times.update(
        p.timestamp_s
        for p in b.points
        if a.start_time_s <= p.timestamp_s <= a.end_time_s
    )
    return tuple(sorted(times))


def _interpolate(trajectory: Trajectory4D, timestamp_s: float) -> tuple[float, float, float]:
    points = trajectory.points
    if timestamp_s < trajectory.start_time_s or timestamp_s > trajectory.end_time_s:
        raise ConflictVerificationError("TIME_OUTSIDE_TRAJECTORY")

    for left, right in zip(points, points[1:]):
        if left.timestamp_s <= timestamp_s <= right.timestamp_s:
            span = right.timestamp_s - left.timestamp_s
            ratio = 0.0 if span == 0 else (timestamp_s - left.timestamp_s) / span
            return (
                left.x + (right.x - left.x) * ratio,
                left.y + (right.y - left.y) * ratio,
                left.altitude_m + (right.altitude_m - left.altitude_m) * ratio,
            )

    if timestamp_s == points[-1].timestamp_s:
        p = points[-1]
        return p.x, p.y, p.altitude_m

    raise ConflictVerificationError("TIME_INTERPOLATION_FAILED")


def verify_pair(
    a: Trajectory4D,
    b: Trajectory4D,
    minimums: SeparationMinimums,
) -> tuple[Conflict, ...]:
    if not a.verified or not b.verified:
        raise ConflictVerificationError("TRAJECTORY_NOT_VERIFIED")
    if a.uav_id == b.uav_id:
        raise ConflictVerificationError("SELF_CONFLICT_PAIR")

    samples = _sample_times(a, b)
    if not samples:
        return ()

    conflicts: list[Conflict] = []
    active_start: float | None = None
    min_h = float("inf")
    min_v = float("inf")
    last_conflict_time: float | None = None

    for t in samples:
        ax, ay, az = _interpolate(a, t)
        bx, by, bz = _interpolate(b, t)
        horizontal = hypot(ax - bx, ay - by)
        vertical = abs(az - bz)
        violates = (
            horizontal < minimums.horizontal_m
            and vertical < minimums.vertical_m
        )

        if violates:
            if active_start is None:
                active_start = t
                min_h = horizontal
                min_v = vertical
            else:
                min_h = min(min_h, horizontal)
                min_v = min(min_v, vertical)
            last_conflict_time = t
        elif active_start is not None:
            conflicts.append(
                Conflict(
                    conflict_id=f"CONFLICT:{a.uav_id}:{b.uav_id}:{active_start:.3f}",
                    uav_a=a.uav_id,
                    uav_b=b.uav_id,
                    time_start_s=active_start,
                    time_end_s=last_conflict_time if last_conflict_time is not None else active_start,
                    minimum_horizontal_m=min_h,
                    minimum_vertical_m=min_v,
                    classification="CONFLICT",
                )
            )
            active_start = None
            min_h = float("inf")
            min_v = float("inf")
            last_conflict_time = None

    if active_start is not None:
        conflicts.append(
            Conflict(
                conflict_id=f"CONFLICT:{a.uav_id}:{b.uav_id}:{active_start:.3f}",
                uav_a=a.uav_id,
                uav_b=b.uav_id,
                time_start_s=active_start,
                time_end_s=last_conflict_time if last_conflict_time is not None else active_start,
                minimum_horizontal_m=min_h,
                minimum_vertical_m=min_v,
                classification="CONFLICT",
            )
        )

    return tuple(conflicts)


def verify_fleet(
    trajectories: Sequence[Trajectory4D],
    minimums: SeparationMinimums,
) -> ConflictReport:
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
