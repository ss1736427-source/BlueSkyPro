#!/usr/bin/env python3

from multi_uav_domain import ArtifactState, DependencyGraph


def main() -> int:
    graph = DependencyGraph()
    states = {
        "assignment": ArtifactState("assignment", True),
        "route": ArtifactState("route", True),
        "performance": ArtifactState("performance", True),
        "trajectory": ArtifactState("trajectory", True),
        "conflict": ArtifactState("conflict", True),
        "resolution": ArtifactState("resolution", True),
        "final": ArtifactState("final", True),
    }

    for state in states.values():
        graph.add(state)

    invalidated = graph.invalidate_from("assignment")

    assert invalidated == {
        "assignment", "route", "performance", "trajectory",
        "conflict", "resolution", "final",
    }
    assert graph.get("route").active is False
    assert graph.get("final").active is False

    print("ASSIGNMENT INVALIDATION TEST: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
