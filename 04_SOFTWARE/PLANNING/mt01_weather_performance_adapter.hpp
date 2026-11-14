#pragma once

#include "mt01_mapping_pipeline.hpp"
#include "model/vehicle_equipment_capability.hpp"
#include "wind_performance_trajectory.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

struct Mt01WeatherPerformanceBindingResult {
    bool valid{false};
    TrajectoryPerformanceInput performance;
    std::vector<std::string> diagnostics;
};

class Mt01WeatherPerformanceAdapter final {
public:
    static Mt01WeatherPerformanceBindingResult bind(
        const Mission& mission,
        const Mt01MappingPipelineResult& pipeline,
        const CoverageCameraModel& camera,
        const VehicleEquipmentCapabilityProfile& capability,
        TrajectoryPerformanceInput base_performance);
};

} // namespace bluesky::planning
