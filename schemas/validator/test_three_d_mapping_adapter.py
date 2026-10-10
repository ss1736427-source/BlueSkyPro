from dataclasses import dataclass
from three_d_mapping_adapter import build_three_d_mapping_result


@dataclass(frozen=True)
class Route:
    length: float
    verified: bool = True


@dataclass(frozen=True)
class Performance:
    total_energy_wh: float
    verified: bool = True


@dataclass(frozen=True)
class Trajectory:
    start_time_s: float
    end_time_s: float
    verified: bool = True


def test_maps_verified_pipeline_outputs():
    result = build_three_d_mapping_result(
        routes=[Route(1200.0), Route(800.0)],
        performance=[Performance(12.5), Performance(9.5)],
        trajectories=[Trajectory(0.0, 60.0), Trajectory(5.0, 55.0)],
        release_status="RELEASE_ELIGIBLE",
        required_reserve_wh=20.0,
        expected_coverage_percent=97.5,
    )
    assert result.route_length_m == 2000.0
    assert result.expected_duration_s == 60.0
    assert result.expected_energy_wh == 22.0
    assert result.expected_coverage_percent == 97.5
    assert result.verified is True


def test_rejects_unverified_route():
    try:
        build_three_d_mapping_result(
            routes=[Route(100.0, False)],
            performance=[Performance(1.0)],
            trajectories=[Trajectory(0.0, 10.0)],
            release_status="RELEASE_ELIGIBLE",
        )
    except ValueError as exc:
        assert str(exc) == "3D_MAPPING_ROUTE_NOT_VERIFIED"
    else:
        raise AssertionError("unverified route must be rejected")
