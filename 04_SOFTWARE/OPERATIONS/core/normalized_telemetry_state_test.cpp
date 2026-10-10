#include "normalized_telemetry_state.hpp"

#include <cassert>

using namespace bluesky::operations;

int main() {
    NormalizedTelemetryState state;
    assert(!normalized_state_usable(state));
    assert(normalized_state_requires_refresh(state));

    state.vehicle_id = "uav-001";
    state.snapshot_version = "snapshot-1";
    state.provenance.source_id = "adapter-001";
    state.provenance.source_type = "autopilot";
    state.provenance.adapter_version = "1.0";
    state.freshness = TelemetryFreshness::Current;
    state.position_valid = true;
    state.navigation_valid = true;
    state.vehicle_state_valid = true;
    assert(normalized_state_usable(state));
    assert(!normalized_state_requires_refresh(state));

    state.freshness = TelemetryFreshness::Stale;
    assert(!normalized_state_usable(state));
    assert(normalized_state_requires_refresh(state));

    state.freshness = TelemetryFreshness::Derived;
    assert(!normalized_state_usable(state));
    assert(!normalized_state_requires_refresh(state));

    state.freshness = TelemetryFreshness::Current;
    state.provenance.source_id.clear();
    assert(!normalized_state_usable(state));

    return 0;
}
