#pragma once

#include "../model/planning_graph.hpp"
#include "../model/flight_safety_constraints.hpp"
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace bluesky::planning {

enum class Feasibility { Feasible, Infeasible, Uncertain, Partial };
enum class RunState { Started, Running, Candidate, Completed, Timeout, Cancelled, Failed };

struct ComputeBudget {
    std::uint64_t wall_time_ms{0};
    std::uint64_t memory_mb{0};
    std::uint32_t max_candidates{0};
};

struct MissionProblem {
    std::string mission_id;
    std::string mission_version;
    std::string problem_class;
    std::vector<std::string> vehicle_ids;
    std::vector<std::string> hard_constraints;
    std::vector<std::string> objective_priorities;
    std::string environment_version;
    const PlanningGraph* planning_graph{nullptr};
    // Flight tasks must opt in explicitly; missing safety evidence then blocks candidates.
    bool requires_flight_safety_validation{false};
    // Current contract is single-aircraft only; this must match the sole vehicle_ids entry.
    std::string flight_safety_vehicle_id;
    std::optional<FlightSafetyEnvelope> flight_safety_envelope;
};

struct CandidateSolution {
    std::string candidate_id;
    std::string solver_id;
    std::string solver_version;
    std::vector<std::string> route_elements;
    std::vector<std::string> constraint_violations;
    double estimated_time_s{0.0};
    double estimated_energy_wh{0.0};
    double estimated_reserve_wh{0.0};
    double objective_score{0.0};
    Feasibility feasibility{Feasibility::Uncertain};
    // One state per flown trajectory segment; IDs must be unique and non-empty.
    std::vector<FlightSafetySegmentState> flight_dynamics;
    // Producer attestation: profile covers every flown segment, including transitions.
    bool flight_dynamics_complete{false};
    std::size_t flight_dynamics_expected_segment_count{0};
};

// Standard handoff for route-producing solvers. Incomplete structural routes
// remain explicitly incomplete and are rejected by the orchestrator flight gate.
inline void attachFlightSafetyDynamics(CandidateSolution& candidate, const Route& route)
{
    auto profile = buildFlightSafetyCandidateDynamics(route);
    candidate.flight_dynamics = std::move(profile.segments);
    candidate.flight_dynamics_complete = profile.complete;
    candidate.flight_dynamics_expected_segment_count = profile.expected_segment_count;
}

struct SolverMetadata {
    std::string solver_id;
    std::string version;
    std::vector<std::string> problem_classes;
    bool supports_incremental_replan{false};
    bool deterministic{true};
};

class SolverContext {
public:
    virtual ~SolverContext() = default;
    virtual const MissionProblem& problem() const = 0;
    virtual const ComputeBudget& budget() const = 0;
    virtual bool cancelled() const = 0;
    virtual void publish(CandidateSolution candidate) = 0;
    virtual const std::vector<CandidateSolution>& candidates() const = 0;
};

class Solver {
public:
    virtual ~Solver() = default;
    virtual SolverMetadata metadata() const = 0;
    virtual bool eligible(const MissionProblem& problem) const = 0;
    virtual RunState run(SolverContext& context) = 0;
    virtual void cancel() = 0;
};

} // namespace bluesky::planning
