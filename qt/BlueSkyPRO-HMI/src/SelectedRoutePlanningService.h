#pragma once

#include <QString>

#include <string>

#include "PlanningBridge.h"
#include "SelectedRouteMapHandoff.h"

namespace bluesky::planning {

// Sends route geometry only after resolving the selected feasible candidate
// against its authoritative graph and attaching it to an already valid request.
// The caller remains responsible for supplying the complete authoritative
// Planning Core request and invoking this service only when inputs change.
class SelectedRoutePlanningService final
{
public:
    explicit SelectedRoutePlanningService(PlanningBridge &bridge);

    bool submitSelectedRoute(
        const MissionProblem &problem,
        const CandidateSolution &selectedCandidate,
        const PlanningGraph &graph,
        const std::string &selectionVersion,
        const std::string &dependencyIdentity,
        const std::string &routeId,
        const std::string &routeVersion,
        QJsonObject request,
        QString *error = nullptr);

private:
    PlanningBridge &bridge_;
};

} // namespace bluesky::planning
