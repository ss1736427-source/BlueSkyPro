#!/usr/bin/env python3

from final_gate import GateInput, evaluate_final_gate


def complete_checks():
    return (
        GateInput("ZONE_SET", "VERIFIED"),
        GateInput("ASSIGNMENT", "VERIFIED"),
        GateInput("ROUTE", "VERIFIED"),
        GateInput("PERFORMANCE", "VERIFIED"),
        GateInput("TRAJECTORY", "VERIFIED"),
        GateInput("CONFLICT", "NO_CONFLICT"),
        GateInput("RESOLUTION", "NO_ACTION"),
    )


def main() -> int:
    passed = evaluate_final_gate(
        complete_checks(),
        unresolved_conflicts=0,
    )
    assert passed.status == "PASS"
    assert passed.release_status == "RELEASE_ELIGIBLE"
    assert passed.authorization_status == "NOT_EVALUATED"

    blocked_conflict = evaluate_final_gate(
        (
            *complete_checks()[:-2],
            GateInput("CONFLICT", "CONFLICT"),
            GateInput("RESOLUTION", "UNRESOLVED"),
        ),
        unresolved_conflicts=1,
    )
    assert blocked_conflict.status == "FAIL"
    assert blocked_conflict.release_status == "BLOCKED"

    invalidated_route = evaluate_final_gate(
        (
            GateInput("ZONE_SET", "VERIFIED"),
            GateInput("ASSIGNMENT", "VERIFIED"),
            GateInput("ROUTE", "VERIFIED", active=False),
            GateInput("PERFORMANCE", "VERIFIED"),
            GateInput("TRAJECTORY", "VERIFIED"),
            GateInput("CONFLICT", "NO_CONFLICT"),
        ),
        unresolved_conflicts=0,
    )
    assert invalidated_route.release_status == "BLOCKED"
    assert "ROUTE:INVALIDATED" in invalidated_route.failed_checks


    missing_resolution = evaluate_final_gate(
        complete_checks()[:-1],
        unresolved_conflicts=0,
    )
    assert missing_resolution.release_status == "BLOCKED"
    assert "RESOLUTION:MISSING" in missing_resolution.failed_checks

    duplicate_check = evaluate_final_gate(
        (*complete_checks(), GateInput("ROUTE", "VERIFIED")),
        unresolved_conflicts=0,
    )
    assert duplicate_check.release_status == "BLOCKED"
    assert "ROUTE:DUPLICATE" in duplicate_check.failed_checks

    auth_not_granted = evaluate_final_gate(
        complete_checks(),
        unresolved_conflicts=0,
        authorization_status="PENDING",
    )
    assert auth_not_granted.release_status == "RELEASE_ELIGIBLE"
    assert auth_not_granted.authorization_status == "PENDING"

    print("FINAL GATE TESTS: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
