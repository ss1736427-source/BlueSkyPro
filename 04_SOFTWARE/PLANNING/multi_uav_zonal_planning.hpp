#pragma once

#include "model/route_model.hpp"
#include <string>
#include <vector>

namespace bluesky::planning {

struct MissionOperatingArea {
    std::string area_id;
    std::string area_version;
    std::vector<GeoPoint> boundary;
};

struct OperationalZone {
    std::string zone_id;
    std::string zone_version;
    std::string area_id;
    std::size_t ordinal{0};
    std::vector<GeoPoint> boundary;
};

struct UavZoneAssignment {
    std::string uav_id;
    std::string zone_id;
    std::string assignment_version;
};

struct ZonalPlanningResult {
    bool feasible{false};
    std::string calculation_version;
    std::string dependency_identity;
    std::vector<OperationalZone> zones;
    std::vector<UavZoneAssignment> assignments;
    std::vector<std::string> findings;
};

class MultiUavZonalPlanner {
public:
    static ZonalPlanningResult partition(
        const MissionOperatingArea& area,
        const std::vector<std::string>& uav_ids,
        const std::string& calculation_version);
};

} // namespace bluesky::planning
