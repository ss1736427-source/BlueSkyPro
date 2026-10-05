#pragma once
#include "model/route_model.hpp"
#include <string>
#include <vector>
namespace bluesky::planning {
enum class RouteValidationStatus { Allowed, Rejected };
enum class RouteFindingCode { MissingWaypointReference, SegmentEndpointMismatch, AltitudeBelowMinimum, AltitudeAboveMaximum };
struct RouteValidationFinding { RouteFindingCode code; std::string object_id; std::string detail; };
struct RouteValidationResult {
 RouteValidationStatus status{RouteValidationStatus::Allowed};
 std::vector<RouteValidationFinding> findings;
 std::string validator_id;
 std::string validator_version;
 std::string calculation_input_version;
};
class RouteConstraintValidator { public: static RouteValidationResult validate(const Route& route); };
}