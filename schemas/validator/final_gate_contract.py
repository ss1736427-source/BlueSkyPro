"""Traceable adapter for the final technical verification gate."""

from __future__ import annotations

from dataclasses import dataclass

from final_gate import FinalGateResult


@dataclass(frozen=True)
class FinalGateArtifact:
    schema_version: str
    artifact_id: str
    mission_id: str
    status: str
    release_status: str
    failed_checks: tuple[str, ...]
    upstream_checks: tuple[str, ...]
    authorization_status: str


def build_final_gate_artifact(
    *,
    mission_id: str,
    result: FinalGateResult,
) -> FinalGateArtifact:
    return FinalGateArtifact(
        schema_version="1.0",
        artifact_id=f"{mission_id}:final-gate",
        mission_id=mission_id,
        status=result.status,
        release_status=result.release_status,
        failed_checks=result.failed_checks,
        upstream_checks=result.upstream_checks,
        authorization_status=result.authorization_status,
    )
