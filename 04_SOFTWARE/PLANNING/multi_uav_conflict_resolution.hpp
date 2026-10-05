#pragma once

#include "multi_uav_conflict.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

enum class MultiUavResolutionStatus { Resolved, Infeasible, Unresolved };

enum class MultiUavResolutionDirection { Up, Down };

enum class MultiUavResolutionFindingCode {
    StartDelayApplied,
    SameAltitudeConflict,
    RightOfWayAssignment,
    VerticalResolutionApplied,
    VerticalResolutionBlocked,
    ResidualConflict,
    AmbiguousCourseSide
};

struct MultiUavResolutionInput {
    std::string uav_id;
    double vertical_resolution_step_m{1.0};
    double minimum_altitude_m{0.0};
    double maximum_altitude_m{0.0};
};

struct MultiUavResolutionFinding {
    MultiUavResolutionFindingCode code;
    std::string uav_id;
    MultiUavResolutionDirection direction{MultiUavResolutionDirection::Up};
    double time_s{0.0};
    std::string detail;
};

struct MultiUavResolutionResult {
    MultiUavResolutionStatus status{MultiUavResolutionStatus::Unresolved};
    std::vector<TrajectoryResult> trajectories;
    std::string calculation_version;
    std::string dependency_identity;
    std::vector<MultiUavResolutionFinding> findings;
    std::vector<MultiUavConflictFinding> residual_conflicts;
};

class MultiUavConflictResolver {
public:
    static MultiUavResolutionResult resolve(
        const std::vector<TrajectoryResult>& trajectories,
        const std::vector<MultiUavSeparationInput>& separation,
        const std::vector<MultiUavResolutionInput>& resolution_inputs,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
