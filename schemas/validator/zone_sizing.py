"""Capability-weighted zone sizing policy.

Sizing remains a partition concern. UAV assignment is intentionally performed
later by the assignment engine.
"""
from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class CapabilityWeight:
    uav_id: str
    weight: float


class ZoneSizingError(ValueError):
    pass


def normalize_weights(capabilities: list[CapabilityWeight]) -> list[CapabilityWeight]:
    if not capabilities:
        raise ZoneSizingError("ZONE_SIZING_FAILED: empty capability set")
    if any(c.weight <= 0 for c in capabilities):
        raise ZoneSizingError("ZONE_SIZING_FAILED: weights must be positive")
    total = sum(c.weight for c in capabilities)
    return [CapabilityWeight(c.uav_id, c.weight / total) for c in capabilities]


def weighted_area_targets(total_area: float, capabilities: list[CapabilityWeight]) -> dict[str, float]:
    if total_area <= 0:
        raise ZoneSizingError("ZONE_SIZING_FAILED: area must be positive")
    normalized = normalize_weights(capabilities)
    return {c.uav_id: total_area * c.weight for c in normalized}
