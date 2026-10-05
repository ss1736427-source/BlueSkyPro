"""Traceable adapter for 4D conflict reports."""

from __future__ import annotations

from dataclasses import dataclass

from conflict_4d import ConflictReport


@dataclass(frozen=True)
class ConflictArtifact:
    schema_version: str
    artifact_id: str
    mission_id: str
    trajectory_ids: tuple[str, ...]
    status: str
    checked_pairs: int
    conflicts: tuple[dict, ...]


def build_conflict_artifact(
    *,
    mission_id: str,
    trajectory_ids: tuple[str, ...],
    report: ConflictReport,
) -> ConflictArtifact:
    return ConflictArtifact(
        schema_version="1.0",
        artifact_id=f"{mission_id}:conflict-report",
        mission_id=mission_id,
        trajectory_ids=trajectory_ids,
        status=report.status,
        checked_pairs=report.checked_pairs,
        conflicts=tuple(
            {
                "conflictId": conflict.conflict_id,
                "uavA": conflict.uav_a,
                "uavB": conflict.uav_b,
                "timeStartS": conflict.time_start_s,
                "timeEndS": conflict.time_end_s,
                "minimumHorizontalM": conflict.minimum_horizontal_m,
                "minimumVerticalM": conflict.minimum_vertical_m,
                "classification": conflict.classification,
            }
            for conflict in report.conflicts
        ),
    )
