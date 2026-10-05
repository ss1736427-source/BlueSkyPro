"""Traceable contract adapter for performance-adjusted routes."""

from __future__ import annotations

from dataclasses import dataclass

from wind_performance import PerformanceAdjustedRoute


@dataclass(frozen=True)
class PerformanceArtifact:
    schema_version: str
    artifact_id: str
    mission_id: str
    source_route_id: str
    source_route_version: str
    wind_source: str
    performance_profile_version: str
    status: str
    total_energy_wh: float
    reserve_margin_wh: float
    segments: tuple[dict, ...]


def build_performance_artifact(
    *,
    mission_id: str,
    route_version: str,
    wind_source: str,
    performance_profile_version: str,
    result: PerformanceAdjustedRoute,
) -> PerformanceArtifact:
    return PerformanceArtifact(
        schema_version="1.0",
        artifact_id=f"{mission_id}:performance:{result.route_id}:{performance_profile_version}",
        mission_id=mission_id,
        source_route_id=result.route_id,
        source_route_version=route_version,
        wind_source=wind_source,
        performance_profile_version=performance_profile_version,
        status="VERIFIED" if result.verified else "BLOCKED_RESERVE",
        total_energy_wh=result.total_energy_wh,
        reserve_margin_wh=result.reserve_margin_wh,
        segments=tuple(
            {
                "segmentIndex": s.segment_index,
                "distanceM": s.distance_m,
                "groundSpeedMps": s.ground_speed_mps,
                "energyWh": s.energy_wh,
                "windMarginMps": s.wind_margin_mps,
            }
            for s in result.segments
        ),
    )
