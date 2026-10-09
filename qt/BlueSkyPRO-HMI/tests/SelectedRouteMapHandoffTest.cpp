#include "../src/SelectedRouteMapHandoff.h"

#include <QJsonArray>
#include <QJsonObject>

#include <cassert>

using namespace bluesky::planning;

int main()
{
    PlanningGraph graph;
    graph.start_node = "N-START";
    graph.goal_node = "N-END";
    graph.nodes = {
        {"N-START", 0.0, 0.0, {55.75, 37.61}, 100.0},
        {"N-MID", 1.0, 1.0, {55.755, 37.615}, 115.0},
        {"N-END", 2.0, 2.0, {55.76, 37.62}, 125.0}
    };
    graph.edges = {{"N-START", "N-MID", 1.0}, {"N-MID", "N-END", 1.0}};

    MissionProblem problem;
    problem.mission_id = "MISSION-001";
    problem.mission_version = "v7";
    problem.environment_version = "ENV-12";
    problem.planning_graph = &graph;

    CandidateSolution candidate;
    candidate.candidate_id = "CANDIDATE-004";
    candidate.feasibility = Feasibility::Feasible;
    candidate.route_elements = {"N-START", "N-MID", "N-END"};

    QJsonObject request{
        {"schemaVersion", "1.0"},
        {"messageType", "planning.request"},
        {"missionId", "MISSION-001"},
        {"resultId", "RESULT-001"},
        {"inputs", QJsonObject{
            {"zoneStatus", "READY"},
            {"assignmentStatus", "READY"},
            {"routes", QJsonArray{QJsonObject{{"routeId", "existing-route"}}}},
            {"performance", QJsonArray{}},
            {"trajectories", QJsonArray{}},
            {"minimums", QJsonObject{{"horizontalM", 10}, {"verticalM", 5}}},
            {"authoritativeMarker", "preserve"}
        }}
    };

    QString error;
    assert(attachSelectedRouteGeometryToRequest(
        problem, candidate, graph, "SEL-2", "DEP-9", "ROUTE-001", "v3",
        request, &error));
    assert(error.isEmpty());
    const QJsonObject inputs = request.value("inputs").toObject();
    assert(inputs.value("authoritativeMarker").toString() == "preserve");
    const QJsonObject geometry = inputs.value("routeGeometry").toObject();
    assert(geometry.value("coordinateReference").toString() == "WGS84");
    const QJsonArray points = geometry.value("points").toArray();
    assert(points.size() == 3);
    assert(points.at(0).toObject().value("waypointId").toString() == "N-START");
    assert(points.at(1).toObject().value("waypointId").toString() == "N-MID");
    assert(points.at(2).toObject().value("waypointId").toString() == "N-END");

    QJsonObject untouched = request;
    PlanningGraph otherGraph = graph;
    problem.planning_graph = &otherGraph;
    assert(!attachSelectedRouteGeometryToRequest(
        problem, candidate, graph, "SEL-2", "DEP-9", "ROUTE-001", "v3",
        untouched, &error));
    assert(error == "PLANNING_GRAPH_IDENTITY_MISMATCH");
    assert(untouched == request);

    problem.planning_graph = &graph;
    candidate.feasibility = Feasibility::Infeasible;
    assert(!attachSelectedRouteGeometryToRequest(
        problem, candidate, graph, "SEL-2", "DEP-9", "ROUTE-001", "v3",
        untouched, &error));
    assert(error == "SELECTED_CANDIDATE_NOT_FEASIBLE");
    assert(untouched == request);
    return 0;
}
