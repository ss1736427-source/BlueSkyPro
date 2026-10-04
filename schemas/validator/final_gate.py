"""Final technical verification gate for the multi-UAV planning pipeline."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence


@dataclass(frozen=True)
class GateInput:
    name: str
    status: str
    active: bool = True


@dataclass(frozen=True)
class FinalGateResult:
    status: str
    release_status: str
    failed_checks: tuple[str, ...]
    upstream_checks: tuple[str, ...]
    authorization_status: str


class FinalGateError(ValueError):
    pass


_REQUIRED_PASS = {
    "ZONE_SET": "VERIFIED",
    "ASSIGNMENT": "VERIFIED",
    "ROUTE": "VERIFIED",
    "PERFORMANCE": "VERIFIED",
    "TRAJECTORY": "VERIFIED",
    "CONFLICT": "NO_CONFLICT",
}


def evaluate_final_gate(
    checks: Sequence[GateInput],
    *,
    unresolved_conflicts: int,
    authorization_status: str = "NOT_EVALUATED",
) -> FinalGateResult:
    by_name: dict[str, GateInput] = {}
    failed: list[str] = []
    for check in checks:
        if check.name in by_name:
            failed.append(f"{check.name}:DUPLICATE")
        else:
            by_name[check.name] = check

    for name, required in _REQUIRED_PASS.items():
        check = by_name.get(name)
        if check is None:
            failed.append(f"{name}:MISSING")
            continue
        if not check.active:
            failed.append(f"{name}:INVALIDATED")
            continue
        if check.status != required:
            failed.append(f"{name}:{check.status}")

    resolution = by_name.get("RESOLUTION")
    if resolution is None:
        failed.append("RESOLUTION:MISSING")
    elif not resolution.active:
        failed.append("RESOLUTION:INVALIDATED")
    elif resolution.status not in {"NO_ACTION", "RESOLVED"}:
        failed.append(f"RESOLUTION:{resolution.status}")

    if unresolved_conflicts != 0:
        failed.append(f"UNRESOLVED_CONFLICTS:{unresolved_conflicts}")

    if authorization_status != "AUTHORIZED":
        failed.append(f"AUTHORIZATION:{authorization_status}")

    if failed:
        return FinalGateResult(
            status="FAIL",
            release_status="BLOCKED",
            failed_checks=tuple(failed),
            upstream_checks=tuple(
                f"{check.name}:{check.status}" for check in checks
            ),
            authorization_status=authorization_status,
        )

    return FinalGateResult(
        status="PASS",
        release_status="RELEASE_ELIGIBLE",
        failed_checks=(),
        upstream_checks=tuple(
            f"{check.name}:{check.status}" for check in checks
        ),
        authorization_status=authorization_status,
    )
