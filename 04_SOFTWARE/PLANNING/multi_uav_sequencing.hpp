#pragma once

#include "multi_uav_conflict.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

enum class MultiUavSequencingStatus { Resolved, Unresolved };

struct MultiUavStartDelayInput {
    std::string uav_id;
    double initial_delay_s{0.0};
    double delay_step_s{1.0};
    double maximum_delay_s{0.0};
};

struct MultiUavSequencingResult {
    MultiUavSequencingStatus status{MultiUavSequencingStatus::Unresolved};
    std::vector<MultiUavStartDelayInput> scheduled_delays;
    std::string calculation_version;
    std::string dependency_identity;
    std::vector<MultiUavConflictFinding> unresolved_conflicts;
};

class MultiUavSequencer {
public:
    static MultiUavSequencingResult resolveByStartDelay(
        const std::vector<TrajectoryResult>& trajectories,
        const std::vector<MultiUavSeparationInput>& separation,
        const std::vector<MultiUavStartDelayInput>& delay_inputs,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
