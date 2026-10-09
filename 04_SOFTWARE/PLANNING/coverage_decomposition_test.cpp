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
    assert(r.cells.size()==5);
    assert(r.cells.front().cell_id=="MT01-CELL-0");
    assert(r.cells.front().area_m2>0.0);

    const auto repeat=CoverageDecompositionEngine::decompose(i);
    assert(repeat.dependency_identity==r.dependency_identity);
    assert(repeat.cells.size()==r.cells.size());

    auto changed=i;
    changed.track_spacing_m=250.0;
    const auto cr=CoverageDecompositionEngine::decompose(changed);
    assert(cr.valid);
    assert(cr.cells.size()==10);
    assert(cr.dependency_identity!=r.dependency_identity);

    auto invalid=i;
    invalid.track_spacing_m=0.0;
    assert(!CoverageDecompositionEngine::decompose(invalid).valid);

    auto incomplete=i;
    incomplete.environment.complete=false;
    assert(!CoverageDecompositionEngine::decompose(incomplete).valid);
    return 0;
}
