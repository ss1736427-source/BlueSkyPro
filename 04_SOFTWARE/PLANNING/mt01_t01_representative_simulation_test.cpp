#include "acquisition_geometry.hpp"
#include "coverage_decomposition.hpp"
#include "coverage_orientation.hpp"
#include "coverage_transition_graph.hpp"

#include <algorithm>

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>

using namespace bluesky::planning;

namespace {
constexpr char kDatasetId[] = "MT01-T01-SIM-001";
constexpr char kUavId[] = "DJI-MAVIC-3E-REFERENCE-SIM";
constexpr char kAlgorithmVersion[] = "MT01-SIM-CHAIN-001";
constexpr double kAoiSouth = 59.0000;
constexpr double kAoiNorth = 59.0009;
constexpr double kAoiWest = 30.0000;
constexpr double kAoiEast = 30.0014;

// All AOI/environment/flight-performance values below are representative
// simulation assumptions, not operational or regulatory data.
std::vector<GeoPoint> representativeAoi() {
    return {
        {kAoiSouth, kAoiWest},
        {kAoiSouth, kAoiEast},
        {kAoiNorth, kAoiEast},
        {kAoiNorth, kAoiWest}
    };
}

bool near(double actual, double expected, double tolerance = 1e-9) {
    return std::abs(actual - expected) <= tolerance;
}
}

int main(int argc, char* argv[]) {
    const auto aoi = representativeAoi();

    // Published DJI Mavic 3E model-level camera dimensions. This is not an
    // aircraft-specific intrinsic calibration or distortion correction.
    AcquisitionGeometryInput camera;
    camera.equipment_id = kUavId;
    camera.equipment_version = "DJI-OFFICIAL-MODEL-SPECS-2026-10-09";
    camera.sensor_width_m = 0.0174;
    camera.sensor_height_m = 0.0130;
    camera.focal_length_m = 0.01229; // physical focal length, not 24 mm equivalent
    camera.image_width_px = 5280.0;
    camera.image_height_px = 3956.0;
    camera.camera_ground_distance_m = 100.0; // representative mean AGL
    camera.frontal_overlap_ratio = 0.75;     // project baseline assumption
    camera.side_overlap_ratio = 0.60;        // project baseline assumption
    camera.ground_speed_mps = 10.0;           // simulation assumption

    const auto geometry =
        AcquisitionGeometryEngine::calculate(camera, kAlgorithmVersion);
    assert(geometry.valid);
    assert(geometry.gsd_width_m_per_px > 0.0);
    assert(geometry.gsd_height_m_per_px > 0.0);
    assert(geometry.trigger_interval_s >= 0.7); // published timed-capture capability
    assert(geometry.track_spacing_m > 0.0);

    CoverageOrientationInput orientationInput;
    orientationInput.aoi = aoi;
    orientationInput.acquisition_geometry = geometry;
    orientationInput.start_angle_deg = 0.0;
    orientationInput.end_angle_deg = 0.0;
    orientationInput.angle_step_deg = 1.0;
    orientationInput.geometry_revision = "SYNTHETIC-AOI-v1";
    orientationInput.calculation_version = kAlgorithmVersion;
    const auto orientations =
        CoverageOrientationGenerator::generate(orientationInput);
    assert(orientations.valid);
    assert(!orientations.candidates.empty());

    ConstrainedEnvironmentSnapshot environment;
    environment.snapshot_id = "MT01-T01-SIM-ENV-001";
    environment.snapshot_version = "1";
    environment.calculation_input_version = "SIM-INPUTS-001";
    environment.complete = true;
    // Synthetic NOTAM-like polygon outside the AOI: included in the planning
    // snapshot, but not a statement about actual airspace or clearance.
    SpatialRestriction simulatedNotam;
    simulatedNotam.restriction_id = "SIM-NOTAM-001";
    simulatedNotam.source_id = "VIRTUAL-FLIGHT-SYNTHETIC";
    simulatedNotam.snapshot_version = "1";
    simulatedNotam.geometry_type = RestrictionGeometryType::Polygon;
    simulatedNotam.polygon = {
        {59.00005, 30.00148}, {59.00005, 30.00162},
        {59.00020, 30.00162}, {59.00020, 30.00148}
    };
    simulatedNotam.minimum_altitude_m = 0.0;
    simulatedNotam.maximum_altitude_m = 500.0;
    simulatedNotam.active = true;
    environment.restrictions.push_back(simulatedNotam);

    CoverageDecompositionInput decompositionInput;
    decompositionInput.aoi = aoi;
    decompositionInput.orientation = orientations.candidates.front();
    decompositionInput.track_spacing_m = geometry.track_spacing_m;
    decompositionInput.minimum_altitude_m = 125.0; // synthetic datum altitude
    decompositionInput.maximum_altitude_m = 125.0;
    decompositionInput.environment = environment;
    decompositionInput.geometry_revision = "SYNTHETIC-AOI-v1";
    decompositionInput.calculation_version = kAlgorithmVersion;
    const auto decomposition =
        CoverageDecompositionEngine::decompose(decompositionInput);
    assert(decomposition.valid);
    assert(!decomposition.cells.empty());

    CoverageTrackInput trackInput;
    trackInput.decomposition = decomposition;
    trackInput.altitude_m = 125.0;
    trackInput.footprint_width_m = geometry.footprint_width_m;
    trackInput.footprint_height_m = geometry.footprint_height_m;
    trackInput.calculation_version = kAlgorithmVersion;
    const auto tracks = CoverageTrackGenerator::generate(trackInput);
    assert(tracks.valid);
    assert(!tracks.tracks.empty());

    const auto edgeResult = CoverageEdgeEngine::evaluate(trackInput, tracks);
    assert(edgeResult.valid);

    AcquisitionEventInput eventInput;
    eventInput.tracks = tracks;
    eventInput.geometry = geometry;
    eventInput.calculation_version = kAlgorithmVersion;
    const auto events = AcquisitionEventValidator::generate(eventInput);
    assert(events.valid);
    assert(!events.events.empty());

    MappingQualityInput qualityInput;
    qualityInput.aoi = aoi;
    qualityInput.environment = environment;
    qualityInput.decomposition = decomposition;
    qualityInput.tracks = tracks;
    qualityInput.events = events;
    qualityInput.geometry = geometry;
    qualityInput.calculation_version = kAlgorithmVersion;
    const auto quality = MappingQualityEngine::evaluate(qualityInput);
    assert(quality.valid);
    assert(quality.gate_passed);
    assert(quality.coverage_ratio >= 0.95);
    assert(quality.max_gsd_m_per_px <= 0.03);
    assert(quality.invalid_event_count == 0);
    assert(quality.uncovered_area_m2 >= 0.0);

    CoverageTransitionGraphInput graphInput;
    graphInput.tracks = tracks;
    graphInput.environment = environment;
    graphInput.calculation_version = kAlgorithmVersion;
    const auto graph = CoverageTransitionGraphBuilder::build(graphInput);
    assert(graph.valid);
    assert(graph.track_ids.size() == tracks.tracks.size());

    CoverageRouteCandidateInput candidateInput;
    candidateInput.graph = graph;
    candidateInput.max_candidates = 8;
    candidateInput.calculation_version = kAlgorithmVersion;
    const auto candidates = CoverageRouteCandidateBuilder::generate(candidateInput);
    assert(candidates.valid);
    assert(!candidates.candidates.empty());

    // Deterministic, synthetic average wind snapshot. Samples are supplied for
    // every potential track/transition segment so each candidate can be tested.
    TrajectoryPerformanceInput performance;
    performance.uav_id = kUavId;
    performance.configuration_version = "SIM-UAV-CFG-001";
    performance.performance_version = "SIM-PERF-ASSUMPTION-001";
    performance.wind_snapshot_id = "SIM-WIND-AVERAGE-001";
    performance.wind_snapshot_version = "1";
    performance.cruise_airspeed_mps = 10.0;
    performance.climb_rate_mps = 2.0;
    performance.descent_rate_mps = 2.0;
    performance.wind_tolerance_mps = 8.0;
    performance.energy_per_horizontal_meter_wh = 0.005;
    performance.energy_per_climb_meter_wh = 0.05;
    performance.energy_per_descent_meter_wh = 0.02;
    performance.usable_energy_wh = 60.0;
    performance.reserve_requirement_wh = 15.0;

    for (std::size_t i = 0; i < tracks.tracks.size(); ++i) {
        performance.wind_samples.push_back({
            "MT01-TRACK-SEG-" + std::to_string(i), 0.0, 1.5, 0.0
        });
    }
    for (const auto& from : tracks.tracks) {
        for (const auto& to : tracks.tracks) {
            if (from.track_id == to.track_id) continue;
            performance.wind_samples.push_back({
                "MT01-TRANSITION-SEG-" + from.track_id + "-" + to.track_id,
                0.0, 1.5, 0.0
            });
        }
    }

    CoverageRoutePerformanceInput routePerformanceInput;
    routePerformanceInput.tracks = tracks;
    routePerformanceInput.candidates = candidates;
    routePerformanceInput.performance = performance;
    routePerformanceInput.calculation_version = kAlgorithmVersion;
    const auto routePerformance =
        CoverageRoutePerformanceEvaluator::evaluate(routePerformanceInput);
    assert(routePerformance.valid);
    assert(!routePerformance.evaluations.empty());

    const auto selection = CoverageRouteSelector::select(
        routePerformance, {"energy", "time"}, "SIM-INPUTS-001",
        kAlgorithmVersion);
    assert(selection.valid);
    assert(!selection.selected_route_id.empty());

    bool selectedTrajectoryFound = false;
    std::size_t selectedCandidateIndex = 0;
    TrajectoryResult selectedTrajectory;
    for (std::size_t i = 0; i < routePerformance.evaluations.size(); ++i) {
        const auto& evaluation = routePerformance.evaluations[i];
        if (evaluation.route_id == selection.selected_route_id &&
            evaluation.status == TrajectoryStatus::Feasible) {
            selectedTrajectory = evaluation;
            selectedCandidateIndex = i;
            selectedTrajectoryFound = true;
            break;
        }
    }
    assert(selectedTrajectoryFound);
    assert(selectedTrajectory.remaining_energy_wh >= performance.reserve_requirement_wh);

    std::cout << std::fixed << std::setprecision(4);
    const bool jsonMode = argc > 1 && std::strcmp(argv[1], "--json") == 0;
    if (jsonMode) {
        // Display polyline derived from the selected coverage candidate. The
        // straight connector between tracks is illustrative; no obstacle-aware
        // transit path is claimed by this representative simulation.
        std::vector<GeoPoint> routePoints;
        if (selectedCandidateIndex < candidates.candidates.size()) {
            const auto& selectedCandidate = candidates.candidates[selectedCandidateIndex];
            for (const auto& trackId : selectedCandidate.track_ids) {
                const auto found = std::find_if(tracks.tracks.begin(), tracks.tracks.end(),
                    [&](const CoverageTrack& track) { return track.track_id == trackId; });
                if (found == tracks.tracks.end()) continue;
                GeoPoint start = found->start;
                GeoPoint end = found->end;
                if (!routePoints.empty()) {
                    const auto& last = routePoints.back();
                    const double ds = std::pow(last.latitude_deg - start.latitude_deg, 2.0)
                                    + std::pow(last.longitude_deg - start.longitude_deg, 2.0);
                    const double de = std::pow(last.latitude_deg - end.latitude_deg, 2.0)
                                    + std::pow(last.longitude_deg - end.longitude_deg, 2.0);
                    if (de < ds) std::swap(start, end);
                }
                routePoints.push_back(start);
                routePoints.push_back(end);
            }
        }
        std::cout << std::fixed << std::setprecision(8);
        std::cout << "{\"schemaVersion\":\"1.0\","
                  << "\"messageType\":\"planning.result\","
                  << "\"missionId\":\"V-M01-01-SIM-001\","
                  << "\"resultId\":\"MT01-T01-SIM-001-RESULT-001\","
                  << "\"verification\":{\"releaseStatus\":\"BLOCKED\","
                  << "\"finalGateStatus\":\"PASS\",\"verified\":false},"
                  << "\"executionClass\":\"REPRESENTATIVE_SIMULATION_NOT_FLIGHT_EVIDENCE\","
                  << "\"uav\":{\"id\":\"DJI-MAVIC-3E-REFERENCE-SIM\","
                  << "\"name\":\"DJI Mavic 3 Enterprise (reference model)\","
                  << "\"camera\":\"4/3 CMOS; 5280x3956; mechanical shutter\"},"
                  << "\"environment\":{\"weatherModel\":\"SYNTHETIC_AVERAGE\","
                  << "\"temperatureC\":15.0,\"windNorthMps\":0.0,"
                  << "\"windEastMps\":1.5,\"windVerticalMps\":0.0,"
                  << "\"precipitationMmPerHour\":0.0,\"notamId\":\"SIM-NOTAM-001\","
                  << "\"notamStatus\":\"SYNTHETIC_OUTSIDE_AOI_NOT_OPERATIONAL_CLEARANCE\"},"
                  << "\"metrics\":{"
                  << "\"aoiAreaM2\":" << quality.aoi_area_m2 << ","
                  << "\"gsdWidthMPerPx\":" << geometry.gsd_width_m_per_px << ","
                  << "\"gsdHeightMPerPx\":" << geometry.gsd_height_m_per_px << ","
                  << "\"trackSpacingM\":" << geometry.track_spacing_m << ","
                  << "\"triggerIntervalS\":" << geometry.trigger_interval_s << ","
                  << "\"coverageRatio\":" << quality.coverage_ratio << ","
                  << "\"uncoveredAreaM2\":" << quality.uncovered_area_m2 << ","
                  << "\"coverageTracks\":" << tracks.tracks.size() << ","
                  << "\"acquisitionEvents\":" << events.events.size() << ","
                  << "\"edgeGaps\":" << edgeResult.gaps.size() << ","
                  << "\"routeCandidates\":" << candidates.candidates.size() << ","
                  << "\"selectedRouteId\":\"" << selection.selected_route_id << "\","
                  << "\"selectedRouteTimeS\":" << selectedTrajectory.total_time_s << ","
                  << "\"selectedRouteEnergyWh\":" << selectedTrajectory.total_energy_wh << ","
                  << "\"remainingEnergyWh\":" << selectedTrajectory.remaining_energy_wh << "},"
                  << "\"routeCoordinates\":[";
        for (std::size_t i = 0; i < routePoints.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << "{\"lat\":" << routePoints[i].latitude_deg
                      << ",\"lon\":" << routePoints[i].longitude_deg << "}";
        }
        std::cout << "],\"simulationAcceptance\":\"PASS\","
                  << "\"operationalStatus\":\"BLOCKED_NOT_EXECUTED\"}\n";
        return 0;
    }

    std::cout << "scenario_id=V-M01-01-SIM-001\n";
    std::cout << "dataset_id=" << kDatasetId << "\n";
    std::cout << "execution_class=REPRESENTATIVE_SIMULATION_NOT_FLIGHT_EVIDENCE\n";
    std::cout << "uav_reference=" << kUavId << "\n";
    std::cout << "aoi_area_m2=" << quality.aoi_area_m2 << "\n";
    std::cout << "gsd_width_m_per_px=" << geometry.gsd_width_m_per_px << "\n";
    std::cout << "gsd_height_m_per_px=" << geometry.gsd_height_m_per_px << "\n";
    std::cout << "footprint_width_m=" << geometry.footprint_width_m << "\n";
    std::cout << "footprint_height_m=" << geometry.footprint_height_m << "\n";
    std::cout << "track_spacing_m=" << geometry.track_spacing_m << "\n";
    std::cout << "image_spacing_m=" << geometry.image_spacing_m << "\n";
    std::cout << "trigger_interval_s=" << geometry.trigger_interval_s << "\n";
    std::cout << "orientation_candidates=" << orientations.candidates.size() << "\n";
    std::cout << "coverage_cells=" << decomposition.cells.size() << "\n";
    std::cout << "coverage_tracks=" << tracks.tracks.size() << "\n";
    std::cout << "edge_gaps=" << edgeResult.gaps.size() << "\n";
    std::cout << "acquisition_events=" << events.events.size() << "\n";
    std::cout << "coverage_ratio=" << quality.coverage_ratio << "\n";
    std::cout << "uncovered_area_m2=" << quality.uncovered_area_m2 << "\n";
    std::cout << "transition_edges=" << graph.edges.size() << "\n";
    std::cout << "route_candidates=" << candidates.candidates.size() << "\n";
    std::cout << "selected_route_id=" << selection.selected_route_id << "\n";
    std::cout << "selected_route_time_s=" << selectedTrajectory.total_time_s << "\n";
    std::cout << "selected_route_energy_wh=" << selectedTrajectory.total_energy_wh << "\n";
    std::cout << "selected_route_remaining_energy_wh=" << selectedTrajectory.remaining_energy_wh << "\n";
    std::cout << "simulation_acceptance=PASS\n";
    std::cout << "operational_v_m01_01=BLOCKED_NOT_EXECUTED\n";
    return 0;
}
