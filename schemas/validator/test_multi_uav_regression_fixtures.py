#!/usr/bin/env python3
"""Deterministic checks for the BlueSky PRO Multi-UAV regression fixtures."""

from __future__ import annotations

import json
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
FIXTURES = ROOT / "schemas" / "examples" / "multi-uav-regression-fixtures.json"

ALLOWED_METHODS = {"SPATIAL_REGENERATION", "TEMPORAL_DELAY", "VERTICAL_CORRECTION"}


def fail(message: str) -> None:
    raise AssertionError(message)


def load() -> list[dict]:
    with FIXTURES.open("r", encoding="utf-8") as handle:
        return json.load(handle)["fixtures"]


def by_id(fixtures: list[dict], test_id: str) -> dict:
    return next(f for f in fixtures if f["testId"] == test_id)


def check_conflict_cases(fixtures: list[dict]) -> None:
    for test_id, classification in (("E2E-014", "CONFLICT"), ("E2E-015", "UNRESOLVED")):
        f = by_id(fixtures, test_id)
        if f["expectedConflictClassification"] != classification:
            fail(f"{test_id}: wrong conflict classification")
        if f["expectedReleaseStatus"] != "BLOCKED":
            fail(f"{test_id}: blocking conflict must block release")


def check_temporal_correction(fixtures: list[dict]) -> None:
    f = by_id(fixtures, "E2E-016")
    candidates = f["candidateCorrections"]
    delays = [c["delayS"] for c in candidates]

    if any(c["method"] != "TEMPORAL_DELAY" for c in candidates):
        fail("E2E-016: invalid correction method")
    if any(delay < 0 or delay > 5 for delay in delays):
        fail("E2E-016: temporal correction outside 0-5 seconds")
    if f["selectedCorrection"]["delayS"] != min(delays):
        fail("E2E-016: selected correction is not deterministic minimum")
    if f["expectedReleaseStatus"] != "RELEASE_ELIGIBLE":
        fail("E2E-016: expected resolved release state missing")


def check_rejected_correction(fixtures: list[dict]) -> None:
    f = by_id(fixtures, "E2E-018")
    c = f["candidateCorrection"]
    if c["method"] not in ALLOWED_METHODS:
        fail("E2E-018: unsupported correction method")
    if not f["secondaryConflictExpected"]:
        fail("E2E-018: secondary conflict must be represented")
    if f["expectedResolutionState"] != "UNRESOLVED":
        fail("E2E-018: correction must end unresolved")
    if f["expectedReleaseStatus"] != "BLOCKED":
        fail("E2E-018: rejected correction must block release")


def check_no_correction(fixtures: list[dict]) -> None:
    f = by_id(fixtures, "E2E-019")
    if f["candidateCorrections"]:
        fail("E2E-019: no-permitted-correction fixture must have no candidates")
    if f["expectedResolutionState"] != "UNRESOLVED":
        fail("E2E-019: expected unresolved state missing")
    if f["expectedReleaseStatus"] != "BLOCKED":
        fail("E2E-019: expected blocked state missing")


def check_invalidation(fixtures: list[dict]) -> None:
    expected = {
        "E2E-020": {"PERF-001:v1", "TRAJ-001:v1", "CONFLICT-001:v1", "FINAL-001:v1"},
        "E2E-021": {"TRAJ-002:v1", "CONFLICT-002:v1", "FINAL-002:v1"},
        "E2E-022": {"ASSIGN-003:v1", "PERF-003:v1", "TRAJ-003:v1", "CONFLICT-003:v1", "FINAL-003:v1"},
    }
    for test_id, downstream in expected.items():
        f = by_id(fixtures, test_id)
        if f["requiredState"] != "RECALCULATION_REQUIRED":
            fail(f"{test_id}: invalidation must require recalculation")
        if set(f["invalidatedArtifacts"]) != downstream:
            fail(f"{test_id}: invalidated dependency set mismatch")


def check_replay(fixtures: list[dict]) -> None:
    f = by_id(fixtures, "E2E-023")
    if f["expectedArtifacts"] != "IDENTICAL":
        fail("E2E-023: replay result must be identical")
    if not f["replayKey"]:
        fail("E2E-023: replay key missing")


def check_version_mismatch(fixtures: list[dict]) -> None:
    f = by_id(fixtures, "E2E-026")
    mismatch = f["mismatch"]
    if mismatch["routeSet"] == mismatch["performanceAdjustedRouteSet"]:
        fail("E2E-026: mismatch fixture is not actually mismatched")
    if f["expectedReleaseStatus"] != "FINAL_CHECK_FAIL":
        fail("E2E-026: version mismatch must fail final gate")


def check_authorization_boundary(fixtures: list[dict]) -> None:
    f = by_id(fixtures, "E2E-027")
    if f["technicalState"] != "RELEASE_ELIGIBLE":
        fail("E2E-027: technical state mismatch")
    if f["authorizationState"] != "NOT_EVALUATED":
        fail("E2E-027: authorization must remain separate")
    if f["expectedAuthorizationBypass"] is not False:
        fail("E2E-027: authorization bypass must remain forbidden")


def main() -> int:
    fixtures = load()
    expected_ids = {
        "E2E-014", "E2E-015", "E2E-016", "E2E-018", "E2E-019",
        "E2E-020", "E2E-021", "E2E-022", "E2E-023", "E2E-026", "E2E-027",
    }
    actual_ids = {f["testId"] for f in fixtures}
    if actual_ids != expected_ids:
        fail(f"fixture set mismatch: {sorted(actual_ids)}")

    check_conflict_cases(fixtures)
    check_temporal_correction(fixtures)
    check_rejected_correction(fixtures)
    check_no_correction(fixtures)
    check_invalidation(fixtures)
    check_replay(fixtures)
    check_version_mismatch(fixtures)
    check_authorization_boundary(fixtures)

    print("BLUE SKY PRO MULTI-UAV REGRESSION FIXTURES")
    for test_id in sorted(expected_ids):
        print(f"{test_id}: PASS")
    print("RESULT: 11/11 CONTRACT FIXTURES PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
