#pragma once

#include <string>
#include <vector>

namespace bluesky::planning {

// Metric local-coordinate pose. Heading is radians, clockwise from +X is NOT used:
// positive angles follow the usual mathematical counter-clockwise convention.
struct TurnPose2D {
    double x_m{0.0};
    double y_m{0.0};
    double heading_rad{0.0};
};

struct TurnPolygon2D {
    std::vector<TurnPose2D> vertices;
};

enum class TurnPathFamily { LSL, RSR, LSR, RSL };

struct TurnConnectorInput {
    TurnPose2D start;
    TurnPose2D finish;
    double minimum_turn_radius_m{0.0};
    double maximum_sample_spacing_m{2.0};
    std::vector<TurnPolygon2D> prohibited_areas;
};

struct TurnConnectorResult {
    bool feasible{false};
    TurnPathFamily family{TurnPathFamily::LSL};
    double minimum_turn_radius_m{0.0};
    double path_length_m{0.0};
    std::vector<TurnPose2D> points;
    std::string diagnostic;
};

// Conservative fixed-wing connector. It evaluates four Dubins path families
// (LSL/RSR/LSR/RSL), samples each path, and rejects any sampled segment that
// intersects a prohibited polygon. It never relaxes the requested turn radius.
// CCC families are intentionally not synthesized; absence of a returned path
// means this bounded solver could not establish a safe connector.
class DubinsTurnConnector {
public:
    static TurnConnectorResult build(const TurnConnectorInput& input);
};

} // namespace bluesky::planning
