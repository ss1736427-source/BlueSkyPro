#include "SelectedRoutePlanningService.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace bluesky::planning {

SelectedRoutePlanningService::SelectedRoutePlanningService(PlanningBridge &bridge)
    : bridge_(bridge)
{
}

bool SelectedRoutePlanningService::submitSelectedRoute(
    const MissionProblem &problem,
    const CandidateSolution &selectedCandidate,
    const PlanningGraph &graph,
    const std::string &selectionVersion,
    const std::string &dependencyIdentity,
    const std::string &routeId,
    const std::string &routeVersion,
    QJsonObject request,
    QString *error)
{
    if (!attachSelectedRouteGeometryToRequest(
            problem,
            selectedCandidate,
            graph,
            selectionVersion,
            dependencyIdentity,
            routeId,
            routeVersion,
            request,
            error)) {
        return false;
    }

    const QByteArray payload = QJsonDocument(request).toJson(QJsonDocument::Compact);
    if (!bridge_.sendRequest(QString::fromUtf8(payload))) {
        if (error && error->isEmpty())
            *error = QStringLiteral("PLANNING_REQUEST_SEND_FAILED");
        return false;
    }

    if (error)
        error->clear();
    return true;
}

} // namespace bluesky::planning
