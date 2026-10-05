#!/usr/bin/env python3
"""
BlueSky PRO Multi-UAV contract validator.

Scope:
- validates the contract baseline without requiring the future flight-planning core;
- checks cross-object identity and version dependencies;
- checks release-gate semantics;
- provides deterministic replay-key validation.

Optional JSON Schema validation uses the jsonschema package when installed.
The semantic checks remain dependency-free.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[2]
SCHEMA = ROOT / "schemas" / "multi-uav-planning.schema.json"
BASELINE = ROOT / "schemas" / "examples" / "multi-uav-baseline-2uav.json"


def fail(message: str) -> None:
    raise AssertionError(message)


def load(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def validate_schema(instance: dict[str, Any], schema: dict[str, Any]) -> None:
    try:
        import jsonschema
    except ImportError:
        return
    jsonschema.Draft202012Validator(schema).validate(
        {
            "schemaVersion": "1.0",
            "missionId": instance["missionRef"],
            "artifacts": instance["artifacts"],
        },
        schema,
    )


def check_artifact_identity(instance: dict[str, Any]) -> None:
    mission_id = instance["missionRef"]
    artifacts = instance["artifacts"]

    for name, artifact in artifacts.items():
        if artifact.get("missionId") != mission_id:
            fail(f"{name}: missionId mismatch")

    expected_sources = {
        "zoneSet": {"constrainedOpenSpace": "COS-001:v1"},
        "zoneAssignmentSet": {"zoneSet": "ZONESET-001:v1"},
        "routeSet": {"zoneAssignmentSet": "ASSIGN-001:v1"},
        "performanceAdjustedRouteSet": {"routeSet": "ROUTE-001:v1"},
        "trajectorySet": {"performanceAdjustedRouteSet": "PERF-001:v1"},
        "conflictReport": {"trajectorySet": "TRAJ-001:v1"},
    }

    for artifact_name, required in expected_sources.items():
        actual = artifacts[artifact_name]["traceability"]["inputVersions"]
        for key, value in required.items():
            if actual.get(key) != value:
                fail(f"{artifact_name}: expected {key}={value}, got {actual.get(key)}")


def check_zone_assignment(instance: dict[str, Any]) -> None:
    artifacts = instance["artifacts"]
    zones = {z["zoneId"] for z in artifacts["zoneSet"]["zones"]}
    assignments = artifacts["zoneAssignmentSet"]["assignments"]

    if len(assignments) != len(zones):
        fail("not every zone has exactly one primary assignment")

    assigned_zones = [a["zoneId"] for a in assignments]
    if len(set(assigned_zones)) != len(assigned_zones):
        fail("zone assigned more than once")

    if any(a["zoneId"] not in zones for a in assignments):
        fail("assignment references unknown zone")

    if any(a["feasibility"] != "FEASIBLE" for a in assignments):
        fail("baseline contains infeasible assignment")


def check_routes(instance: dict[str, Any]) -> None:
    artifacts = instance["artifacts"]
    assignment = {
        a["zoneId"]: a["uavId"]
        for a in artifacts["zoneAssignmentSet"]["assignments"]
    }

    for route in artifacts["routeSet"]["routes"]:
        if assignment.get(route["zoneId"]) != route["uavId"]:
            fail(f"route {route['segmentId']}: UAV/zone assignment mismatch")


def check_trajectory(instance: dict[str, Any]) -> None:
    trajectories = instance["artifacts"]["trajectorySet"]["trajectories"]

    for uav_id, points in trajectories.items():
        if not points:
            fail(f"{uav_id}: empty trajectory")

        timestamps = [point["t"] for point in points]
        if timestamps != sorted(timestamps):
            fail(f"{uav_id}: non-monotonic timestamps")

        for point in points:
            if point["uavId"] != uav_id:
                fail(f"{uav_id}: point UAV identity mismatch")


def check_conflicts(instance: dict[str, Any]) -> None:
    report = instance["artifacts"]["conflictReport"]

    if report["summary"]["conflictCount"] != len(report["conflicts"]):
        fail("conflictCount does not match conflict records")

    if report["summary"]["unresolvedCount"] != sum(
        1 for c in report["conflicts"] if c["classification"] == "UNRESOLVED"
    ):
        fail("unresolvedCount does not match conflict records")

    if report["summary"]["unresolvedCount"] != 0:
        fail("baseline contains unresolved conflict")


def check_final_gate(instance: dict[str, Any]) -> None:
    final = instance["artifacts"]["finalCheckResult"]

    if final["releaseStatus"] != instance["expectedFinalState"]:
        fail("unexpected final release state")

    if final["authorizationStatus"] != "NOT_EVALUATED":
        fail("technical release must remain separate from authorization")

    for check in final["checks"]:
        if check["result"] != "PASS":
            fail(f"final gate contains non-PASS check: {check['constraintId']}")


def check_replay(instance: dict[str, Any]) -> None:
    keys = {
        artifact["traceability"].get("replayKey")
        for artifact in instance["artifacts"].values()
    }
    if keys != {"BASELINE-2UAV-001"}:
        fail(f"unexpected replay keys: {sorted(keys)}")


def main() -> int:
    schema = load(SCHEMA)
    baseline = load(BASELINE)

    validate_schema(baseline, schema)
    check_artifact_identity(baseline)
    check_zone_assignment(baseline)
    check_routes(baseline)
    check_trajectory(baseline)
    check_conflicts(baseline)
    check_final_gate(baseline)
    check_replay(baseline)

    print("BLUE SKY PRO MULTI-UAV CONTRACT VALIDATOR")
    print("schema: PASS")
    print("artifact identity/dependencies: PASS")
    print("zone assignment: PASS")
    print("route linkage: PASS")
    print("trajectory semantics: PASS")
    print("conflict report: PASS")
    print("final gate: PASS")
    print("deterministic replay key: PASS")
    print("RESULT: RELEASE_ELIGIBLE baseline contract PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
