"""Traceable ZoneAssignmentSet contract built from assignment results."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence

from uav_assignment import AssignmentCandidate, AssignmentResult


@dataclass(frozen=True)
class AssignmentArtifact:
    schema_version: str
    artifact_id: str
    mission_id: str
    zone_set_id: str
    source_artifact_ids: tuple[str, ...]
    source_versions: tuple[str, ...]
    status: str
    assignments: tuple[dict, ...]
    candidate_matrix: tuple[dict, ...]


def build_assignment_artifact(
    *,
    mission_id: str,
    zone_set_id: str,
    zone_set_version: str,
    assignment_version: str,
    result: AssignmentResult,
) -> AssignmentArtifact:
    """Convert the domain result into a deterministic traceable artifact."""
    artifact_id = f"{mission_id}:zone-assignment:{assignment_version}"

    assignments = tuple(
        {
            "zoneId": item.zone_id,
            "uavId": item.uav_id,
            "score": item.score,
        }
        for item in result.assignments
    )

    candidates = tuple(
        {
            "zoneId": item.zone_id,
            "uavId": item.uav_id,
            "feasible": item.feasible,
            "failedConstraints": list(item.failed_constraints),
            "capabilityMargin": item.capability_margin,
            "reserveMarginSeconds": item.reserve_margin_s,
            "windMargin": item.wind_margin,
            "score": item.score if item.feasible else None,
        }
        for item in result.candidates
    )

    return AssignmentArtifact(
        schema_version="1.0",
        artifact_id=artifact_id,
        mission_id=mission_id,
        zone_set_id=zone_set_id,
        source_artifact_ids=(zone_set_id,),
        source_versions=(zone_set_version,),
        status=result.status,
        assignments=assignments,
        candidate_matrix=candidates,
    )
