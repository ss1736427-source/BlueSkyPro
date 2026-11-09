#pragma once

#include "vehicle_motion_model.hpp"
#include "route_model.hpp"

#include <optional>
#include <string>
#include <vector>

namespace bluesky::planning {

enum class FlightSafetyEvidence {
    OfficialValidated,
    FlightTestValidated,
    DerivedScenario,
    Unknown
};

enum class FlightSafetyStatus {
    Allowed,
    Rejected,
    Unknown
};

enum class FlightSafetyParameter {
    ModelEvidence,
    MinimumSpeed,
    MaximumSpeed,
    MaximumBankAngle,
    MaximumLateralAcceleration
};

struct FlightSafetyEnvelope {
    std::string vehicle_model_id;
    std::string model_version;
    std::string configuration_id;
    std::string source_reference;
    std::string speed_reference;
    std::string minimum_speed_basis;
    FlightSafetyEvidence evidence{FlightSafetyEvidence::Unknown};
    MotionClass motion_class{MotionClass::FixedWing};

    std::optional<double> minimum_speed_mps;
    std::optional<double> maximum_speed_mps;
    std::optional<double> maximum_bank_angle_rad;
    std::optional<double> maximum_lateral_acceleration_mps2;
};

struct FlightSafetySegmentState {
    std::string segment_id;
    std::optional<double> speed_mps;
    std::optional<double> bank_angle_rad;
    std::optional<double> lateral_acceleration_mps2;
};

struct FlightSafetyFinding {
    FlightSafetyParameter parameter;
    std::string segment_id;
    std::string unit;
    double actual_value{};
    double permitted_limit{};
    std::string detail;
};

struct MaximumRegimeUse {
    FlightSafetyParameter parameter;
    std::string segment_id;
    std::string unit;
    double planned_value{};
    double permitted_limit{};
};

struct FlightSafetyValidationResult {
    FlightSafetyStatus status{FlightSafetyStatus::Unknown};
    std::string vehicle_model_id;
    std::string model_version;
    std::string configuration_id;
    std::string source_reference;
    std::vector<FlightSafetyFinding> findings;
    std::vector<MaximumRegimeUse> maximum_regime_uses;
    bool maximum_regime_warning_required{false};
    std::string operator_warning;
};

class FlightSafetyConstraintValidator {
public:
    // Every configured limit is a hard bound. Missing mandatory evidence is Unknown,
    // and Unknown is not admissible. Equality with an upper bound is allowed and
    // recorded for the operator warning; any exceedance is rejected.
    static FlightSafetyValidationResult validate(
        const FlightSafetyEnvelope& envelope,
        const std::vector<FlightSafetySegmentState>& segments,
        double equality_tolerance = 1e-6);

    // Route adapter: every route segment is checked by the same shared envelope gate.
    static FlightSafetyValidationResult validateRoute(
        const FlightSafetyEnvelope& envelope,
        const Route& route,
        double equality_tolerance = 1e-6);
};

} // namespace bluesky::planning
