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

def test_rejects_empty_routes():
    try:
        build_three_d_mapping_result(
            routes=[],
            performance=[],
            trajectories=[],
            release_status="RELEASE_ELIGIBLE",
        )
    except ValueError as exc:
        assert str(exc) == "3D_MAPPING_NO_ROUTES"
    else:
        raise AssertionError("empty route set must be rejected")


def test_rejects_route_performance_mismatch():
    try:
        build_three_d_mapping_result(
            routes=[Route(100.0)],
            performance=[],
            trajectories=[Trajectory(0.0, 10.0)],
            release_status="RELEASE_ELIGIBLE",
        )
    except ValueError as exc:
        assert str(exc) == "3D_MAPPING_ROUTE_PERFORMANCE_MISMATCH"
    else:
        raise AssertionError("route/performance mismatch must be rejected")


def test_rejects_performance_trajectory_mismatch():
    try:
        build_three_d_mapping_result(
            routes=[Route(100.0)],
            performance=[Performance(1.0), Performance(2.0)],
            trajectories=[Trajectory(0.0, 10.0)],
            release_status="RELEASE_ELIGIBLE",
        )
    except ValueError as exc:
        assert str(exc) == "3D_MAPPING_PERFORMANCE_TRAJECTORY_MISMATCH"
    else:
        raise AssertionError("performance/trajectory mismatch must be rejected")


def test_rejects_unverified_performance():
    try:
        build_three_d_mapping_result(
            routes=[Route(100.0)],
            performance=[Performance(1.0, False)],
            trajectories=[Trajectory(0.0, 10.0)],
            release_status="RELEASE_ELIGIBLE",
        )
    except ValueError as exc:
        assert str(exc) == "3D_MAPPING_PERFORMANCE_NOT_VERIFIED"
    else:
        raise AssertionError("unverified performance must be rejected")


def test_rejects_unverified_trajectory():
    try:
        build_three_d_mapping_result(
            routes=[Route(100.0)],
            performance=[Performance(1.0)],
            trajectories=[Trajectory(0.0, 10.0, False)],
            release_status="RELEASE_ELIGIBLE",
        )
    except ValueError as exc:
        assert str(exc) == "3D_MAPPING_TRAJECTORY_NOT_VERIFIED"
    else:
        raise AssertionError("unverified trajectory must be rejected")


def test_rejects_zero_or_negative_aggregate_route_length():
    for length in (0.0, -1.0):
        try:
            build_three_d_mapping_result(
                routes=[Route(length)],
                performance=[Performance(1.0)],
                trajectories=[Trajectory(0.0, 10.0)],
                release_status="RELEASE_ELIGIBLE",
            )
        except ValueError as exc:
            assert str(exc) == "3D_MAPPING_ZERO_ROUTE_LENGTH"
        else:
            raise AssertionError("non-positive route length must be rejected")


def test_rejects_inverted_time_range():
    try:
        build_three_d_mapping_result(
            routes=[Route(100.0)],
            performance=[Performance(1.0)],
            trajectories=[Trajectory(20.0, 10.0)],
            release_status="RELEASE_ELIGIBLE",
        )
    except ValueError as exc:
        assert str(exc) == "3D_MAPPING_INVALID_TIME_RANGE"
    else:
        raise AssertionError("inverted time range must be rejected")


def test_unknown_release_status_does_not_mark_result_verified():
    result = build_three_d_mapping_result(
        routes=[Route(100.0)],
        performance=[Performance(1.0)],
        trajectories=[Trajectory(0.0, 10.0)],
        release_status="PENDING",
    )
    assert result.verified is False

def main():
    tests = (
        test_maps_verified_pipeline_outputs,
        test_rejects_unverified_route,
        test_rejects_empty_routes,
        test_rejects_route_performance_mismatch,
        test_rejects_performance_trajectory_mismatch,
        test_rejects_unverified_performance,
        test_rejects_unverified_trajectory,
        test_rejects_zero_or_negative_aggregate_route_length,
        test_rejects_inverted_time_range,
        test_unknown_release_status_does_not_mark_result_verified,
    )
    for test in tests:
        test()
    print(f"3D MAPPING ADAPTER TESTS: {len(tests)}/{len(tests)} PASS")


if __name__ == "__main__":
    main()

