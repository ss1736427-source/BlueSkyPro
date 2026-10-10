#pragma once

#include <cstdint>
#include <string>

namespace bluesky::operations {

enum class TelemetryFreshness {
    Current,
    Stale,
    Delayed,
    Missing,
    Derived
};

struct TelemetryProvenance {
    std::string source_id;
    std::string source_type;
    std::string adapter_version;
};

struct NormalizedTelemetryState {
    std::string vehicle_id;
    std::string snapshot_version;
    TelemetryProvenance provenance;
    TelemetryFreshness freshness{TelemetryFreshness::Missing};
    std::int64_t source_timestamp_ms{0};
    std::int64_t received_timestamp_ms{0};
    bool position_valid{false};
    bool navigation_valid{false};
    bool vehicle_state_valid{false};
    bool energy_state_valid{false};
    bool c2_state_valid{false};
};

inline bool normalized_state_usable(const NormalizedTelemetryState& state) {
    return !state.vehicle_id.empty() &&
           !state.snapshot_version.empty() &&
           !state.provenance.source_id.empty() &&
           state.freshness == TelemetryFreshness::Current &&
           state.position_valid &&
           state.navigation_valid &&
           state.vehicle_state_valid;
}

inline bool normalized_state_requires_refresh(const NormalizedTelemetryState& state) {
    return state.freshness == TelemetryFreshness::Stale ||
           state.freshness == TelemetryFreshness::Delayed ||
           state.freshness == TelemetryFreshness::Missing;
}

} // namespace bluesky::operations
