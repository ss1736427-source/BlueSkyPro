"""4D trajectory generation from performance-adjusted routes."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence

from wind_performance import PerformanceAdjustedRoute


@dataclass(frozen=True)
class TrajectoryPoint:
    point_id: str
    x: float
    y: float
    altitude_m: float
    timestamp_s: float
    ground_speed_mps: float


@dataclass(frozen=True)
class Trajectory4D:
    trajectory_id: str
    route_id: str
    uav_id: str
    start_time_s: float
    end_time_s: float
    points: tuple[TrajectoryPoint, ...]
    verified: bool


class TrajectoryError(ValueError):
    pass


def build_trajectory_4d(
    *,
    trajectory_id: str,
    route_id: str,
    uav_id: str,
    route_points: Sequence[object],
    performance: PerformanceAdjustedRoute,
    start_time_s: float,
    altitude_m: float,
) -> Trajectory4D:
    if not performance.verified:
        raise TrajectoryError("PERFORMANCE_NOT_VERIFIED")
    if len(route_points) != len(performance.segments) + 1:
        raise TrajectoryError("ROUTE_PERFORMANCE_POINT_COUNT_MISMATCH")
    if start_time_s < 0:
        raise TrajectoryError("INVALID_START_TIME")
    if altitude_m < 0:
        raise TrajectoryError("INVALID_ALTITUDE")

    points: list[TrajectoryPoint] = [
        TrajectoryPoint(
            point_id=route_points[0].point_id,
            x=route_points[0].position.x,
            y=route_points[0].position.y,
            altitude_m=altitude_m,
            timestamp_s=start_time_s,
            ground_speed_mps=performance.segments[0].ground_speed_mps,
        )
    ]

    current_time = start_time_s
    for index, segment in enumerate(performance.segments):
        duration = segment.distance_m / segment.ground_speed_mps
        current_time += duration
        destination = route_points[index + 1]
        points.append(
            TrajectoryPoint(
                point_id=destination.point_id,
                x=destination.position.x,
                y=destination.position.y,
                altitude_m=altitude_m,
                timestamp_s=current_time,
                ground_speed_mps=segment.ground_speed_mps,
            )
        )

    trajectory = Trajectory4D(
        trajectory_id=trajectory_id,
        route_id=route_id,
        uav_id=uav_id,
        start_time_s=start_time_s,
        end_time_s=current_time,
        points=tuple(points),
        verified=False,
    )
    verify_trajectory_4d(trajectory)
    return Trajectory4D(
        trajectory_id=trajectory.trajectory_id,
        route_id=trajectory.route_id,
        uav_id=trajectory.uav_id,
        start_time_s=trajectory.start_time_s,
        end_time_s=trajectory.end_time_s,
        points=trajectory.points,
        verified=True,
    )


def verify_trajectory_4d(trajectory: Trajectory4D) -> None:
    if len(trajectory.points) < 2:
        raise TrajectoryError("TRAJECTORY_TOO_SHORT")
    if trajectory.end_time_s < trajectory.start_time_s:
        raise TrajectoryError("TRAJECTORY_TIME_ORDER_INVALID")

    previous = trajectory.points[0].timestamp_s
    for point in trajectory.points[1:]:
        if point.timestamp_s <= previous:
            raise TrajectoryError("TRAJECTORY_TIME_NOT_STRICTLY_INCREASING")
        if point.altitude_m < 0:
            raise TrajectoryError("INVALID_ALTITUDE")
        if point.ground_speed_mps <= 0:
            raise TrajectoryError("NON_POSITIVE_GROUND_SPEED")
        previous = point.timestamp_s
