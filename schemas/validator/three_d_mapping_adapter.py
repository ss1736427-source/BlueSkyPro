"""3D Mapping presentation adapter for the existing planning pipeline.

This module does not create a second planner. It consumes verified route/performance
artifacts and exposes acquisition metrics required by the 3D Mapping mission profile.
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Sequence


@dataclass(frozen=True)
class ThreeDMappingPlanResult:
    line_spacing_m: float | None
    line_count: int | None
    expected_frames: int | None
    expected_coverage_percent: float | None
    route_length_m: float
    expected_duration_s: float
    expected_energy_wh: float
    required_reserve_wh: float | None
    data_volume_mb: float | None
    release_status: str
    verified: bool


class ThreeDMappingAdapterError(ValueError):
    pass


def build_three_d_mapping_result(
    *,
    routes: Sequence[object],
    performance: Sequence[object],
    trajectories: Sequence[object],
    release_status: str,
    required_reserve_wh: float | None = None,
    line_spacing_m: float | None = None,
    line_count: int | None = None,
    expected_frames: int | None = None,
    expected_coverage_percent: float | None = None,
    data_volume_mb: float | None = None,
) -> ThreeDMappingPlanResult:
    if not routes:
        raise ThreeDMappingAdapterError("3D_MAPPING_NO_ROUTES")
    if len(routes) != len(performance):
        raise ThreeDMappingAdapterError("3D_MAPPING_ROUTE_PERFORMANCE_MISMATCH")
    if len(performance) != len(trajectories):
        raise ThreeDMappingAdapterError("3D_MAPPING_PERFORMANCE_TRAJECTORY_MISMATCH")
    if any(not getattr(item, "verified", False) for item in routes):
        raise ThreeDMappingAdapterError("3D_MAPPING_ROUTE_NOT_VERIFIED")
    if any(not getattr(item, "verified", False) for item in performance):
        raise ThreeDMappingAdapterError("3D_MAPPING_PERFORMANCE_NOT_VERIFIED")
    if any(not getattr(item, "verified", False) for item in trajectories):
        raise ThreeDMappingAdapterError("3D_MAPPING_TRAJECTORY_NOT_VERIFIED")

    route_length = sum(float(getattr(route, "length", 0.0)) for route in routes)
    energy = sum(float(getattr(item, "total_energy_wh", 0.0)) for item in performance)
    start = min(float(getattr(item, "start_time_s")) for item in trajectories)
    end = max(float(getattr(item, "end_time_s")) for item in trajectories)

    if route_length <= 0:
        raise ThreeDMappingAdapterError("3D_MAPPING_ZERO_ROUTE_LENGTH")
    if end < start:
        raise ThreeDMappingAdapterError("3D_MAPPING_INVALID_TIME_RANGE")

    return ThreeDMappingPlanResult(
        line_spacing_m=line_spacing_m,
        line_count=line_count,
        expected_frames=expected_frames,
        expected_coverage_percent=expected_coverage_percent,
        route_length_m=route_length,
        expected_duration_s=end - start,
        expected_energy_wh=energy,
        required_reserve_wh=required_reserve_wh,
        data_volume_mb=data_volume_mb,
        release_status=release_status,
        verified=release_status in {"FINAL_CHECK_PASS", "RELEASE_ELIGIBLE"},
    )
