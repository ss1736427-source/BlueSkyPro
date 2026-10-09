#include "coverage_decomposition.hpp"
#include <cassert>
#include <cmath>
using namespace bluesky::planning;
int main() {
    CoverageDecompositionInput i;
    i.aoi={{59.0,30.0},{59.0,30.02},{59.01,30.02},{59.01,30.0}};
    i.orientation.orientation_deg=0.0;
    i.orientation.candidate_id="MT01-ORIENT-0";
    i.track_spacing_m=500.0;
    i.minimum_altitude_m=100.0;
    i.maximum_altitude_m=100.0;
    i.environment.snapshot_id="ENV-1";
    i.environment.snapshot_version="1";
    i.environment.calculation_input_version="calc-1";
    i.geometry_revision="AOI-1";
    i.calculation_version="MT01-DECOMP-1";

    const auto r=CoverageDecompositionEngine::decompose(i);
    assert(r.valid);
    assert(r.cells.size()==3);
    assert(r.cells.front().cell_id=="MT01-CELL-0");
    assert(r.cells.front().area_m2>0.0);

    const auto repeat=CoverageDecompositionEngine::decompose(i);
    assert(repeat.dependency_identity==r.dependency_identity);
    assert(repeat.cells.size()==r.cells.size());

    auto changed=i;
    changed.track_spacing_m=250.0;
    const auto cr=CoverageDecompositionEngine::decompose(changed);
    assert(cr.valid);
    assert(cr.cells.size()==5);
    assert(cr.dependency_identity!=r.dependency_identity);

    auto invalid=i;
    invalid.track_spacing_m=0.0;
    assert(!CoverageDecompositionEngine::decompose(invalid).valid);

    auto incomplete=i;
    incomplete.environment.complete=false;
    assert(!CoverageDecompositionEngine::decompose(incomplete).valid);
    auto restricted=i;
    SpatialRestriction zone;
    zone.restriction_id="TEST-INTERNAL";
    zone.geometry_type=RestrictionGeometryType::Polygon;
    zone.polygon={{59.0030,30.0095},{59.0030,30.0120},{59.0070,30.0120},{59.0070,30.0095}};
    zone.minimum_altitude_m=50.0;
    zone.maximum_altitude_m=150.0;
    restricted.environment.restrictions={zone};

    const auto restricted_result=CoverageDecompositionEngine::decompose(restricted);
    assert(restricted_result.valid);
    std::size_t constrained=0;
    for(const auto& cell:restricted_result.cells)
        if(cell.constraint_state==CoverageCellConstraintState::Constrained) ++constrained;
    assert(constrained==1);

    CoverageTrackInput restricted_tracks_input;
    restricted_tracks_input.decomposition=restricted_result;
    restricted_tracks_input.altitude_m=100.0;
    restricted_tracks_input.footprint_width_m=100.0;
    restricted_tracks_input.footprint_height_m=100.0;
    restricted_tracks_input.calculation_version="TEST-TRACK-1";
    const auto restricted_tracks=CoverageTrackGenerator::generate(restricted_tracks_input);
    assert(restricted_tracks.valid);
    assert(restricted_tracks.tracks.size()==4);
    std::size_t restricted_cell_tracks=0;
    for(const auto& track:restricted_tracks.tracks)
        if(track.cell_id=="MT01-CELL-1") ++restricted_cell_tracks;
    assert(restricted_cell_tracks==2);

    CoverageTrackResult quality_tracks;
    quality_tracks.valid=true;
    quality_tracks.dependency_identity="TRACKS-QUALITY";
    CoverageTrack qt;
    qt.track_id="MT01-TRACK-Q0";
    qt.cell_id="MT01-CELL-Q";
    qt.start={59.0,30.0};
    qt.end={59.0,30.001};
    qt.altitude_m=100.0;
    qt.length_m=57.0;
    quality_tracks.tracks={qt,qt};

    AcquisitionGeometryResult quality_geometry;
    quality_geometry.valid=true;
    quality_geometry.dependency_identity="GEOM-QUALITY";
    quality_geometry.footprint_width_m=100.0;
    quality_geometry.footprint_height_m=100.0;
    quality_geometry.gsd_width_m_per_px=0.02;
    quality_geometry.gsd_height_m_per_px=0.02;
    quality_geometry.frontal_overlap_ratio=0.75;
    quality_geometry.side_overlap_ratio=0.60;

    AcquisitionEventResult quality_events;
    quality_events.valid=true;
    quality_events.dependency_identity="EVENTS-QUALITY";
    AcquisitionEvent qe;
    qe.event_id="MT01-EVENT-Q0";
    qe.track_id="MT01-TRACK-Q0";
    qe.position=qt.start;
    qe.gsd_width_m_per_px=0.02;
    qe.gsd_height_m_per_px=0.02;
    qe.sensor_state_valid=true;
    qe.trigger_state_valid=true;
    quality_events.events={qe};

    MappingQualityInput quality_input;
    quality_input.aoi=i.aoi;
    quality_input.decomposition=r;
    quality_input.tracks=quality_tracks;
    quality_input.events=quality_events;
    quality_input.geometry=quality_geometry;
    quality_input.calculation_version="MT01-QUALITY-1";
    const auto quality=MappingQualityEngine::evaluate(quality_input);
    assert(quality.valid);
    assert(quality.footprint_union_area_m2>0.0);
    assert(quality.footprint_union_area_m2 < 2.0 * 57.0 * 100.0);
    assert(quality.footprint_union_area_m2 > 0.0);
    assert(quality.coverage_ratio > 0.0);

    auto clipped_quality_input=quality_input;
    clipped_quality_input.aoi={
        {59.0,30.0},{59.0,30.0005},{59.0005,30.0005},{59.0005,30.0}};
    SpatialRestriction quality_restriction;
    quality_restriction.restriction_id="QUALITY-EXCLUSION";
    quality_restriction.geometry_type=RestrictionGeometryType::Polygon;
    quality_restriction.polygon={
        {59.0001,30.0001},{59.0001,30.0002},
        {59.0002,30.0002},{59.0002,30.0001}};
    clipped_quality_input.environment.complete=true;
    clipped_quality_input.environment.restrictions={quality_restriction};
    const auto clipped_quality=MappingQualityEngine::evaluate(clipped_quality_input);
    assert(clipped_quality.valid);
    assert(clipped_quality.footprint_union_area_m2>0.0);
    assert(clipped_quality.footprint_union_area_m2<=clipped_quality.aoi_area_m2);
    assert(clipped_quality.coverage_ratio<=1.0);
    assert(!clipped_quality.uncovered_geometry.empty());
    assert(clipped_quality.uncovered_area_m2>=0.0);
    assert(clipped_quality.uncovered_area_m2 < clipped_quality.aoi_area_m2);
    assert(!clipped_quality.uncovered_components.empty());
    bool has_supported_classification=false;
    for(const auto& component:clipped_quality.uncovered_components) {
        assert(component.classification==UncoveredGeometryClassification::BoundaryGap ||
               component.classification==UncoveredGeometryClassification::ExclusionInduced ||
               component.classification==UncoveredGeometryClassification::UnclassifiedSourceNotBound);
        if(component.classification==UncoveredGeometryClassification::ExclusionInduced)
            has_supported_classification=true;
    }
    assert(has_supported_classification);

    return 0;
}
