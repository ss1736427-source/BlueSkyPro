#pragma once

#include "acquisition_geometry.hpp"
#include "model/route_model.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

struct CoverageOrientationInput {
    std::vector<GeoPoint> aoi;
    AcquisitionGeometryResult acquisition_geometry;

    // Controlled search parameters. No defaults are imposed by the engine.
    double start_angle_deg{0.0};
    double end_angle_deg{0.0};
    double angle_step_deg{0.0};

    std::string geometry_revision;
    std::string calculation_version;
};

struct CoverageOrientationCandidate {
    std::size_t generation_index{0};
    double orientation_deg{0.0};
    double projected_width_m{0.0};
    std::size_t estimated_track_count{0};
    std::string candidate_id;
};

struct CoverageOrientationResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<CoverageOrientationCandidate> candidates;
};

class CoverageOrientationGenerator final {
public:
    static CoverageOrientationResult generate(
        const CoverageOrientationInput& input);
};

} // namespace bluesky::planning
