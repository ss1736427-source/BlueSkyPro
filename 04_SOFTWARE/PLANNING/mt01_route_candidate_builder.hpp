#pragma once

#include "mt01_kinematic_geometry_builder.hpp"
#include "route_constraint_validator.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

struct Mt01RouteCandidateMetadata {
    std::string route_id;
    std::string route_version;
    std::string generator_id;
    std::string generator_version;
    std::string calculation_input_version;
    RouteConstraints constraints;
    RouteEnvironmentReference environment;
};

struct Mt01RouteCandidateResult {
    bool available{false};
    Route route;
    RouteValidationResult structural_validation;
    std::vector<std::string> diagnostics;
};

// Converts a validated MT-01 kinematic point/segment chain into the canonical
// Route data model. It deliberately leaves trajectory_geometry_complete=false:
// DEM, wind-ground-track, NOTAM/airspace, obstacle and mission release gates
// must still be completed by their own services.
class Mt01RouteCandidateBuilder {
public:
    static Mt01RouteCandidateResult build(
        const Mt01KinematicGeometryResult& geometry,
        const CoveragePlanResult& coverage,
        const std::string& mission_id,
        const std::string& mission_version,
        const Mt01RouteCandidateMetadata& metadata);
};

} // namespace bluesky::planning
