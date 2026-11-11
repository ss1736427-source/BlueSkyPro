#include "dubins_turn_connector.hpp"

#include <cassert>
#include <cmath>

using namespace bluesky::planning;

int main() {
    TurnConnectorInput straight;
    straight.start={0.0,0.0,0.0};
    straight.finish={100.0,0.0,0.0};
    straight.minimum_turn_radius_m=20.0;
    straight.maximum_sample_spacing_m=2.0;
    const auto direct=DubinsTurnConnector::build(straight);
    assert(direct.feasible);
    assert(direct.family==TurnPathFamily::LSL || direct.family==TurnPathFamily::RSR);
    assert(std::abs(direct.path_length_m-100.0)<1e-5);
    assert(direct.points.size()>2);
    assert(std::abs(direct.points.back().x_m-100.0)<1e-9);
    assert(std::abs(direct.points.back().y_m)<1e-9);

    auto blocked=straight;
    TurnPolygon2D obstacle;
    obstacle.vertices={{40.0,-5.0,0.0},{60.0,-5.0,0.0},{60.0,5.0,0.0},{40.0,5.0,0.0}};
    blocked.prohibited_areas.push_back(obstacle);
    const auto rejected=DubinsTurnConnector::build(blocked);
    assert(!rejected.feasible);
    assert(rejected.diagnostic.find("TURN_CONNECTOR_BLOCKED")!=std::string::npos);

    auto invalid=straight;
    invalid.minimum_turn_radius_m=0.0;
    assert(!DubinsTurnConnector::build(invalid).feasible);

    auto malformed=straight;
    TurnPolygon2D bad;
    bad.vertices={{0.0,0.0,0.0},{1.0,1.0,0.0}};
    malformed.prohibited_areas.push_back(bad);
    const auto malformedResult=DubinsTurnConnector::build(malformed);
    assert(!malformedResult.feasible);
    assert(malformedResult.diagnostic.find("TURN_EXCLUSION_GEOMETRY_INVALID")!=std::string::npos);
    return 0;
}
