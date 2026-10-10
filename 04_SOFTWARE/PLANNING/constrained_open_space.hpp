#pragma once
#include "model/route_model.hpp"
#include <string>
#include <vector>
#include <limits>
namespace bluesky::planning {
enum class RestrictionGeometryType { Polygon, Circle };
struct SpatialRestriction {
 std::string restriction_id; std::string source_id; std::string snapshot_version;
 RestrictionGeometryType geometry_type{RestrictionGeometryType::Polygon};
 std::vector<GeoPoint> polygon; GeoPoint center; double radius_m{0.0};
 double minimum_altitude_m{0.0}; double maximum_altitude_m{0.0}; bool active{true};
};
struct ConstrainedEnvironmentSnapshot {
 std::string snapshot_id; std::string snapshot_version; std::string calculation_input_version;
 std::vector<SpatialRestriction> restrictions; bool complete{true};
};
struct SpatialEdge { GeoPoint from; GeoPoint to; double altitude_m{0.0}; double altitude_min_m{std::numeric_limits<double>::quiet_NaN()}; double altitude_max_m{std::numeric_limits<double>::quiet_NaN()}; };
struct OpenSpaceResult {
 bool allowed{true}; std::string snapshot_id; std::string snapshot_version;
 std::string calculation_input_version; std::string dependency_identity;
 std::vector<std::string> blocking_restriction_ids;
};
class ConstrainedOpenSpace {
public:
 static OpenSpaceResult evaluateSegment(const ConstrainedEnvironmentSnapshot&, const SpatialEdge&);
 static OpenSpaceResult evaluateRoute(const ConstrainedEnvironmentSnapshot&, const Route&);
};
} // namespace bluesky::planning
