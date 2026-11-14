#include "algorithm_orchestrator.hpp"
#include "../notam_prohibited_zone_validator.hpp"

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

std::vector<std::string> validateRouteSpeedProfile(
    const Route& route, const FlightSafetyEnvelope& envelope) {
    std::vector<std::string> violations;
    if (!route.speed_profile) return violations; // Legacy constant-speed candidates remain supported.
    const auto& profile = *route.speed_profile;
    const auto fail = [&](const std::string& detail) {
        violations.push_back("SPEED_PROFILE_VALIDATION: " + detail);
    };
    const std::string requiredReference =
        envelope.motion_class == MotionClass::FixedWing ? "TAS" : "AIR_RELATIVE";
    if (profile.vehicle_model_id != envelope.vehicle_model_id
        || profile.vehicle_model_version != envelope.model_version
        || profile.configuration_id != envelope.configuration_id
        || profile.speed_reference != requiredReference
        || profile.dynamics_source.empty()) {
        fail("профиль не привязан к точной модели, версии, конфигурации и системе отсчёта назначенного БВС.");
        return violations;
    }
    if (profile.segments.size() != route.segments.size() || profile.segments.empty()) {
        fail("профиль не покрывает все сегменты канонического маршрута.");
        return violations;
    }
    if (!envelope.minimum_speed_mps || !envelope.maximum_speed_mps) {
        fail("для проверки профиля отсутствуют подтверждённые нижняя и верхняя границы скорости БВС.");
        return violations;
    }
    double previousExit = 0.0;
    for (std::size_t i = 0; i < profile.segments.size(); ++i) {
        const auto& p = profile.segments[i];
        const auto& s = route.segments[i];
        const double tolerance = std::max(0.5, s.distance_m * 0.01);
        const double phaseDistance = p.acceleration_distance_m
            + p.deceleration_distance_m + p.cruise_distance_m;
        const auto positive = [](double value) {
            return std::isfinite(value) && value > 0.0;
        };
        if (p.segment_id != s.segment_id || !positive(p.distance_m)
            || !std::isfinite(s.distance_m) || s.distance_m <= 0.0
            || std::abs(p.distance_m - s.distance_m) > tolerance
            || !positive(p.entry_speed_mps) || !positive(p.exit_speed_mps)
            || !positive(p.maximum_reachable_speed_mps)
            || !positive(p.maximum_acceleration_mps2)
            || !positive(p.maximum_deceleration_mps2)
            || !std::isfinite(p.acceleration_distance_m)
            || !std::isfinite(p.deceleration_distance_m)
            || !std::isfinite(p.cruise_distance_m)
            || p.acceleration_distance_m < 0.0 || p.deceleration_distance_m < 0.0
            || p.cruise_distance_m < 0.0
            || std::abs(phaseDistance - s.distance_m) > tolerance
            || p.maximum_reachable_speed_mps + 1e-9
                < std::max(p.entry_speed_mps, p.exit_speed_mps)
            || p.entry_speed_mps < *envelope.minimum_speed_mps - 1e-6
            || p.exit_speed_mps < *envelope.minimum_speed_mps - 1e-6
            || p.maximum_reachable_speed_mps > *envelope.maximum_speed_mps + 1e-6
            || (i > 0 && std::abs(p.entry_speed_mps - previousExit) > 1e-6)) {
            fail("сегмент " + s.segment_id
                 + " не прошёл проверку порядка, расстояния, непрерывности или скоростных границ.");
            return violations;
        }
        const double expectedAccelerationDistance = std::max(0.0,
            (p.maximum_reachable_speed_mps * p.maximum_reachable_speed_mps
             - p.entry_speed_mps * p.entry_speed_mps)
            / (2.0 * p.maximum_acceleration_mps2));
        const double expectedDecelerationDistance = std::max(0.0,
            (p.maximum_reachable_speed_mps * p.maximum_reachable_speed_mps
             - p.exit_speed_mps * p.exit_speed_mps)
            / (2.0 * p.maximum_deceleration_mps2));
        if (std::abs(expectedAccelerationDistance - p.acceleration_distance_m) > tolerance
            || std::abs(expectedDecelerationDistance - p.deceleration_distance_m) > tolerance) {
            fail("сегмент " + s.segment_id
                 + " содержит ускорение/торможение, не соответствующее заявленным динамическим пределам.");
            return violations;
        }
        previousExit = p.exit_speed_mps;
    }
    return violations;
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
    CandidateConstraintValidator constraint_validator,
    CandidateWeatherAssessor weather_assessor)
    : solvers_(std::move(solvers)),
      constraint_validator_(std::move(constraint_validator)),
      weather_assessor_(std::move(weather_assessor)) {}

OrchestratorDecision AlgorithmOrchestrator::solve(SolverContext& context) {
    OrchestratorDecision decision;
    CandidateSolution best;
    if (context.problem().requires_notam_validation &&
        !context.problem().notam_snapshot) {
        decision.feasibility = Feasibility::Uncertain;
        decision.explanation =
            "Расчёт остановлен: для обязательной проверки NOTAM после погодного расчёта отсутствует подтверждённый снимок ограничений.";
        return decision;
    }
    std::vector<MaximumRegimeUse> bestMaximumRegimeUses;
    bool bestMaximumRegimeWarning = false;
    std::string bestOperatorWarning;
    CandidateWeatherAssessment bestWeatherAssessment;
    bool bestWeatherAssessmentApplied = false;
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
            FlightSafetyValidationResult flightSafety;
            if (context.problem().requires_flight_safety_validation) {
                if (context.problem().vehicle_ids.size() != 1 ||
                    context.problem().flight_safety_vehicle_id.empty() ||
                    context.problem().vehicle_ids.front() != context.problem().flight_safety_vehicle_id) {
                    violations.push_back("FLIGHT_SAFETY_UNKNOWN: текущий safety-контракт требует ровно один назначенный БВС и точную привязку контура к его идентификатору.");
                } else if (!context.problem().flight_safety_envelope.has_value()) {
                    violations.push_back("FLIGHT_SAFETY_UNKNOWN: отсутствует подтверждённый контур ограничений выбранной конфигурации БВС.");
                } else if (!candidate.flight_dynamics_complete ||
                           candidate.flight_dynamics_expected_segment_count == 0 ||
                           candidate.flight_dynamics.size() !=
                               candidate.flight_dynamics_expected_segment_count) {
                    violations.push_back("FLIGHT_SAFETY_UNKNOWN: динамический профиль не подтверждён как полный для всех участков траектории, включая переходы и развороты.");
                } else {
                    flightSafety = FlightSafetyConstraintValidator::validate(
                        *context.problem().flight_safety_envelope, candidate.flight_dynamics);
                    if (flightSafety.status != FlightSafetyStatus::Allowed) {
                        if (flightSafety.findings.empty()) {
                            violations.push_back("FLIGHT_SAFETY_UNKNOWN: динамические ограничения БВС не подтверждены.");
                        } else {
                            for (const auto& finding : flightSafety.findings) {
                                std::ostringstream reason;
                                reason << "FLIGHT_SAFETY: " << finding.detail;
                                if (!finding.segment_id.empty()) reason << " Участок: " << finding.segment_id << ".";
                                violations.push_back(reason.str());
                            }
                        }
                    }
                }
            }
            if (context.problem().requires_flight_safety_validation
                && context.problem().flight_safety_envelope.has_value()
                && candidate.trajectory_route.has_value()) {
                const auto profileViolations = validateRouteSpeedProfile(
                    *candidate.trajectory_route,
                    *context.problem().flight_safety_envelope);
                violations.insert(violations.end(), profileViolations.begin(),
                                  profileViolations.end());
            }

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

            CandidateSolution rankedCandidate = candidate;
            CandidateWeatherAssessment weatherAssessment;
            if (violations.empty() && weather_assessor_) {
                if (!candidate.trajectory_route.has_value()
                    || !candidate.trajectory_route->trajectory_geometry_complete
                    || candidate.trajectory_route->waypoints.size() < 2
                    || candidate.trajectory_route->segments.size() + 1
                        != candidate.trajectory_route->waypoints.size()) {
                    violations.push_back(
                        "WEATHER_ASSESSMENT_UNKNOWN: кандидат не содержит полной канонической геометрии траектории, включая переходы и развороты.");
                } else {
                    weatherAssessment = weather_assessor_(context.problem(), candidate);
                    const bool validWeatherResult =
                        weatherAssessment.evaluated
                        && !weatherAssessment.snapshot_id.empty()
                        && !weatherAssessment.snapshot_version.empty()
                        && std::isfinite(weatherAssessment.corrected_time_s)
                        && weatherAssessment.corrected_time_s > 0.0
                        && std::isfinite(weatherAssessment.corrected_energy_wh)
                        && weatherAssessment.corrected_energy_wh >= 0.0
                        && std::isfinite(weatherAssessment.remaining_energy_wh)
                        && weatherAssessment.remaining_energy_wh >= 0.0;
                    if (!validWeatherResult) {
                        std::string reason =
                            "WEATHER_ASSESSMENT_UNKNOWN: погодная оценка кандидата не подтверждена.";
                        if (!weatherAssessment.error.empty())
                            reason += " " + weatherAssessment.error;
                        violations.push_back(std::move(reason));
                    } else {
                        // Weather-corrected values and the solved airspeed profile
                        // replace nominal estimates before objective ranking.
                        rankedCandidate.estimated_time_s = weatherAssessment.corrected_time_s;
                        rankedCandidate.estimated_energy_wh = weatherAssessment.corrected_energy_wh;
                        rankedCandidate.estimated_reserve_wh = weatherAssessment.remaining_energy_wh;
                        if (weatherAssessment.corrected_route.has_value()) {
                            const auto& corrected = *weatherAssessment.corrected_route;
                            const auto& original = *candidate.trajectory_route;
                            const bool sameLineage =
                                corrected.lineage.mission_id == original.lineage.mission_id
                                && corrected.lineage.mission_version == original.lineage.mission_version
                                && corrected.lineage.route_id == original.lineage.route_id
                                && corrected.lineage.route_version == original.lineage.route_version;
                            const bool completeGeometry = corrected.trajectory_geometry_complete
                                && corrected.waypoints.size() >= 2
                                && corrected.segments.size() + 1 == corrected.waypoints.size();
                            const bool profileSnapshotMatches = corrected.speed_profile.has_value()
                                && corrected.speed_profile->profile_id.size() > 0
                                && corrected.speed_profile->profile_version.size() > 0
                                && corrected.speed_profile->wind_snapshot_id == weatherAssessment.snapshot_id
                                && corrected.speed_profile->wind_snapshot_version == weatherAssessment.snapshot_version
                                && corrected.environment.wind_snapshot_id == weatherAssessment.snapshot_id;
                            if (!sameLineage || !completeGeometry || !profileSnapshotMatches) {
                                violations.push_back(
                                    "WEATHER_ROUTE_FEEDBACK_INVALID: corrected route lost canonical lineage, complete geometry or matching wind/profile snapshot identity.");
                            } else {
                                rankedCandidate.trajectory_route = corrected;
                                if (context.problem().requires_flight_safety_validation
                                    && context.problem().flight_safety_envelope.has_value()) {
                                    const auto profileViolations = validateRouteSpeedProfile(
                                        corrected, *context.problem().flight_safety_envelope);
                                    violations.insert(violations.end(), profileViolations.begin(),
                                                      profileViolations.end());
                                }
                                if (constraint_validator_) {
                                    const auto correctedViolations =
                                        constraint_validator_(context.problem(), rankedCandidate);
                                    violations.insert(violations.end(), correctedViolations.begin(),
                                                      correctedViolations.end());
                                }
                                if (!structurally_valid_candidate(rankedCandidate, context.problem())) {
                                    violations.push_back(
                                        "WEATHER_ROUTE_FEEDBACK_INVALID: corrected route failed structural candidate validation.");
                                }
                            }
                        }
                    }
                }
            }

            if (violations.empty() && context.problem().requires_notam_validation) {
                if (!weather_assessor_) {
                    violations.push_back(
                        "NOTAM_VALIDATION_UNKNOWN: обязательная проверка после ветрового расчёта невозможна без weather assessor.");
                } else if (!rankedCandidate.trajectory_route.has_value() ||
                           !weatherAssessment.trajectory.has_value()) {
                    violations.push_back(
                        "NOTAM_VALIDATION_UNKNOWN: отсутствует канонический маршрут или фактический 4D результат ветрового расчёта.");
                } else if (!rankedCandidate.trajectory_route->constraints.avoid_restricted_areas) {
                    violations.push_back(
                        "NOTAM_VALIDATION_UNKNOWN: кандидат отключил обязательное избегание ограниченного воздушного пространства.");
                } else {
                    const auto notamResult = NotamProhibitedZoneValidator::validate(
                        *rankedCandidate.trajectory_route,
                        *context.problem().notam_snapshot,
                        *weatherAssessment.trajectory);
                    if (notamResult.status != RouteValidationStatus::Allowed) {
                        if (notamResult.findings.empty()) {
                            violations.push_back(
                                "NOTAM_VALIDATION_REJECTED: маршрут не прошёл итоговую временную проверку NOTAM.");
                        } else {
                            for (const auto& finding : notamResult.findings) {
                                std::ostringstream reason;
                                reason << "NOTAM_VALIDATION: " << finding.detail;
                                if (!finding.notam_id.empty())
                                    reason << " NOTAM: " << finding.notam_id << ".";
                                if (!finding.component_id.empty())
                                    reason << " Компонент: " << finding.component_id << ".";
                                if (!finding.waypoint_id.empty())
                                    reason << " Точка: " << finding.waypoint_id << ".";
                                if (!finding.segment_start_waypoint_id.empty())
                                    reason << " Участок: " << finding.segment_start_waypoint_id
                                           << " -> " << finding.segment_end_waypoint_id << ".";
                                violations.push_back(reason.str());
                            }
                        }
                    }
                }
            }

            // Mandatory constraints are evaluated before any objective ranking.
            if (!violations.empty()) {
                decision.rejected_candidates.push_back(
                    {candidate.candidate_id, candidate.solver_id, std::move(violations)});
                continue;
            }

            // Preserve the entire admissible set, not only the winning route.
            // With no weather assessor this is the nominal pre-weather candidate
            // set; with an assessor these entries contain corrected metrics.
            decision.admissible_candidates.push_back(rankedCandidate);

            if (!have_best || better_candidate(rankedCandidate, best, context.problem())) {
                best = rankedCandidate;
                bestWeatherAssessment = weatherAssessment;
                bestWeatherAssessmentApplied = static_cast<bool>(weather_assessor_);
                bestMaximumRegimeUses = flightSafety.maximum_regime_uses;
                bestMaximumRegimeWarning = flightSafety.maximum_regime_warning_required;
                bestOperatorWarning = flightSafety.operator_warning;
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
    decision.selected_maximum_regime_uses = std::move(bestMaximumRegimeUses);
    decision.maximum_regime_warning_required = bestMaximumRegimeWarning;
    decision.operator_warning = std::move(bestOperatorWarning);
    decision.weather_assessment_applied = bestWeatherAssessmentApplied;
    if (bestWeatherAssessmentApplied) {
        decision.weather_snapshot_id = bestWeatherAssessment.snapshot_id;
        decision.weather_snapshot_version = bestWeatherAssessment.snapshot_version;
        decision.weather_corrected_time_s = bestWeatherAssessment.corrected_time_s;
        decision.weather_corrected_energy_wh = bestWeatherAssessment.corrected_energy_wh;
        decision.weather_remaining_energy_wh = bestWeatherAssessment.remaining_energy_wh;
    }

    std::ostringstream explanation;
    explanation << "Выбран маршрут, рассчитанный алгоритмом " << best.solver_id
                << ", как лучший допустимый кандидат с учётом приоритетов текущей задачи."
                << " Обязательные ограничения проверены до ранжирования.";
    if (bestWeatherAssessmentApplied)
        explanation << " Ранжирование выполнено по времени и энергии после погодной коррекции"
                    << " (снимок " << bestWeatherAssessment.snapshot_id
                    << ", версия " << bestWeatherAssessment.snapshot_version << ").";
    decision.explanation = explanation.str();
    return decision;
}

} // namespace bluesky::planning
