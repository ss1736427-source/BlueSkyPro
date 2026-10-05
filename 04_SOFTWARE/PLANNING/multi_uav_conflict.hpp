#pragma once

#include "wind_performance_trajectory.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

enum class MultiUavConflictStatus { Clear, Conflict };

enum class MultiUavConflictFindingCode {
    InvalidTrajectory,
    SpatialTemporalConflict
};

struct MultiUavSeparationInput {
    std::string uav_id;
    double minimum_horizontal_separation_m{0.0};
    double minimum_vertical_separation_m{0.0};
    double temporal_tolerance_s{0.0};
};

struct MultiUavConflictFinding {
    MultiUavConflictFindingCode code;
    std::string uav_a;
    std::string uav_b;
    double time_s{0.0};
    std::string detail;
};

struct MultiUavConflictResult {
    MultiUavConflictStatus status{MultiUavConflictStatus::Clear};
    std::string calculation_version;
    std::string dependency_identity;
    std::vector<MultiUavConflictFinding> findings;
};

class MultiUavConflictDetector {
public:
    static MultiUavConflictResult evaluate(
        const std::vector<TrajectoryResult>& trajectories,
        const std::vector<MultiUavSeparationInput>& separation,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
