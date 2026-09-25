"""Minimal executable domain contracts for BlueSky PRO Multi-UAV planning.

This module implements versioned artifact identity and deterministic downstream
invalidation. It intentionally does not implement route generation, geometry
optimization, trajectory solving, or flight-control logic.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Iterable


ARTIFACT_ORDER = (
    "MISSION",
    "CONSTRAINED_OPEN_SPACE",
    "ZONE_SET",
    "ZONE_ASSIGNMENT_SET",
    "ROUTE_SET",
    "PERFORMANCE_ADJUSTED_ROUTE_SET",
    "TRAJECTORY_SET",
    "CONFLICT_REPORT",
    "CONFLICT_RESOLUTION",
    "FINAL_CHECK_RESULT",
)

DOWNSTREAM = {
    "MISSION": {
        "CONSTRAINED_OPEN_SPACE", "ZONE_SET", "ZONE_ASSIGNMENT_SET",
        "ROUTE_SET", "PERFORMANCE_ADJUSTED_ROUTE_SET", "TRAJECTORY_SET",
        "CONFLICT_REPORT", "CONFLICT_RESOLUTION", "FINAL_CHECK_RESULT",
    },
    "CONSTRAINED_OPEN_SPACE": {
        "ZONE_SET", "ZONE_ASSIGNMENT_SET", "ROUTE_SET",
        "PERFORMANCE_ADJUSTED_ROUTE_SET", "TRAJECTORY_SET",
        "CONFLICT_REPORT", "CONFLICT_RESOLUTION", "FINAL_CHECK_RESULT",
    },
    "ZONE_SET": {
        "ZONE_ASSIGNMENT_SET", "ROUTE_SET",
        "PERFORMANCE_ADJUSTED_ROUTE_SET", "TRAJECTORY_SET",
        "CONFLICT_REPORT", "CONFLICT_RESOLUTION", "FINAL_CHECK_RESULT",
    },
    "ZONE_ASSIGNMENT_SET": {
        "ROUTE_SET", "PERFORMANCE_ADJUSTED_ROUTE_SET", "TRAJECTORY_SET",
        "CONFLICT_REPORT", "CONFLICT_RESOLUTION", "FINAL_CHECK_RESULT",
    },
    "ROUTE_SET": {
        "PERFORMANCE_ADJUSTED_ROUTE_SET", "TRAJECTORY_SET",
        "CONFLICT_REPORT", "CONFLICT_RESOLUTION", "FINAL_CHECK_RESULT",
    },
    "PERFORMANCE_ADJUSTED_ROUTE_SET": {
        "TRAJECTORY_SET", "CONFLICT_REPORT",
        "CONFLICT_RESOLUTION", "FINAL_CHECK_RESULT",
    },
    "TRAJECTORY_SET": {
        "CONFLICT_REPORT", "CONFLICT_RESOLUTION", "FINAL_CHECK_RESULT",
    },
    "CONFLICT_REPORT": {
        "CONFLICT_RESOLUTION", "FINAL_CHECK_RESULT",
    },
    "CONFLICT_RESOLUTION": {
        "TRAJECTORY_SET", "CONFLICT_REPORT", "FINAL_CHECK_RESULT",
    },
    "FINAL_CHECK_RESULT": set(),
}


@dataclass(frozen=True)
class ArtifactRef:
    kind: str
    artifact_id: str
    version: int

    @property
    def key(self) -> str:
        return f"{self.artifact_id}:v{self.version}"


@dataclass
class ArtifactState:
    ref: ArtifactRef
    input_versions: dict[str, str] = field(default_factory=dict)
    status: str = "VERIFIED"
    invalidated_by: str | None = None

    def invalidate(self, source: ArtifactRef) -> None:
        self.status = "INVALIDATED"
        self.invalidated_by = source.key


class DependencyGraph:
    """Deterministic dependency/invalidation graph."""

    def __init__(self, artifacts: Iterable[ArtifactState]) -> None:
        self.artifacts = {a.ref.kind: a for a in artifacts}

    def invalidate_from(self, source: ArtifactRef) -> list[ArtifactRef]:
        affected_kinds = DOWNSTREAM.get(source.kind, set())
        affected: list[ArtifactRef] = []

        for kind in ARTIFACT_ORDER:
            if kind in affected_kinds and kind in self.artifacts:
                artifact = self.artifacts[kind]
                artifact.invalidate(source)
                affected.append(artifact.ref)

        return affected

    def active(self, kind: str) -> bool:
        artifact = self.artifacts[kind]
        return artifact.status != "INVALIDATED"


def temporal_delay_allowed(delay_s: float) -> bool:
    return 0.0 <= delay_s <= 5.0


def release_eligible(
    *,
    final_checks_pass: bool,
    unresolved_conflicts: int,
    all_dependencies_active: bool,
    authorization_status: str,
) -> bool:
    """Technical release only; authorization is deliberately independent."""
    return (
        final_checks_pass
        and unresolved_conflicts == 0
        and all_dependencies_active
        and authorization_status == "NOT_EVALUATED"
    )
