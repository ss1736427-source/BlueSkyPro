"""Traceable contract adapter for the complete Multi-UAV pipeline."""

from __future__ import annotations

from dataclasses import dataclass

from multi_uav_orchestrator import PipelineResult


@dataclass(frozen=True)
class PipelineArtifact:
    schema_version: str
    artifact_id: str
    mission_id: str
    status: str
    conflict_status: str
    resolution_status: str
    resolution_method: str | None
    delay_s: float
    unresolved_conflicts: int
    release_status: str
    authorization_status: str


def build_pipeline_artifact(
    *,
    mission_id: str,
    result: PipelineResult,
) -> PipelineArtifact:
    return PipelineArtifact(
        schema_version="1.0",
        artifact_id=f"{mission_id}:multi-uav-pipeline",
        mission_id=mission_id,
        status=result.final_gate.status,
        conflict_status=result.conflict.status,
        resolution_status=result.resolution.status,
        resolution_method=result.resolution.method,
        delay_s=result.resolution.delay_s,
        unresolved_conflicts=len(result.final_gate.failed_checks),
        release_status=result.final_gate.release_status,
        authorization_status=result.final_gate.authorization_status,
    )
