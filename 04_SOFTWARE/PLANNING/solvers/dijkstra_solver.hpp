#pragma once

#include "../solver_contract/solver_contract.hpp"
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::planning {

struct DijkstraNode {
    std::string id;
    std::vector<std::pair<std::string, double>> neighbours;
};

class DijkstraSolver final : public Solver {
public:
    SolverMetadata metadata() const override;
    bool eligible(const MissionProblem& problem) const override;
    RunState run(SolverContext& context) override;
    void cancel() override;

private:
    std::atomic_bool cancelled_{false};
};

} // namespace bluesky::planning
