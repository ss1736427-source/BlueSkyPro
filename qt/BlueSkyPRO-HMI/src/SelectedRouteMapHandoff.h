#pragma once

#include "RouteGeometrySerializer.h"
#include "../../../04_SOFTWARE/PLANNING/canonical_route_builder.hpp"

namespace bluesky::planning {

// Joins an already selected candidate and its authoritative graph to an already
// assembled Planning Core request. It deliberately does not invent request inputs.
bool attachSelectedRouteGeometryToRequest(
    const MissionProblem &problem,
    const CandidateSolution &selectedCandidate,
    const PlanningGraph &graph,
    const std::string &selectionVersion,
    const std::string &dependencyIdentity,
    const std::string &routeId,
    const std::string &routeVersion,
    QJsonObject &request,
    QString *error = nullptr);

} // namespace bluesky::planning
