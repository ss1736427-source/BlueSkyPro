#pragma once

#include "../solver_contract/solver_contract.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::planning::benchmark {

struct BenchmarkResult {
    std::string solver_id;
    std::string solver_version;
    std::string problem_id;
    RunState state{RunState::Failed};
    Feasibility feasibility{Feasibility::Uncertain};
    double route_cost{0.0};
    double wall_time_ms{0.0};
    std::size_t route_nodes{0};
};

struct BenchmarkReport {
    std::string problem_id;
    std::vector<BenchmarkResult> results;
};

BenchmarkReport run_astar_dijkstra_benchmark(const MissionProblem& problem,
                                             std::uint64_t repetitions = 1);

} // namespace bluesky::planning::benchmark
