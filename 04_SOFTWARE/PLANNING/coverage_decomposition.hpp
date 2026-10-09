#pragma once

#include "constrained_open_space.hpp"
#include "coverage_orientation.hpp"
#include "model/route_model.hpp"

#include <string>
#include <vector>

namespace bluesky::planning {

enum class CoverageCellConstraintState { Open, Constrained };

struct CoveragePolygonSplitPiece {
    std::vector<GeoPoint> polygon;
    std::string restriction_id;
    std::string source_id;
};

struct CoveragePolygonSplitInput {
    std::vector<GeoPoint> subject_polygon;
    std::vector<GeoPoint> restriction_polygon;
    std::string restriction_id;
    std::string source_id;
    std::string calculation_version;
};

struct CoveragePolygonSplitResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<CoveragePolygonSplitPiece> pieces;
};

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
    ConstrainedEnvironmentSnapshot environment;
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
    std::vector<CoverageTrack> evaluated_tracks;
};

struct AcquisitionEvent {
    std::size_t generation_index{0};
    std::string event_id;
    std::string track_id;
    GeoPoint position;
    double camera_ground_distance_m{0.0};
    double gsd_width_m_per_px{0.0};
    double gsd_height_m_per_px{0.0};
    double footprint_width_m{0.0};
    double footprint_height_m{0.0};
    double trigger_interval_s{0.0};
    double frontal_overlap_ratio{0.0};
    double side_overlap_ratio{0.0};
    bool sensor_state_valid{false};
    bool trigger_state_valid{false};
};

struct AcquisitionEventInput {
    CoverageTrackResult tracks;
    AcquisitionGeometryResult geometry;
    std::string calculation_version;
};

struct AcquisitionEventResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    std::vector<AcquisitionEvent> events;
};

class CoveragePolygonSplitter final {
public:
    static CoveragePolygonSplitResult split(
        const CoveragePolygonSplitInput& input);
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

class AcquisitionEventValidator final {
public:
    static AcquisitionEventResult generate(
        const AcquisitionEventInput& input);
};

enum class UncoveredGeometryClassification {
    BoundaryGap,
    ExclusionInduced,
    UnclassifiedSourceNotBound
};

struct UncoveredGeometryComponent {
    std::vector<GeoPoint> polygon;
    double area_m2{0.0};
    UncoveredGeometryClassification classification{
        UncoveredGeometryClassification::UnclassifiedSourceNotBound};
    std::vector<std::string> source_ids;
};

struct MandatoryCoverageArea {
    std::string area_id;
    std::vector<GeoPoint> polygon;
};

struct MandatoryCoverageResult {
    std::string area_id;
    double area_m2{0.0};
    double covered_area_m2{0.0};
    double coverage_ratio{0.0};
    bool fully_covered{false};
};

struct MappingQualityInput {
    std::vector<GeoPoint> aoi;
    std::vector<MandatoryCoverageArea> mandatory_areas;
    ConstrainedEnvironmentSnapshot environment;
    CoverageDecompositionResult decomposition;
    CoverageTrackResult tracks;
    AcquisitionEventResult events;
    AcquisitionGeometryResult geometry;
    std::string calculation_version;
};

struct MappingQualityResult {
    bool valid{false};
    std::string failure_code;
    std::string dependency_identity;
    double aoi_area_m2{0.0};
    double estimated_covered_area_m2{0.0};
    double footprint_union_area_m2{0.0};
    double coverage_ratio{0.0};
    std::vector<MandatoryCoverageResult> mandatory_coverage;
    bool mandatory_coverage_passed{true};
    std::vector<std::vector<GeoPoint>> uncovered_geometry;
    std::vector<UncoveredGeometryComponent> uncovered_components;
    double uncovered_area_m2{0.0};
    double min_gsd_m_per_px{0.0};
    double max_gsd_m_per_px{0.0};
    double min_frontal_overlap_ratio{0.0};
    double min_side_overlap_ratio{0.0};
    std::size_t invalid_event_count{0};
    bool gate_passed{false};
    std::vector<std::vector<GeoPoint>> covered_geometry;
};

class MappingQualityEngine final {
public:
    static MappingQualityResult evaluate(const MappingQualityInput& input);
};

} // namespace bluesky::planning
