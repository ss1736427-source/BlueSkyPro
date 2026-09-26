"""Traceable contract for ground conflict resolution."""

from __future__ import annotations

from dataclasses import dataclass

from conflict_resolution import ResolutionResult


@dataclass(frozen=True)
class ResolutionArtifact:
    schema_version: str
    artifact_id: str
    mission_id: str
    source_conflict_artifact_id: str
    status: str
    method: str | None
    affected_uav_id: str | None
    delay_s: float
    vertical_correction_m: float
    post_verification_status: str
    rejected_candidates: tuple[str, ...]


def build_resolution_artifact(
    *,
    mission_id: str,
    conflict_artifact_id: str,
    result: ResolutionResult,
) -> ResolutionArtifact:
    if result.status == "RESOLVED" and result.conflict_report.status != "NO_CONFLICT":
        raise ValueError("RESOLUTION_CONTRACT_INVALID: resolved result is not conflict-free")

    return ResolutionArtifact(
        schema_version="1.0",
        artifact_id=f"{mission_id}:conflict-resolution",
        mission_id=mission_id,
        source_conflict_artifact_id=conflict_artifact_id,
        status=result.status,
        method=result.method,
        affected_uav_id=result.affected_uav_id,
        delay_s=result.delay_s,
        vertical_correction_m=result.vertical_correction_m,
        post_verification_status=result.conflict_report.status,
        rejected_candidates=result.rejected_candidates,
    )
