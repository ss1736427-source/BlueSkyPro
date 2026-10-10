"""Deterministic wind/performance adjustment for verified routes.

This is a contract-level performance model, not a flight-controller model.
"""

from __future__ import annotations

from dataclasses import dataclass
from math import cos, radians, sin, sqrt
from typing import Sequence

from route_in_zone import Route


@dataclass(frozen=True)
class WindSample:
    wind_speed_mps: float
    wind_from_deg: float


@dataclass(frozen=True)
class PerformanceProfile:
    cruise_speed_mps: float
    energy_per_meter_wh: float
    reserve_wh: float
    max_wind_mps: float
    model_authority: str = "CONTRACT"


@dataclass(frozen=True)
class PerformanceSegment:
    segment_index: int
    distance_m: float
    ground_speed_mps: float
    energy_wh: float
    wind_margin_mps: float


@dataclass(frozen=True)
class PerformanceAdjustedRoute:
    route_id: str
    uav_id: str
    segments: tuple[PerformanceSegment, ...]
    total_energy_wh: float
    reserve_margin_wh: float
    verified: bool
    model_authority: str


class PerformanceError(ValueError):
    pass


def _wind_component_along(
    dx: float,
    dy: float,
    wind_speed: float,
    wind_from_deg: float,
) -> float:
    length = sqrt(dx * dx + dy * dy)
    if length <= 0:
        raise PerformanceError("ZERO_LENGTH_SEGMENT")

    # Wind direction is "from"; convert to direction of travel component.
    travel_x, travel_y = dx / length, dy / length
    wind_to = radians((wind_from_deg + 180.0) % 360.0)
    wx, wy = cos(wind_to) * wind_speed, sin(wind_to) * wind_speed
    return wx * travel_x + wy * travel_y


def adjust_route_for_wind(
    route: Route,
    wind: WindSample,
    performance: PerformanceProfile,
) -> PerformanceAdjustedRoute:
    if not route.verified:
        raise PerformanceError("ROUTE_NOT_VERIFIED")
    if performance.model_authority not in {"CONTRACT", "AUTHORITATIVE"}:
        raise PerformanceError("INVALID_MODEL_AUTHORITY")
    if performance.cruise_speed_mps <= 0:
        raise PerformanceError("INVALID_CRUISE_SPEED")
    if performance.energy_per_meter_wh <= 0:
        raise PerformanceError("INVALID_ENERGY_MODEL")
    if performance.reserve_wh < 0:
        raise PerformanceError("INVALID_RESERVE")
    if wind.wind_speed_mps < 0:
        raise PerformanceError("INVALID_WIND")
    if wind.wind_speed_mps > performance.max_wind_mps:
        raise PerformanceError("WIND_LIMIT_EXCEEDED")

    segments: list[PerformanceSegment] = []
    total_energy = 0.0

    for index, (left, right) in enumerate(zip(route.points, route.points[1:])):
        dx = right.position.x - left.position.x
        dy = right.position.y - left.position.y
        distance = sqrt(dx * dx + dy * dy)
        component = _wind_component_along(
            dx, dy, wind.wind_speed_mps, wind.wind_from_deg
        )

        ground_speed = performance.cruise_speed_mps + component
        if ground_speed <= 0:
            raise PerformanceError("GROUND_SPEED_NON_POSITIVE")

        # Baseline energy model is distance-based; wind increases/decreases
        # travel time and is retained as explicit evidence rather than hidden.
        energy = distance * performance.energy_per_meter_wh
        total_energy += energy

        segments.append(
            PerformanceSegment(
                segment_index=index,
                distance_m=distance,
                ground_speed_mps=ground_speed,
                energy_wh=energy,
                wind_margin_mps=performance.max_wind_mps - wind.wind_speed_mps,
            )
        )

    reserve_margin = performance.reserve_wh - total_energy
    return PerformanceAdjustedRoute(
        route_id=route.route_id,
        uav_id=route.uav_id,
        segments=tuple(segments),
        total_energy_wh=total_energy,
        reserve_margin_wh=reserve_margin,
        verified=reserve_margin >= 0,
        model_authority=performance.model_authority,
    )
