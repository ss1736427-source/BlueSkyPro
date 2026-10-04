"""Traceable adapter for 4D trajectories."""

from __future__ import annotations

from dataclasses import dataclass

from trajectory_4d import Trajectory4D


@dataclass(frozen=True)
class TrajectoryArtifact:
    schema_version: str
    artifact_id: str
    mission_id: str
    source_route_id: str
    source_performance_id: str
    trajectory_id: str
    status: str
    start_time_s: float
    end_time_s: float
    points: tuple[dict, ...]


def build_trajectory_artifact(
    *,
    mission_id: str,
    performance_artifact_id: str,
    trajectory: Trajectory4D,
) -> TrajectoryArtifact:
    return TrajectoryArtifact(
        schema_version="1.0",
        artifact_id=f"{mission_id}:trajectory:{trajectory.trajectory_id}",
        mission_id=mission_id,
        source_route_id=trajectory.route_id,
        source_performance_id=performance_artifact_id,
        trajectory_id=trajectory.trajectory_id,
        status="VERIFIED" if trajectory.verified else "BLOCKED",
        start_time_s=trajectory.start_time_s,
        end_time_s=trajectory.end_time_s,
        points=tuple(
            {
                "pointId": p.point_id,
                "x": p.x,
                "y": p.y,
                "altitudeM": p.altitude_m,
                "timestampS": p.timestamp_s,
                "groundSpeedMps": p.ground_speed_mps,
            }
            for p in trajectory.points
        ),
    )
