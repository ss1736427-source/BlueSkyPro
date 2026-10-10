#pragma once

#include "../model/planning_graph.hpp"
#include "../solver_contract/solver_contract.hpp"

#include <atomic>
#include <functional>
#include <string>
#include <vector>

namespace bluesky::planning {

class AStarSolver final : public Solver {
public:
    using Heuristic = std::function<double(const PlanningNode&, const PlanningNode&)>;

    explicit AStarSolver(Heuristic heuristic);

    SolverMetadata metadata() const override;
    bool eligible(const MissionProblem& problem) const override;
    RunState run(SolverContext& context) override;
    void cancel() override;

private:
    Heuristic heuristic_;
    std::atomic<bool> cancelled_{false};
};

} // namespace bluesky::planning
