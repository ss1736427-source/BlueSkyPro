#pragma once

#include <cmath>
#include <string>

namespace bluesky::planning {

enum class MotionClass { FixedWing, Multirotor };
enum class MotionEvidence { OfficialValidated, FlightTestValidated, DerivedScenario, Unknown };

struct VehicleMotionModel {
    std::string model_id;
    std::string model_version;
    std::string configuration_id;
    MotionClass motion_class{MotionClass::FixedWing};
    MotionEvidence evidence{MotionEvidence::Unknown};
    // Air-relative speed in m/s. Must be the speed reference required by the selected model.
    double reference_speed_mps{0.0};
    double minimum_speed_mps{0.0};
    double maximum_speed_mps{0.0};
    // Fixed-wing coordinated level-turn input, radians.
    double bank_angle_rad{0.0};
    // Multirotor sustained lateral acceleration input, m/s^2.
    double lateral_acceleration_mps2{0.0};
    std::string speed_reference; // e.g. TAS; never silently assume IAS/TAS/ground speed equivalence.
    std::string source_reference;
};

struct TurnRadiusEstimate {
    bool valid{false};
    bool may_satisfy_motion_hard_gate{false};
    double air_relative_radius_m{0.0};
    double reference_speed_mps{0.0};
    MotionEvidence evidence{MotionEvidence::Unknown};
    std::string model_id;
    std::string model_version;
    std::string diagnostic;
};


struct TurnSpeedRecommendation {
    bool valid{false};
    bool may_satisfy_motion_hard_gate{false};
    double requested_radius_m{0.0};
    double selected_speed_mps{0.0};
    double minimum_safe_turn_speed_mps{0.0};
    double stall_speed_at_bank_mps{0.0};
    double achieved_ideal_radius_m{0.0};
    bool radius_requirement_met{false};
    MotionEvidence evidence{MotionEvidence::Unknown};
    std::string diagnostic;
};

class VehicleMotionModelEvaluator {
public:
    // Returns an air-relative idealized radius. Wind correction and trajectory realization
    // are separate operations. DerivedScenario results are advisory only.
    static TurnRadiusEstimate estimateTurnRadius(const VehicleMotionModel& model) {
        TurnRadiusEstimate out;
        out.evidence = model.evidence;
        out.model_id = model.model_id;
        out.model_version = model.model_version;
        out.reference_speed_mps = model.reference_speed_mps;

        const auto finitePositive = [](double v) { return std::isfinite(v) && v > 0.0; };
        if (model.model_id.empty() || model.model_version.empty() ||
            model.configuration_id.empty()) {
            out.diagnostic = "Model identity, version and exact configuration are required.";
            return out;
        }
        if (!finitePositive(model.reference_speed_mps) ||
            !finitePositive(model.minimum_speed_mps) ||
            !finitePositive(model.maximum_speed_mps) ||
            model.minimum_speed_mps > model.maximum_speed_mps ||
            model.reference_speed_mps < model.minimum_speed_mps ||
            model.reference_speed_mps > model.maximum_speed_mps) {
            out.diagnostic = "Reference speed must be finite and inside the configured speed envelope.";
            return out;
        }
        if (model.speed_reference.empty() || model.source_reference.empty()) {
            out.diagnostic = "Speed reference and source provenance are mandatory.";
            return out;
        }

        constexpr double gravity = 9.80665;
        constexpr double pi = 3.14159265358979323846;
        double radius = 0.0;
        switch (model.motion_class) {
        case MotionClass::FixedWing:
            if (model.speed_reference != "TAS") {
                out.diagnostic = "Fixed-wing coordinated-turn calculation requires explicit TAS.";
                return out;
            }
            if (!std::isfinite(model.bank_angle_rad) ||
                model.bank_angle_rad <= 0.0 || model.bank_angle_rad >= (pi / 2.0)) {
                out.diagnostic = "Fixed-wing bank angle must be between 0 and 90 degrees.";
                return out;
            }
            radius = (model.reference_speed_mps * model.reference_speed_mps) /
                     (gravity * std::tan(model.bank_angle_rad));
            break;
        case MotionClass::Multirotor:
            if (model.speed_reference != "AIR_RELATIVE") {
                out.diagnostic = "Multirotor calculation requires explicit air-relative speed.";
                return out;
            }
            if (!finitePositive(model.lateral_acceleration_mps2)) {
                out.diagnostic = "Validated positive sustained lateral acceleration is required.";
                return out;
            }
            radius = (model.reference_speed_mps * model.reference_speed_mps) /
                     model.lateral_acceleration_mps2;
            break;
        }

        if (!finitePositive(radius)) {
            out.diagnostic = "Calculated turn radius is not finite and positive.";
            return out;
        }
        out.valid = true;
        out.air_relative_radius_m = radius;
        out.may_satisfy_motion_hard_gate =
            model.evidence == MotionEvidence::OfficialValidated ||
            model.evidence == MotionEvidence::FlightTestValidated;
        out.diagnostic = out.may_satisfy_motion_hard_gate
            ? "Air-relative estimate calculated from validated configuration inputs; independent trajectory validation remains required."
            : "Advisory scenario only: unvalidated assumptions cannot satisfy the vehicle-motion hard gate.";
        return out;
    }

    // Selects the slowest admissible fixed-wing speed that can meet the requested
    // air-relative radius at the configured bank angle. Stall speed is increased
    // for bank load factor and then multiplied by an explicit safety margin.
    // This is a planning estimate, not an in-flight speed command or stall protection.
    static TurnSpeedRecommendation recommendMinimumTurnSpeed(
        const VehicleMotionModel& model,
        double requested_radius_m,
        double stall_speed_clean_mps,
        double stall_margin_factor) {
        TurnSpeedRecommendation out;
        out.requested_radius_m = requested_radius_m;
        out.evidence = model.evidence;

        constexpr double gravity = 9.80665;
        constexpr double pi = 3.14159265358979323846;
        const auto finitePositive = [](double v) { return std::isfinite(v) && v > 0.0; };

        if (model.motion_class != MotionClass::FixedWing) {
            out.diagnostic = "Adaptive stall-limited turn-speed selection is only defined for fixed-wing aircraft.";
            return out;
        }
        if (model.model_id.empty() || model.model_version.empty() ||
            model.configuration_id.empty() || model.speed_reference != "TAS" ||
            model.source_reference.empty()) {
            out.diagnostic = "Exact aircraft/configuration identity, TAS reference and provenance are required.";
            return out;
        }
        if (!finitePositive(requested_radius_m) ||
            !finitePositive(stall_speed_clean_mps) ||
            !finitePositive(stall_margin_factor) || stall_margin_factor <= 1.0 ||
            !finitePositive(model.minimum_speed_mps) ||
            !finitePositive(model.maximum_speed_mps) ||
            model.minimum_speed_mps > model.maximum_speed_mps ||
            !std::isfinite(model.bank_angle_rad) ||
            model.bank_angle_rad <= 0.0 || model.bank_angle_rad >= pi / 2.0) {
            out.diagnostic = "Invalid radius, stall data, safety margin, speed envelope or bank angle.";
            return out;
        }

        const double cosBank = std::cos(model.bank_angle_rad);
        if (!finitePositive(cosBank)) {
            out.diagnostic = "Bank angle produces an invalid load-factor calculation.";
            return out;
        }
        out.stall_speed_at_bank_mps = stall_speed_clean_mps / std::sqrt(cosBank);
        out.minimum_safe_turn_speed_mps = std::max(
            model.minimum_speed_mps,
            out.stall_speed_at_bank_mps * stall_margin_factor);

        if (out.minimum_safe_turn_speed_mps > model.maximum_speed_mps) {
            out.diagnostic = "No safe turn speed: stall-margin speed exceeds the validated aircraft speed envelope.";
            return out;
        }

        // For a coordinated level turn, radius decreases monotonically with TAS.
        // Therefore the lowest admissible TAS gives the smallest achievable radius.
        out.selected_speed_mps = out.minimum_safe_turn_speed_mps;
        out.achieved_ideal_radius_m =
            (out.selected_speed_mps * out.selected_speed_mps) /
            (gravity * std::tan(model.bank_angle_rad));
        out.radius_requirement_met =
            out.achieved_ideal_radius_m <= requested_radius_m * (1.0 + 1e-9);
        if (!out.radius_requirement_met) {
            out.diagnostic = "Requested radius cannot be achieved at or above the minimum safe turn speed.";
            return out;
        }
        out.valid = true;
        out.may_satisfy_motion_hard_gate =
            model.evidence == MotionEvidence::OfficialValidated ||
            model.evidence == MotionEvidence::FlightTestValidated;
        out.diagnostic = out.may_satisfy_motion_hard_gate
            ? "Minimum admissible TAS selected for the requested radius; wind, trajectory, energy and independent safety validation remain required."
            : "Advisory scenario only: unvalidated inputs cannot satisfy the vehicle-motion hard gate.";
        return out;
    }

};

} // namespace bluesky::planning
