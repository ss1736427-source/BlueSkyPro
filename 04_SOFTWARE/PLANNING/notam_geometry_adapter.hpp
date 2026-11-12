#pragma once

#include "notam_prohibited_zone_validator.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace bluesky::planning {

struct NotamGeometryAdapterInput {
    const NotamSnapshot* snapshot{nullptr};
    std::int64_t mission_start_epoch_s{0};
    std::int64_t mission_end_epoch_s{0};
    double target_altitude_agl_m{0.0};
    double safety_buffer_m{0.0};
    std::vector<GeoPoint> aoi_boundary;
};

struct NotamGeometryAdapterResult {
    bool valid{false};
    std::string snapshot_id;
    std::size_t applicable_zone_count{0};
    std::vector<std::vector<GeoPoint>> exclusion_areas;
    std::vector<std::string> applicable_notam_ids;
    std::vector<std::string> applicable_component_ids;
    std::vector<std::string> diagnostics;
};

// Converts one immutable NOTAM snapshot into conservative 2D exclusion
// components for coverage and turn generation. It does not replace final 4D validation.
class NotamGeometryAdapter {
public:
    static NotamGeometryAdapterResult build(const NotamGeometryAdapterInput& input);
};

} // namespace bluesky::planning
