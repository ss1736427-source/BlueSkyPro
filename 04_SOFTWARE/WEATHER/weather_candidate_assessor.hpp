#pragma once

#include "weather_field.hpp"
#include "../PLANNING/orchestrator/algorithm_orchestrator.hpp"
#include "../PLANNING/wind_performance_trajectory.hpp"

#include <string>
#include <vector>

namespace bluesky::weather {

// Creates the production bridge between a decoded, immutable forecast snapshot
// and AlgorithmOrchestrator's pre-ranking candidate assessment hook. The caller
// supplies the performance profile for the one assigned aircraft.
bluesky::planning::CandidateWeatherAssessor makeCandidateWeatherAssessor(
    std::vector<WindField> fields,
    bluesky::planning::TrajectoryPerformanceInput performance,
    std::string snapshot_id,
    std::string snapshot_version,
    std::string calculation_version,
    int maximum_vertical_gap_m = 100);

} // namespace bluesky::weather
