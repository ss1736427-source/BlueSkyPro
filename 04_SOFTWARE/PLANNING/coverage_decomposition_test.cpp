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
    assert(restricted_tracks.tracks.size()==2);
    for(const auto& track:restricted_tracks.tracks)
        assert(track.cell_id!="MT01-CELL-1");

    return 0;
}
