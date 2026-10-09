#pragma once

#include "constrained_open_space.hpp"
#include "coverage_orientation.hpp"
#include "model/route_model.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

enum class CoverageCellConstraintState { Open, Constrained };

struct CoverageDecompositionInput {
    std::vector<GeoPoint> aoi;
    CoverageOrientationCandidate orientation;
    double track_spacing_m{0.0};
    double minimum_altitude_m{0.0};
    double maximum_altitude_m{0.0};
    ConstrainedEnvironmentSnapshot environment;
    std::string geometry_revision;
    std::string calculation_version;
};

struct CoveragePlanningCell {
    std::size_t generation_index{0};
    std::string cell_id;
    double orientation_deg{0.0};
    CoverageCellConstraintState constraint_state{CoverageCellConstraintState::Open};
    std::vector<GeoPoint> polygon;
    double area_m2{0.0};
};

struct CoverageDecompositionResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<CoveragePlanningCell> cells;
};

struct CoverageTrack {
    std::size_t generation_index{0};
    std::string track_id;
    std::string cell_id;
    GeoPoint start;
    GeoPoint end;
    double altitude_m{0.0};
    double length_m{0.0};
};

struct CoverageTrackResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<CoverageTrack> tracks;
};

struct CoverageTrackInput {
    CoverageDecompositionResult decomposition;
    double altitude_m{0.0};
    std::string calculation_version;
};

class CoverageDecompositionEngine final {
public:
    static CoverageDecompositionResult decompose(const CoverageDecompositionInput& input);
};

class CoverageTrackGenerator final {
public:
    static CoverageTrackResult generate(const CoverageTrackInput& input);
};

} // namespace bluesky::planning
