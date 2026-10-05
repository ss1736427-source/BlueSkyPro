#include "multi_uav_sequencing.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace bluesky::planning {
namespace {

TrajectoryResult shifted(const TrajectoryResult& source, double delay_s) {
    TrajectoryResult result = source;
    for (auto& point : result.points) {
        point.elapsed_time_s += delay_s;
    }
    for (auto& segment : result.segments) {
        segment.traversal_time_s = segment.traversal_time_s;
    }
    std::ostringstream dep;
    dep << source.dependency_identity << "|START_DELAY:" << std::setprecision(17) << delay_s;
    result.dependency_identity = dep.str();
    return result;
}

std::vector<TrajectoryResult> applyDelays(
    const std::vector<TrajectoryResult>& trajectories,
    const std::vector<MultiUavStartDelayInput>& delays) {
    std::vector<TrajectoryResult> result;
    result.reserve(trajectories.size());
    for (const auto& trajectory : trajectories) {
        double delay = 0.0;
        for (const auto& item : delays) {
            if (item.uav_id == trajectory.uav_id) {
                delay = item.initial_delay_s;
                break;
            }
        }
        result.push_back(shifted(trajectory, delay));
    }
    return result;
}

const MultiUavStartDelayInput* findInput(
    const std::vector<MultiUavStartDelayInput>& inputs,
    const std::string& uav_id) {
    for (const auto& input : inputs) {
        if (input.uav_id == uav_id) return &input;
    }
    return nullptr;
}

} // namespace

MultiUavSequencingResult MultiUavSequencer::resolveByStartDelay(
    const std::vector<TrajectoryResult>& trajectories,
    const std::vector<MultiUavSeparationInput>& separation,
    const std::vector<MultiUavStartDelayInput>& delay_inputs,
    const std::string& calculation_version) {
    MultiUavSequencingResult result;
    result.calculation_version = calculation_version;

    std::ostringstream dep;
    dep << calculation_version;
    for (const auto& t : trajectories) dep << "|T:" << t.uav_id << ":" << t.dependency_identity;
    for (const auto& d : delay_inputs) {
        dep << "|D:" << d.uav_id << ":" << std::setprecision(17)
            << d.initial_delay_s << "," << d.delay_step_s << "," << d.maximum_delay_s;
    }
    result.dependency_identity = dep.str();

    if (trajectories.size() < 2) {
        result.status = MultiUavSequencingStatus::Resolved;
        result.scheduled_delays = delay_inputs;
        return result;
    }

    result.scheduled_delays = delay_inputs;
    for (const auto& input : delay_inputs) {
        if (input.delay_step_s <= 0.0 || input.initial_delay_s < 0.0 ||
            input.maximum_delay_s < input.initial_delay_s) {
            result.status = MultiUavSequencingStatus::Unresolved;
            result.unresolved_conflicts.push_back({
                MultiUavConflictFindingCode::InvalidTrajectory,
                input.uav_id, "", 0.0, "invalid start-delay scheduling parameters"});
            return result;
        }
    }

    constexpr std::size_t kMaxIterations = 10000;
    for (std::size_t iteration = 0; iteration < kMaxIterations; ++iteration) {
        const auto scheduled = applyDelays(trajectories, result.scheduled_delays);
        const auto conflicts = MultiUavConflictDetector::evaluate(
            scheduled, separation, calculation_version + ".conflict");
        if (conflicts.status == MultiUavConflictStatus::Clear) {
            result.status = MultiUavSequencingStatus::Resolved;
            return result;
        }

        if (conflicts.findings.empty()) {
            result.unresolved_conflicts = conflicts.findings;
            return result;
        }

        const auto& conflict = conflicts.findings.front();
        const auto* a = findInput(result.scheduled_delays, conflict.uav_a);
        const auto* b = findInput(result.scheduled_delays, conflict.uav_b);
        if (!a || !b) {
            result.unresolved_conflicts = conflicts.findings;
            return result;
        }

        auto* later = &result.scheduled_delays.front();
        if (a->uav_id < b->uav_id) {
            for (auto& item : result.scheduled_delays)
                if (item.uav_id == b->uav_id) { later = &item; break; }
        } else {
            for (auto& item : result.scheduled_delays)
                if (item.uav_id == a->uav_id) { later = &item; break; }
        }

        const double next_delay = later->initial_delay_s + later->delay_step_s;
        if (next_delay > later->maximum_delay_s) {
            result.unresolved_conflicts = conflicts.findings;
            return result;
        }
        later->initial_delay_s = next_delay;
    }

    result.unresolved_conflicts.push_back({
        MultiUavConflictFindingCode::SpatialTemporalConflict,
        "", "", 0.0, "sequencing iteration limit reached"});
    return result;
}

} // namespace bluesky::planning
