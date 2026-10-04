"""Deterministic global UAV ↔ Zone Assignment Engine."""

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
    compatible_zone_ids: tuple[str, ...] = ()


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
    if uav.compatible_zone_ids and zone.zone_id not in uav.compatible_zone_ids:
        failures.append("ZONE_INCOMPATIBLE")

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
    if len(zones) > len(fleet):
        raise AssignmentError("ASSIGNMENT_FAILED: insufficient fleet size")
    zone_ids = [zone.zone_id for zone in zones]
    if len(zone_ids) != len(set(zone_ids)):
        raise AssignmentError("ASSIGNMENT_FAILED: duplicate zone id")

    candidates = tuple(
        evaluate_candidate(uav, zone)
        for zone in zones
        for uav in fleet
    )
    by_zone = {
        zone.zone_id: tuple(
            c for c in candidates if c.zone_id == zone.zone_id and c.feasible
        )
        for zone in zones
    }

    # A complete deterministic search is used instead of greedy zone ordering.
    # Zones with fewer feasible UAVs are branched first; ties remain stable by
    # original zone order. The objective is maximum total assignment score,
    # with lexical tie-breaking for deterministic results.
    ordered_zones = tuple(
        sorted(
            zones,
            key=lambda zone: (
                len(by_zone[zone.zone_id]),
                next(i for i, item in enumerate(zones) if item.zone_id == zone.zone_id),
                zone.zone_id,
            ),
        )
    )

    if any(not by_zone[zone.zone_id] for zone in ordered_zones):
        failed_zone = next(
            zone for zone in ordered_zones if not by_zone[zone.zone_id]
        )
        raise AssignmentError(
            f"ASSIGNMENT_FAILED: no feasible UAV for {failed_zone.zone_id}"
        )

    best_score = float("-inf")
    best_assignment: tuple[ZoneAssignment, ...] | None = None

    def search(
        index: int,
        used: set[str],
        selected: list[ZoneAssignment],
        score: float,
    ) -> None:
        nonlocal best_score, best_assignment
        if index == len(ordered_zones):
            candidate = tuple(selected)
            signature = tuple(
                (item.zone_id, item.uav_id)
                for item in sorted(candidate, key=lambda item: item.zone_id)
            )
            best_signature = (
                tuple((item.zone_id, item.uav_id)
                      for item in sorted(best_assignment, key=lambda item: item.zone_id))
                if best_assignment is not None else None
            )
            if score > best_score or (
                score == best_score and (best_signature is None or signature < best_signature)
            ):
                best_score = score
                best_assignment = candidate
            return

        remaining = ordered_zones[index:]
        upper_bound = score + sum(
            max(
                (
                    candidate.score
                    for candidate in by_zone[item.zone_id]
                    if candidate.uav_id not in used
                ),
                default=float("-inf"),
            )
            for item in remaining
        )
        if upper_bound <= best_score:
            return

        zone = ordered_zones[index]
        options = sorted(
            (candidate for candidate in by_zone[zone.zone_id] if candidate.uav_id not in used),
            key=lambda candidate: (-candidate.score, candidate.uav_id),
        )
        for candidate in options:
            selected.append(
                ZoneAssignment(
                    zone_id=zone.zone_id,
                    uav_id=candidate.uav_id,
                    score=candidate.score,
                )
            )
            used.add(candidate.uav_id)
            search(index + 1, used, selected, score + candidate.score)
            used.remove(candidate.uav_id)
            selected.pop()

    search(0, set(), [], 0.0)

    if best_assignment is None:
        raise AssignmentError("ASSIGNMENT_FAILED: no globally feasible assignment")

    assignments = tuple(
        sorted(best_assignment, key=lambda item: item.zone_id)
    )
    return AssignmentResult(assignments, candidates, "VERIFIED")
