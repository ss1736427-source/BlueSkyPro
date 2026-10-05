#!/usr/bin/env python3

from multi_uav_domain import ArtifactRef, ArtifactState, DependencyGraph


def state(kind: str, version: int = 1) -> ArtifactState:
    return ArtifactState(ArtifactRef(kind, kind.lower(), version))


def main() -> int:
    states = [
        state("MISSION"),
        state("CONSTRAINED_OPEN_SPACE"),
        state("ZONE_SET"),
        state("ZONE_ASSIGNMENT_SET"),
        state("ROUTE_SET"),
        state("PERFORMANCE_ADJUSTED_ROUTE_SET"),
        state("TRAJECTORY_SET"),
        state("CONFLICT_REPORT"),
        state("CONFLICT_RESOLUTION"),
        state("FINAL_CHECK_RESULT"),
    ]
    graph = DependencyGraph(states)

    invalidated = graph.invalidate_from(ArtifactRef("ZONE_ASSIGNMENT_SET", "zone_assignment_set", 1))
    invalidated_kinds = {item.kind for item in invalidated}

    assert invalidated_kinds == {
        "ROUTE_SET",
        "PERFORMANCE_ADJUSTED_ROUTE_SET",
        "TRAJECTORY_SET",
        "CONFLICT_REPORT",
        "CONFLICT_RESOLUTION",
        "FINAL_CHECK_RESULT",
    }
    assert graph.active("ROUTE_SET") is False
    assert graph.active("FINAL_CHECK_RESULT") is False
    assert graph.active("ZONE_SET") is True

    print("ASSIGNMENT INVALIDATION TEST: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
