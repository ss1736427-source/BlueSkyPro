#include "algorithm_orchestrator.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <string>

namespace bluesky::planning {

namespace {

bool supported_priority(const std::string& priority) {
    return priority == "completion_time" || priority == "time" || priority == "fast" ||
           priority == "minimum_cost" || priority == "route_efficiency" ||
           priority == "energy" || priority == "endurance" || priority == "reserve";
}

bool objective_priorities_are_supported(const MissionProblem& problem) {
    return !problem.objective_priorities.empty() &&
           std::all_of(problem.objective_priorities.begin(),
                       problem.objective_priorities.end(),
                       supported_priority);
}

bool route_matches_graph(const CandidateSolution& candidate, const MissionProblem& problem) {
    const auto* graph = problem.planning_graph;
    if (!graph) return true;
    if (candidate.route_elements.empty()) return false;
    if (candidate.route_elements.front() != graph->start_node ||
        candidate.route_elements.back() != graph->goal_node) return false;
    if (!graph->find_node(candidate.route_elements.front()) ||
        !graph->find_node(candidate.route_elements.back())) return false;

    for (std::size_t i = 1; i < candidate.route_elements.size(); ++i) {
        const auto& from = candidate.route_elements[i - 1];
        const auto& to = candidate.route_elements[i];
        if (!graph->find_node(from) || !graph->find_node(to)) return false;

        const bool edge_exists = std::any_of(
            graph->edges.begin(), graph->edges.end(),
            [&](const PlanningGraphEdge& edge) {
                return edge.from == from && edge.to == to &&
                       std::isfinite(edge.cost) && edge.cost >= 0.0;
            });
        if (!edge_exists) return false;
    }
    return true;
}

bool structurally_valid_candidate(const CandidateSolution& candidate,
                                  const MissionProblem& problem) {
    if (candidate.feasibility != Feasibility::Feasible) return false;
    if (candidate.candidate_id.empty() || candidate.solver_id.empty() ||
        candidate.solver_version.empty()) return false;
    if (candidate.route_elements.empty()) return false;
    if (std::any_of(candidate.route_elements.begin(), candidate.route_elements.end(),
                    [](const std::string& element) { return element.empty(); })) {
        return false;
    }
    if (!std::isfinite(candidate.estimated_time_s) ||
        !std::isfinite(candidate.estimated_energy_wh) ||
        !std::isfinite(candidate.estimated_reserve_wh) ||
        !std::isfinite(candidate.objective_score)) return false;
    if (candidate.estimated_time_s < 0.0 ||
        candidate.estimated_energy_wh < 0.0 ||
        candidate.estimated_reserve_wh < 0.0) return false;
    return route_matches_graph(candidate, problem);
}

bool accepts_candidates(RunState state) {
    return state == RunState::Candidate || state == RunState::Completed;
}

bool better_candidate(const CandidateSolution& candidate,
                      const CandidateSolution& current,
                      const MissionProblem& problem) {
    const auto better_lower = [](double lhs, double rhs) {
        return lhs + 1e-9 < rhs;
    };
    const auto better_higher = [](double lhs, double rhs) {
        return lhs > rhs + 1e-9;
    };

    for (const auto& priority : problem.objective_priorities) {
        if (priority == "completion_time" || priority == "time" || priority == "fast") {
            if (better_lower(candidate.estimated_time_s, current.estimated_time_s)) return true;
            if (better_lower(current.estimated_time_s, candidate.estimated_time_s)) return false;
        } else if (priority == "minimum_cost" || priority == "route_efficiency") {
            if (better_lower(candidate.objective_score, current.objective_score)) return true;
            if (better_lower(current.objective_score, candidate.objective_score)) return false;
        } else if (priority == "energy") {
            if (better_lower(candidate.estimated_energy_wh, current.estimated_energy_wh)) return true;
            if (better_lower(current.estimated_energy_wh, candidate.estimated_energy_wh)) return false;
        } else if (priority == "endurance" || priority == "reserve") {
            if (better_higher(candidate.estimated_reserve_wh, current.estimated_reserve_wh)) return true;
            if (better_higher(current.estimated_reserve_wh, candidate.estimated_reserve_wh)) return false;
        }
    }

    // No implicit optimization criterion is allowed after the declared priorities.
    // Once all declared objectives are tied, candidate_id is the sole stable tie-breaker.
    return candidate.candidate_id < current.candidate_id;
}

} // namespace

AlgorithmOrchestrator::AlgorithmOrchestrator(
    std::vector<std::unique_ptr<Solver>> solvers,
    CandidateConstraintValidator constraint_validator)
    : solvers_(std::move(solvers)),
      constraint_validator_(std::move(constraint_validator)) {}

OrchestratorDecision AlgorithmOrchestrator::solve(SolverContext& context) {
    OrchestratorDecision decision;
    CandidateSolution best;
    bool have_best = false;

    if (!objective_priorities_are_supported(context.problem())) {
        decision.feasibility = Feasibility::Uncertain;
        decision.explanation = context.problem().objective_priorities.empty()
            ? "Расчёт остановлен: для задачи не определены приоритеты выбора маршрута."
            : "Расчёт остановлен: задача содержит неподдерживаемый приоритет выбора маршрута.";
        return decision;
    }

    for (auto& solver : solvers_) {
        if (context.cancelled()) {
            if (solver) solver->cancel();
            decision.feasibility = Feasibility::Uncertain;
            decision.explanation = "Расчёт отменён до завершения поиска допустимого маршрута.";
            return decision;
        }

        if (!solver || !solver->eligible(context.problem())) {
            if (solver) decision.rejected_solvers.push_back(solver->metadata().solver_id);
            continue;
        }

        const auto meta = solver->metadata();
        decision.considered_solvers.push_back(meta.solver_id);

        const auto state = solver->run(context);
        if (context.cancelled()) {
            solver->cancel();
            decision.feasibility = Feasibility::Uncertain;
            decision.explanation = "Расчёт отменён до завершения оценки кандидатов.";
            return decision;
        }
        if (!accepts_candidates(state)) continue;

        for (const auto& candidate : context.candidates()) {
            if (candidate.solver_id != meta.solver_id ||
                candidate.solver_version != meta.version) {
                continue;
            }

            std::vector<std::string> violations = candidate.constraint_violations;
            if (constraint_validator_) {
                const auto validated_violations =
                    constraint_validator_(context.problem(), candidate);
                violations.insert(violations.end(),
                                  validated_violations.begin(),
                                  validated_violations.end());
            }

            if (!structurally_valid_candidate(candidate, context.problem())) {
                if (violations.empty()) {
                    violations.push_back("Кандидат не прошёл структурную проверку допустимости.");
                }
                decision.rejected_candidates.push_back(
                    {candidate.candidate_id, candidate.solver_id, std::move(violations)});
                continue;
            }

            // Mandatory constraints are evaluated before any objective ranking.
            if (!violations.empty()) {
                decision.rejected_candidates.push_back(
                    {candidate.candidate_id, candidate.solver_id, std::move(violations)});
                continue;
            }

            if (!have_best || better_candidate(candidate, best, context.problem())) {
                best = candidate;
                have_best = true;
            }
        }
    }

    if (!have_best) {
        decision.feasibility = Feasibility::Infeasible;
        decision.explanation = decision.rejected_candidates.empty()
            ? "Допустимый маршрут не найден доступными алгоритмами."
            : "Допустимый маршрут не найден: все полученные кандидаты отклонены проверкой допустимости.";
        return decision;
    }

    decision.feasibility = Feasibility::Feasible;
    decision.selected_solver_id = best.solver_id;
    decision.selected_candidate_id = best.candidate_id;

    std::ostringstream explanation;
    explanation << "Выбран маршрут, рассчитанный алгоритмом " << best.solver_id
                << ", как лучший допустимый кандидат с учётом приоритетов текущей задачи."
                << " Обязательные ограничения проверены до ранжирования.";
    decision.explanation = explanation.str();
    return decision;
}

} // namespace bluesky::planning
