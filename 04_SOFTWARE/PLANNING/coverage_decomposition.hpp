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
    double footprint_width_m{0.0};
    double footprint_height_m{0.0};
    std::string calculation_version;
};

struct CoverageEdgeGap {
    std::string cell_id;
    std::string track_id;
    bool start_gap{false};
    bool end_gap{false};
    double start_margin_m{0.0};
    double end_margin_m{0.0};
};

struct CoverageEdgeResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<CoverageEdgeGap> gaps;
    std::vector<CoverageTrack> repaired_tracks;
};

class CoverageDecompositionEngine final {
public:
    static CoverageDecompositionResult decompose(const CoverageDecompositionInput& input);
};

class CoverageTrackGenerator final {
public:
    static CoverageTrackResult generate(const CoverageTrackInput& input);
};

class CoverageEdgeEngine final {
public:
    static CoverageEdgeResult evaluate(
        const CoverageTrackInput& input,
        const CoverageTrackResult& tracks);
};

} // namespace bluesky::planning
