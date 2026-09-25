#!/usr/bin/env python3
"""Executable tests for the Multi-UAV domain dependency layer."""

from __future__ import annotations

import sys

from multi_uav_domain import (
    ArtifactRef,
    ArtifactState,
    DependencyGraph,
    release_eligible,
    temporal_delay_allowed,
)


def check_route_invalidation() -> None:
    artifacts = [
        ArtifactState(ArtifactRef("ROUTE_SET", "ROUTE-001", 1)),
        ArtifactState(ArtifactRef("PERFORMANCE_ADJUSTED_ROUTE_SET", "PERF-001", 1)),
        ArtifactState(ArtifactRef("TRAJECTORY_SET", "TRAJ-001", 1)),
        ArtifactState(ArtifactRef("CONFLICT_REPORT", "CONFLICT-001", 1)),
        ArtifactState(ArtifactRef("FINAL_CHECK_RESULT", "FINAL-001", 1)),
    ]
    graph = DependencyGraph(artifacts)
    affected = graph.invalidate_from(ArtifactRef("ROUTE_SET", "ROUTE-001", 2))

    assert [x.kind for x in affected] == [
        "PERFORMANCE_ADJUSTED_ROUTE_SET",
        "TRAJECTORY_SET",
        "CONFLICT_REPORT",
        "FINAL_CHECK_RESULT",
    ]
    assert all(not graph.active(k) for k in (
        "PERFORMANCE_ADJUSTED_ROUTE_SET",
        "TRAJECTORY_SET",
        "CONFLICT_REPORT",
        "FINAL_CHECK_RESULT",
    ))


def check_performance_invalidation() -> None:
    artifacts = [
        ArtifactState(ArtifactRef("PERFORMANCE_ADJUSTED_ROUTE_SET", "PERF-002", 1)),
        ArtifactState(ArtifactRef("TRAJECTORY_SET", "TRAJ-002", 1)),
        ArtifactState(ArtifactRef("CONFLICT_REPORT", "CONFLICT-002", 1)),
        ArtifactState(ArtifactRef("FINAL_CHECK_RESULT", "FINAL-002", 1)),
    ]
    graph = DependencyGraph(artifacts)
    affected = graph.invalidate_from(
        ArtifactRef("PERFORMANCE_ADJUSTED_ROUTE_SET", "PERF-002", 2)
    )
    assert [x.kind for x in affected] == [
        "TRAJECTORY_SET", "CONFLICT_REPORT", "FINAL_CHECK_RESULT"
    ]


def check_config_change_reaches_assignment() -> None:
    artifacts = [
        ArtifactState(ArtifactRef("ZONE_ASSIGNMENT_SET", "ASSIGN-003", 1)),
        ArtifactState(ArtifactRef("ROUTE_SET", "ROUTE-003", 1)),
        ArtifactState(ArtifactRef("PERFORMANCE_ADJUSTED_ROUTE_SET", "PERF-003", 1)),
        ArtifactState(ArtifactRef("TRAJECTORY_SET", "TRAJ-003", 1)),
        ArtifactState(ArtifactRef("CONFLICT_REPORT", "CONFLICT-003", 1)),
        ArtifactState(ArtifactRef("FINAL_CHECK_RESULT", "FINAL-003", 1)),
    ]
    graph = DependencyGraph(artifacts)
    affected = graph.invalidate_from(
        ArtifactRef("ZONE_ASSIGNMENT_SET", "ASSIGN-003", 2)
    )
    assert [x.kind for x in affected] == [
        "ROUTE_SET",
        "PERFORMANCE_ADJUSTED_ROUTE_SET",
        "TRAJECTORY_SET",
        "CONFLICT_REPORT",
        "FINAL_CHECK_RESULT",
    ]


def check_conflict_resolution_retrajectory() -> None:
    artifacts = [
        ArtifactState(ArtifactRef("CONFLICT_RESOLUTION", "RES-001", 1)),
        ArtifactState(ArtifactRef("TRAJECTORY_SET", "TRAJ-001", 1)),
        ArtifactState(ArtifactRef("CONFLICT_REPORT", "CONFLICT-001", 1)),
        ArtifactState(ArtifactRef("FINAL_CHECK_RESULT", "FINAL-001", 1)),
    ]
    graph = DependencyGraph(artifacts)
    affected = graph.invalidate_from(
        ArtifactRef("CONFLICT_RESOLUTION", "RES-001", 2)
    )
    assert [x.kind for x in affected] == [
        "TRAJECTORY_SET", "CONFLICT_REPORT", "FINAL_CHECK_RESULT"
    ]


def check_delay_boundary() -> None:
    assert temporal_delay_allowed(0.0)
    assert temporal_delay_allowed(5.0)
    assert not temporal_delay_allowed(-0.001)
    assert not temporal_delay_allowed(5.001)


def check_release_boundary() -> None:
    assert release_eligible(
        final_checks_pass=True,
        unresolved_conflicts=0,
        all_dependencies_active=True,
        authorization_status="NOT_EVALUATED",
    )
    assert not release_eligible(
        final_checks_pass=True,
        unresolved_conflicts=1,
        all_dependencies_active=True,
        authorization_status="NOT_EVALUATED",
    )
    assert not release_eligible(
        final_checks_pass=True,
        unresolved_conflicts=0,
        all_dependencies_active=True,
        authorization_status="AUTHORIZED",
    )


def main() -> int:
    check_route_invalidation()
    check_performance_invalidation()
    check_config_change_reaches_assignment()
    check_conflict_resolution_retrajectory()
    check_delay_boundary()
    check_release_boundary()
    print("MULTI-UAV DOMAIN TESTS: 6/6 PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
