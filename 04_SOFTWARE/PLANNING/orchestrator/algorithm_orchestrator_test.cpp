#include "algorithm_orchestrator.hpp"
#include "../model/planning_graph.hpp"
#include "../solvers/astar_solver.hpp"
#include "../solvers/dijkstra_solver.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <vector>

using namespace bluesky::planning;

namespace {
class TestContext final : public SolverContext {
public:
    TestContext(const MissionProblem& p, ComputeBudget b) : p_(p), b_(b) {}
    const MissionProblem& problem() const override { return p_; }
    const ComputeBudget& budget() const override { return b_; }
    bool cancelled() const override { return cancelled_; }
    void publish(CandidateSolution c) override { candidates_.push_back(std::move(c)); }
    const std::vector<CandidateSolution>& candidates() const override { return candidates_; }
    void set_cancelled(bool value) { cancelled_ = value; }
private:
    const MissionProblem& p_;
    ComputeBudget b_;
    bool cancelled_{false};
    std::vector<CandidateSolution> candidates_;
};

class RankingSolver final : public Solver {
public:
    RankingSolver(std::string id, double time_s, double score)
        : id_(std::move(id)), time_s_(time_s), score_(score) {}
    SolverMetadata metadata() const override { return {id_, "test", {"point_to_point"}, false, true}; }
    bool eligible(const MissionProblem& problem) const override {
        return problem.problem_class == "point_to_point";
    }
    RunState run(SolverContext& context) override {
        CandidateSolution candidate;
        candidate.candidate_id = context.problem().mission_id + ":" + id_ + ":1";
        candidate.solver_id = id_;
        candidate.solver_version = "test";
        candidate.route_elements = {"A", "B"};
        candidate.estimated_time_s = time_s_;
        candidate.objective_score = score_;
        candidate.feasibility = Feasibility::Feasible;
        context.publish(std::move(candidate));
        return RunState::Completed;
    }
    void cancel() override {}
private:
    std::string id_;
    double time_s_;
    double score_;
};

class InvalidCandidateSolver final : public Solver {
public:
    SolverMetadata metadata() const override { return {"invalid", "test", {"point_to_point"}, false, true}; }
    bool eligible(const MissionProblem& problem) const override {
        return problem.problem_class == "point_to_point";
    }
    RunState run(SolverContext& context) override {
        CandidateSolution candidate;
        candidate.candidate_id = context.problem().mission_id + ":invalid:1";
        candidate.solver_id = "invalid";
        candidate.solver_version.clear();
        candidate.route_elements = {"A", "B"};
        candidate.estimated_time_s = 1.0;
        candidate.objective_score = 0.1;
        candidate.feasibility = Feasibility::Feasible;
        context.publish(std::move(candidate));
        return RunState::Completed;
    }
    void cancel() override {}
};

class TimeoutCandidateSolver final : public Solver {
public:
    SolverMetadata metadata() const override { return {"timeout-candidate", "test", {"point_to_point"}, false, true}; }
    bool eligible(const MissionProblem& problem) const override {
        return problem.problem_class == "point_to_point";
    }
    RunState run(SolverContext& context) override {
        CandidateSolution candidate;
        candidate.candidate_id = context.problem().mission_id + ":timeout-candidate:1";
        candidate.solver_id = "timeout-candidate";
        candidate.solver_version = "test";
        candidate.route_elements = {"A", "B"};
        candidate.estimated_time_s = 1.0;
        candidate.objective_score = 0.1;
        candidate.feasibility = Feasibility::Feasible;
        context.publish(std::move(candidate));
        return RunState::Timeout;
    }
    void cancel() override {}
};

class HardConstraintViolationSolver final : public Solver {
public:
    SolverMetadata metadata() const override { return {"violating", "test", {"point_to_point"}, false, true}; }
    bool eligible(const MissionProblem& problem) const override {
        return problem.problem_class == "point_to_point";
    }
    RunState run(SolverContext& context) override {
        CandidateSolution candidate;
        candidate.candidate_id = context.problem().mission_id + ":violating:1";
        candidate.solver_id = "violating";
        candidate.solver_version = "test";
        candidate.route_elements = {"A", "B"};
        candidate.estimated_time_s = 1.0;
        candidate.objective_score = 0.1;
        candidate.constraint_violations = {"restricted_airspace"};
        candidate.feasibility = Feasibility::Feasible;
        context.publish(std::move(candidate));
        return RunState::Completed;
    }
    void cancel() override {}
};

std::vector<std::unique_ptr<Solver>> make_solvers() {
    auto heuristic = [](const PlanningNode& from, const PlanningNode& to) {
        const double dx = from.x - to.x;
        const double dy = from.y - to.y;
        return std::sqrt(dx * dx + dy * dy);
    };
    std::vector<std::unique_ptr<Solver>> solvers;
    solvers.push_back(std::make_unique<AStarSolver>(heuristic));
    solvers.push_back(std::make_unique<DijkstraSolver>());
    return solvers;
}
}

int main() {
    PlanningGraph graph;
    graph.nodes.push_back({"A", 0, 0});
    graph.nodes.push_back({"B", 1, 0});
    graph.nodes.push_back({"C", 0, 1});
    graph.nodes.push_back({"D", 1, 1});
    graph.edges = {
        {"A", "B", 10.0},
        {"A", "C", 2.0},
        {"C", "D", 2.0},
        {"D", "B", 1.0}
    };
    graph.start_node = "A";
    graph.goal_node = "B";

    MissionProblem problem;
    problem.mission_id = "ORCH-REAL-001";
    problem.problem_class = "point_to_point";
    problem.planning_graph = &graph;
    problem.objective_priorities = {"completion_time"};

    TestContext context(problem, ComputeBudget{1000, 256, 8});
    AlgorithmOrchestrator orchestrator(make_solvers());
    const auto decision = orchestrator.solve(context);

    assert(decision.feasibility == Feasibility::Feasible);
    assert(decision.considered_solvers.size() == 2);
    assert(decision.considered_solvers[0] == "astar");
    assert(decision.considered_solvers[1] == "dijkstra");
    assert(decision.selected_solver_id == "astar");
    assert(decision.selected_candidate_id == "ORCH-REAL-001:astar:1");
    assert(context.candidates().size() == 2);
    assert(std::abs(context.candidates()[0].estimated_time_s - 5.0) < 1e-9);
    assert(std::abs(context.candidates()[1].estimated_time_s - 5.0) < 1e-9);

    MissionProblem cost_problem = problem;
    cost_problem.mission_id = "ORCH-PROFILE-002";
    cost_problem.objective_priorities = {"minimum_cost"};
    TestContext cost_context(cost_problem, ComputeBudget{1000, 256, 8});
    AlgorithmOrchestrator cost_orchestrator(make_solvers());
    const auto cost_decision = cost_orchestrator.solve(cost_context);

    assert(cost_decision.feasibility == Feasibility::Feasible);
    assert(cost_decision.considered_solvers.size() == 2);
    assert(cost_decision.considered_solvers[0] == "dijkstra");
    assert(cost_decision.considered_solvers[1] == "astar");
    assert(cost_decision.selected_solver_id == "dijkstra");
    assert(cost_decision.selected_candidate_id == "ORCH-PROFILE-002:dijkstra:1");
    assert(cost_context.candidates().size() == 2);

    PlanningGraph zero_information_graph = graph;
    zero_information_graph.nodes[1].x = 0.0;
    zero_information_graph.nodes[1].y = 0.0;
    zero_information_graph.start_node = "A";
    zero_information_graph.goal_node = "B";

    MissionProblem zero_information_problem = problem;
    zero_information_problem.mission_id = "ORCH-GRAPH-003";
    zero_information_problem.planning_graph = &zero_information_graph;
    TestContext zero_information_context(
        zero_information_problem, ComputeBudget{1000, 256, 8});
    AlgorithmOrchestrator zero_information_orchestrator(make_solvers());
    const auto zero_information_decision = zero_information_orchestrator.solve(zero_information_context);

    assert(zero_information_decision.feasibility == Feasibility::Feasible);
    assert(zero_information_decision.considered_solvers.size() == 2);
    assert(zero_information_decision.considered_solvers[0] == "dijkstra");
    assert(zero_information_decision.considered_solvers[1] == "astar");
    assert(zero_information_decision.selected_solver_id == "dijkstra");
    assert(zero_information_decision.selected_candidate_id == "ORCH-GRAPH-003:dijkstra:1");
    assert(zero_information_context.candidates().size() == 2);

    MissionProblem no_heuristic_problem = problem;
    no_heuristic_problem.mission_id = "ORCH-CONDITION-004";
    TestContext fallback_context(no_heuristic_problem, ComputeBudget{1000, 256, 8});
    std::vector<std::unique_ptr<Solver>> fallback_solvers;
    fallback_solvers.push_back(std::make_unique<AStarSolver>(AStarSolver::Heuristic{}));
    fallback_solvers.push_back(std::make_unique<DijkstraSolver>());

    AlgorithmOrchestrator fallback_orchestrator(std::move(fallback_solvers));
    const auto fallback_decision = fallback_orchestrator.solve(fallback_context);

    assert(fallback_decision.feasibility == Feasibility::Feasible);
    assert(fallback_decision.considered_solvers.size() == 1);
    assert(fallback_decision.considered_solvers[0] == "dijkstra");
    assert(fallback_decision.selected_solver_id == "dijkstra");
    assert(fallback_decision.selected_candidate_id == "ORCH-CONDITION-004:dijkstra:1");
    assert(fallback_context.candidates().size() == 1);
    assert(std::abs(fallback_context.candidates()[0].objective_score - 5.0) < 1e-9);

    MissionProblem ranking_problem = problem;
    ranking_problem.mission_id = "ORCH-RANKING-005";
    ranking_problem.objective_priorities = {"minimum_cost"};
    TestContext ranking_context(ranking_problem, ComputeBudget{1000, 256, 8});
    std::vector<std::unique_ptr<Solver>> ranking_solvers;
    ranking_solvers.push_back(std::make_unique<RankingSolver>("slow-cheap", 20.0, 5.0));
    ranking_solvers.push_back(std::make_unique<RankingSolver>("fast-expensive", 10.0, 9.0));
    AlgorithmOrchestrator ranking_orchestrator(std::move(ranking_solvers));
    const auto ranking_decision = ranking_orchestrator.solve(ranking_context);

    assert(ranking_decision.feasibility == Feasibility::Feasible);
    assert(ranking_decision.selected_solver_id == "slow-cheap");
    assert(ranking_decision.selected_candidate_id == "ORCH-RANKING-005:slow-cheap:1");

    MissionProblem validation_problem = problem;
    validation_problem.mission_id = "ORCH-VALIDATION-006";
    TestContext validation_context(validation_problem, ComputeBudget{1000, 256, 8});
    std::vector<std::unique_ptr<Solver>> validation_solvers;
    validation_solvers.push_back(std::make_unique<InvalidCandidateSolver>());
    validation_solvers.push_back(std::make_unique<RankingSolver>("valid", 5.0, 1.0));
    AlgorithmOrchestrator validation_orchestrator(std::move(validation_solvers));
    const auto validation_decision = validation_orchestrator.solve(validation_context);

    assert(validation_decision.feasibility == Feasibility::Feasible);
    assert(validation_decision.selected_solver_id == "valid");
    assert(validation_decision.selected_candidate_id == "ORCH-VALIDATION-006:valid:1");
    assert(validation_context.candidates().size() == 2);

    MissionProblem timeout_problem = problem;
    timeout_problem.mission_id = "ORCH-STATE-007";
    TestContext timeout_context(timeout_problem, ComputeBudget{1000, 256, 8});
    std::vector<std::unique_ptr<Solver>> timeout_solvers;
    timeout_solvers.push_back(std::make_unique<TimeoutCandidateSolver>());
    timeout_solvers.push_back(std::make_unique<RankingSolver>("valid", 5.0, 1.0));
    AlgorithmOrchestrator timeout_orchestrator(std::move(timeout_solvers));
    const auto timeout_decision = timeout_orchestrator.solve(timeout_context);

    assert(timeout_decision.feasibility == Feasibility::Feasible);
    assert(timeout_decision.selected_solver_id == "valid");
    assert(timeout_decision.selected_candidate_id == "ORCH-STATE-007:valid:1");
    assert(timeout_context.candidates().size() == 2);

    MissionProblem cancelled_problem = problem;
    cancelled_problem.mission_id = "ORCH-CANCEL-008";
    TestContext cancelled_context(cancelled_problem, ComputeBudget{1000, 256, 8});
    cancelled_context.set_cancelled(true);
    AlgorithmOrchestrator cancelled_orchestrator(make_solvers());
    const auto cancelled_decision = cancelled_orchestrator.solve(cancelled_context);

    assert(cancelled_decision.feasibility == Feasibility::Uncertain);
    assert(cancelled_decision.considered_solvers.empty());
    assert(cancelled_decision.selected_solver_id.empty());
    assert(cancelled_decision.selected_candidate_id.empty());
    assert(cancelled_context.candidates().empty());

    MissionProblem efficiency_problem = problem;
    efficiency_problem.mission_id = "ORCH-EFFICIENCY-009";
    efficiency_problem.objective_priorities = {"route_efficiency"};
    TestContext efficiency_context(efficiency_problem, ComputeBudget{1000, 256, 8});
    std::vector<std::unique_ptr<Solver>> efficiency_solvers;
    efficiency_solvers.push_back(std::make_unique<RankingSolver>("long-efficient", 20.0, 5.0));
    efficiency_solvers.push_back(std::make_unique<RankingSolver>("short-inefficient", 10.0, 9.0));
    AlgorithmOrchestrator efficiency_orchestrator(std::move(efficiency_solvers));
    const auto efficiency_decision = efficiency_orchestrator.solve(efficiency_context);

    assert(efficiency_decision.feasibility == Feasibility::Feasible);
    assert(efficiency_decision.selected_solver_id == "long-efficient");
    assert(efficiency_decision.selected_candidate_id == "ORCH-EFFICIENCY-009:long-efficient:1");

    MissionProblem ordered_problem = problem;
    ordered_problem.mission_id = "ORCH-ORDER-010";
    ordered_problem.objective_priorities = {"completion_time", "minimum_cost"};
    TestContext ordered_context(ordered_problem, ComputeBudget{1000, 256, 8});
    std::vector<std::unique_ptr<Solver>> ordered_solvers;
    ordered_solvers.push_back(std::make_unique<RankingSolver>("cheap-slow", 20.0, 1.0));
    ordered_solvers.push_back(std::make_unique<RankingSolver>("fast-expensive", 10.0, 9.0));
    AlgorithmOrchestrator ordered_orchestrator(std::move(ordered_solvers));
    const auto ordered_decision = ordered_orchestrator.solve(ordered_context);

    assert(ordered_decision.feasibility == Feasibility::Feasible);
    assert(ordered_decision.selected_solver_id == "fast-expensive");
    assert(ordered_decision.selected_candidate_id == "ORCH-ORDER-010:fast-expensive:1");

    // Hard constraints are feasibility gates: a violating candidate must never win ranking.
    MissionProblem constraints_problem = problem;
    constraints_problem.mission_id = "ORCH-CONSTRAINTS-011";
    constraints_problem.hard_constraints = {"restricted_airspace"};
    TestContext constraints_context(constraints_problem, ComputeBudget{1000, 256, 8});
    std::vector<std::unique_ptr<Solver>> constraint_solvers;
    constraint_solvers.push_back(std::make_unique<HardConstraintViolationSolver>());
    constraint_solvers.push_back(std::make_unique<RankingSolver>("safe", 5.0, 1.0));
    AlgorithmOrchestrator constraints_orchestrator(std::move(constraint_solvers));
    const auto constraints_decision = constraints_orchestrator.solve(constraints_context);

    assert(constraints_decision.feasibility == Feasibility::Feasible);
    assert(constraints_decision.selected_solver_id == "safe");
    assert(constraints_decision.selected_candidate_id == "ORCH-CONSTRAINTS-011:safe:1");
    assert(constraints_context.candidates().size() == 2);

    // If every candidate violates a hard constraint, no route may be selected.
    MissionProblem all_invalid_problem = problem;
    all_invalid_problem.mission_id = "ORCH-CONSTRAINTS-012";
    all_invalid_problem.hard_constraints = {"restricted_airspace"};
    TestContext all_invalid_context(all_invalid_problem, ComputeBudget{1000, 256, 8});
    std::vector<std::unique_ptr<Solver>> all_invalid_solvers;
    all_invalid_solvers.push_back(std::make_unique<HardConstraintViolationSolver>());
    AlgorithmOrchestrator all_invalid_orchestrator(std::move(all_invalid_solvers));
    const auto all_invalid_decision = all_invalid_orchestrator.solve(all_invalid_context);

    assert(all_invalid_decision.feasibility == Feasibility::Infeasible);
    assert(all_invalid_decision.selected_solver_id.empty());
    assert(all_invalid_decision.selected_candidate_id.empty());

    return 0;
}
