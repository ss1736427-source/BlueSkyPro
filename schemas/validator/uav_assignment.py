"""Deterministic UAV ↔ Zone Assignment Engine."""

from __future__ import annotations
from dataclasses import dataclass
from typing import Sequence

from zone_partition import Zone


@dataclass(frozen=True)
class UAVCapability:
    uav_id: str
    ready: bool
    payload_compatible: bool
    endurance_s: float
    required_time_s: float
    reserve_s: float
    c2_available: bool
    authorized: bool
    capability_score: float = 0.0
    wind_margin: float = 0.0


@dataclass(frozen=True)
class AssignmentCandidate:
    uav_id: str
    zone_id: str
    feasible: bool
    failed_constraints: tuple[str, ...]
    capability_margin: float
    reserve_margin_s: float
    wind_margin: float
    score: float


@dataclass(frozen=True)
class ZoneAssignment:
    zone_id: str
    uav_id: str
    score: float


@dataclass(frozen=True)
class AssignmentResult:
    assignments: tuple[ZoneAssignment, ...]
    candidates: tuple[AssignmentCandidate, ...]
    status: str


class AssignmentError(ValueError):
    pass


def evaluate_candidate(uav: UAVCapability, zone: Zone) -> AssignmentCandidate:
    failures: list[str] = []
    reserve_margin = uav.endurance_s - uav.required_time_s - uav.reserve_s

    if not uav.ready:
        failures.append("NOT_READY")
    if not uav.payload_compatible:
        failures.append("PAYLOAD_INCOMPATIBLE")
    if reserve_margin < 0:
        failures.append("INSUFFICIENT_RESERVE")
    if not uav.c2_available:
        failures.append("C2_UNAVAILABLE")
    if not uav.authorized:
        failures.append("NOT_AUTHORIZED")

    feasible = not failures
    score = (
        uav.capability_score
        + max(0.0, reserve_margin)
        + max(0.0, uav.wind_margin)
    ) if feasible else float("-inf")

    return AssignmentCandidate(
        uav_id=uav.uav_id,
        zone_id=zone.zone_id,
        feasible=feasible,
        failed_constraints=tuple(failures),
        capability_margin=uav.capability_score,
        reserve_margin_s=reserve_margin,
        wind_margin=uav.wind_margin,
        score=score,
    )


def assign_zones(
    zones: Sequence[Zone],
    fleet: Sequence[UAVCapability],
) -> AssignmentResult:
    if not zones:
        raise AssignmentError("ASSIGNMENT_FAILED: no zones")
    if not fleet:
        raise AssignmentError("ASSIGNMENT_FAILED: empty fleet")

    candidates = tuple(
        evaluate_candidate(uav, zone)
        for zone in zones
        for uav in fleet
    )

    selected: list[ZoneAssignment] = []
    used_uavs: set[str] = set()

    for zone in zones:
        feasible = [
            c for c in candidates
            if c.zone_id == zone.zone_id and c.feasible and c.uav_id not in used_uavs
        ]
        if not feasible:
            raise AssignmentError(
                f"ASSIGNMENT_FAILED: no feasible UAV for {zone.zone_id}"
            )

        winner = max(feasible, key=lambda c: (c.score, c.uav_id))
        selected.append(ZoneAssignment(zone.zone_id, winner.uav_id, winner.score))
        used_uavs.add(winner.uav_id)

    return AssignmentResult(tuple(selected), candidates, "VERIFIED")
