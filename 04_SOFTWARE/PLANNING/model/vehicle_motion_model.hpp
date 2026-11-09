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
};

} // namespace bluesky::planning
