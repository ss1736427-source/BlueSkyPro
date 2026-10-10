#pragma once

#include "../model/planning_graph.hpp"
#include <cstdint>
#include <string>
#include <vector>

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
};

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
