#include "SelectedRouteMapHandoff.h"

#include "../../../04_SOFTWARE/PLANNING/selected_route_set.hpp"

namespace bluesky::planning {

bool attachSelectedRouteGeometryToRequest(
    const MissionProblem &problem,
    const CandidateSolution &selectedCandidate,
    const PlanningGraph &graph,
    const std::string &selectionVersion,
    const std::string &dependencyIdentity,
    const std::string &routeId,
    const std::string &routeVersion,
    QJsonObject &request,
    QString *error)
{
    const auto fail = [error](const QString &code) {
        if (error)
            *error = code;
        return false;
    };

    // The candidate must be resolved against the exact graph used by planning.
    if (problem.planning_graph != &graph)
        return fail(QStringLiteral("PLANNING_GRAPH_IDENTITY_MISMATCH"));
    if (selectedCandidate.feasibility != Feasibility::Feasible)
        return fail(QStringLiteral("SELECTED_CANDIDATE_NOT_FEASIBLE"));
    if (selectedCandidate.candidate_id.empty())
        return fail(QStringLiteral("SELECTED_CANDIDATE_ID_REQUIRED"));

    const SelectedRouteSet selected = SelectedRouteSetBuilder::build(
        problem, selectedCandidate, selectionVersion, dependencyIdentity);
    const CanonicalRouteBuildResult built = CanonicalRouteBuilder::build(
        selected, graph, routeId, routeVersion);
    if (!built.valid)
        return fail(QString::fromStdString(built.error));

    return attachRouteGeometryToRequest(built.route, request, error);
}

} // namespace bluesky::planning
