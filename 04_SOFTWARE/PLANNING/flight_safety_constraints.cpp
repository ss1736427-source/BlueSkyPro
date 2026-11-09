#include "model/flight_safety_constraints.hpp"

#include <cmath>
#include <set>

namespace bluesky::planning {
namespace {

bool finitePositive(const std::optional<double>& value)
{
    return value.has_value() && std::isfinite(*value) && *value > 0.0;
}

bool validatedEvidence(FlightSafetyEvidence evidence)
{
    return evidence == FlightSafetyEvidence::OfficialValidated
        || evidence == FlightSafetyEvidence::FlightTestValidated;
}

void addFinding(FlightSafetyValidationResult& result,
                FlightSafetyParameter parameter,
                const std::string& segmentId,
                const std::string& unit,
                double actual,
                double limit,
                const std::string& detail)
{
    result.findings.push_back({parameter, segmentId, unit, actual, limit, detail});
}

void recordMaximum(FlightSafetyValidationResult& result,
                   FlightSafetyParameter parameter,
                   const std::string& segmentId,
                   const std::string& unit,
                   double actual,
                   double limit,
                   double tolerance)
{
    if (std::abs(actual - limit) <= tolerance) {
        result.maximum_regime_uses.push_back({parameter, segmentId, unit, actual, limit});
    }
}

} // namespace

FlightSafetyCandidateDynamics buildFlightSafetyCandidateDynamics(const Route& route)
{
    FlightSafetyCandidateDynamics result;
    result.expected_segment_count = route.segments.size();
    result.segments.reserve(route.segments.size());

    bool allSegmentsHaveExplicitDynamics = !route.segments.empty();
    for (const auto& segment : route.segments) {
        result.segments.push_back({
            segment.segment_id,
            segment.nominal_speed_mps,
            segment.bank_angle_rad,
            segment.lateral_acceleration_mps2
        });
        if (segment.segment_id.empty() || !std::isfinite(segment.nominal_speed_mps)
            || segment.nominal_speed_mps <= 0.0
            || (!segment.bank_angle_rad && !segment.lateral_acceleration_mps2)) {
            allSegmentsHaveExplicitDynamics = false;
        }
    }

    const bool routeStructureComplete =
        route.waypoints.size() >= 2
        && route.segments.size() + 1 == route.waypoints.size();
    result.complete = route.trajectory_geometry_complete
        && routeStructureComplete
        && allSegmentsHaveExplicitDynamics;
    return result;
}

FlightSafetyValidationResult FlightSafetyConstraintValidator::validate(
    const FlightSafetyEnvelope& envelope,
    const std::vector<FlightSafetySegmentState>& segments,
    double equalityTolerance)
{
    FlightSafetyValidationResult result;
    result.vehicle_model_id = envelope.vehicle_model_id;
    result.model_version = envelope.model_version;
    result.configuration_id = envelope.configuration_id;
    result.source_reference = envelope.source_reference;

    const auto unknown = [&](const std::string& detail) {
        result.status = FlightSafetyStatus::Unknown;
        result.findings.push_back({
            FlightSafetyParameter::ModelEvidence, {}, {}, 0.0, 0.0, detail
        });
    };

    if (envelope.vehicle_model_id.empty() || envelope.model_version.empty()
        || envelope.configuration_id.empty() || envelope.source_reference.empty()
        || envelope.speed_reference.empty()) {
        unknown("Exact vehicle/configuration identity, speed reference and provenance are required.");
        return result;
    }
    if (!validatedEvidence(envelope.evidence)) {
        unknown("Only official or flight-test validated limits may satisfy the flight-safety hard gate.");
        return result;
    }
    const std::string requiredSpeedReference =
        envelope.motion_class == MotionClass::FixedWing ? "TAS" : "AIR_RELATIVE";
    if (envelope.speed_reference != requiredSpeedReference) {
        unknown("Speed reference does not match the selected vehicle motion class.");
        return result;
    }
    if (envelope.minimum_speed_basis.empty()) {
        unknown("The basis for the minimum safe speed must be explicit and configuration-specific.");
        return result;
    }
    if (envelope.motion_class == MotionClass::FixedWing
        && envelope.minimum_speed_basis != "CONFIGURATION_SPECIFIC_STALL_SPEED_WITH_BANK_AND_MARGIN") {
        unknown("Fixed-wing minimum speed must include configuration-specific stall speed, bank/load factor and safety margin.");
        return result;
    }
    if (!finitePositive(envelope.minimum_speed_mps)
        || !finitePositive(envelope.maximum_speed_mps)
        || *envelope.minimum_speed_mps > *envelope.maximum_speed_mps
        || !std::isfinite(equalityTolerance) || equalityTolerance < 0.0) {
        unknown("A valid minimum/maximum speed envelope and non-negative equality tolerance are required.");
        return result;
    }
    if (segments.empty()) {
        unknown("At least one trajectory segment is required for dynamic envelope validation.");
        return result;
    }
    if (envelope.motion_class == MotionClass::FixedWing && !envelope.maximum_bank_angle_rad) {
        unknown("A validated maximum bank angle is mandatory for fixed-wing aircraft.");
        return result;
    }
    if (envelope.motion_class == MotionClass::Multirotor
        && !envelope.maximum_lateral_acceleration_mps2) {
        unknown("A validated maximum lateral acceleration is mandatory for multirotor aircraft.");
        return result;
    }
    if (envelope.maximum_bank_angle_rad
        && (!std::isfinite(*envelope.maximum_bank_angle_rad)
            || *envelope.maximum_bank_angle_rad <= 0.0
            || *envelope.maximum_bank_angle_rad >= 1.5707963267948966)) {
        unknown("Configured maximum bank angle must be finite and between 0 and 90 degrees.");
        return result;
    }
    if (envelope.maximum_lateral_acceleration_mps2
        && !finitePositive(envelope.maximum_lateral_acceleration_mps2)) {
        unknown("Configured maximum lateral acceleration must be finite and positive.");
        return result;
    }

    std::set<std::string> segmentIds;
    for (const auto& segment : segments) {
        if (segment.segment_id.empty() || !segmentIds.insert(segment.segment_id).second) {
            unknown("Trajectory segment identifiers must be non-empty and unique.");
            return result;
        }
        if (!finitePositive(segment.speed_mps)) {
            unknown("A finite positive speed is required for segment " + segment.segment_id + ".");
            return result;
        }
        if (*segment.speed_mps < *envelope.minimum_speed_mps - equalityTolerance) {
            addFinding(result, FlightSafetyParameter::MinimumSpeed, segment.segment_id,
                       "m/s", *segment.speed_mps, *envelope.minimum_speed_mps,
                       "Planned speed is below the validated minimum safe speed.");
        }
        if (*segment.speed_mps > *envelope.maximum_speed_mps + equalityTolerance) {
            addFinding(result, FlightSafetyParameter::MaximumSpeed, segment.segment_id,
                       "m/s", *segment.speed_mps, *envelope.maximum_speed_mps,
                       "Planned speed exceeds the validated maximum speed.");
        } else {
            recordMaximum(result, FlightSafetyParameter::MaximumSpeed, segment.segment_id,
                          "m/s", *segment.speed_mps, *envelope.maximum_speed_mps,
                          equalityTolerance);
        }

        if (envelope.motion_class == MotionClass::FixedWing) {
            if (!segment.bank_angle_rad || !std::isfinite(*segment.bank_angle_rad)
                || *segment.bank_angle_rad < 0.0) {
                unknown("A finite non-negative bank angle is required for segment "
                        + segment.segment_id + ".");
                return result;
            }
            if (*segment.bank_angle_rad > *envelope.maximum_bank_angle_rad + equalityTolerance) {
                addFinding(result, FlightSafetyParameter::MaximumBankAngle, segment.segment_id,
                           "rad", *segment.bank_angle_rad, *envelope.maximum_bank_angle_rad,
                           "Planned bank angle exceeds the validated maximum bank angle.");
            } else {
                recordMaximum(result, FlightSafetyParameter::MaximumBankAngle, segment.segment_id,
                              "rad", *segment.bank_angle_rad, *envelope.maximum_bank_angle_rad,
                              equalityTolerance);
            }
        }

        // Lateral acceleration is an independent hard bound whenever the
        // validated envelope supplies it. For multirotors it is mandatory;
        // for fixed-wing it supplements the speed/bank/load-factor checks.
        if (envelope.motion_class == MotionClass::Multirotor ||
            envelope.maximum_lateral_acceleration_mps2.has_value()) {
            if (!segment.lateral_acceleration_mps2
                || !std::isfinite(*segment.lateral_acceleration_mps2)
                || *segment.lateral_acceleration_mps2 < 0.0) {
                unknown("A finite non-negative lateral acceleration is required for segment "
                        + segment.segment_id + ".");
                return result;
            }
            if (*segment.lateral_acceleration_mps2
                > *envelope.maximum_lateral_acceleration_mps2 + equalityTolerance) {
                addFinding(result, FlightSafetyParameter::MaximumLateralAcceleration,
                           segment.segment_id, "m/s^2", *segment.lateral_acceleration_mps2,
                           *envelope.maximum_lateral_acceleration_mps2,
                           "Planned lateral acceleration exceeds the validated maximum.");
            } else {
                recordMaximum(result, FlightSafetyParameter::MaximumLateralAcceleration,
                              segment.segment_id, "m/s^2", *segment.lateral_acceleration_mps2,
                              *envelope.maximum_lateral_acceleration_mps2, equalityTolerance);
            }
        }
    }

    result.status = result.findings.empty()
        ? FlightSafetyStatus::Allowed : FlightSafetyStatus::Rejected;
    // A rejected or unknown candidate is never presented as a selected maximum-regime plan.
    if (result.status != FlightSafetyStatus::Allowed) {
        result.maximum_regime_uses.clear();
    }
    result.maximum_regime_warning_required =
        result.status == FlightSafetyStatus::Allowed && !result.maximum_regime_uses.empty();
    if (result.maximum_regime_warning_required) {
        result.operator_warning =
            "ВНИМАНИЕ: расчёт выполнен с использованием максимально допустимых режимов БВС.";
    }
    return result;
}

FlightSafetyValidationResult FlightSafetyConstraintValidator::validateRoute(
    const FlightSafetyEnvelope& envelope,
    const Route& route,
    double equalityTolerance)
{
    std::vector<FlightSafetySegmentState> states;
    states.reserve(route.segments.size());
    for (const auto& segment : route.segments) {
        states.push_back({
            segment.segment_id,
            segment.nominal_speed_mps,
            segment.bank_angle_rad,
            segment.lateral_acceleration_mps2
        });
    }
    return validate(envelope, states, equalityTolerance);
}

} // namespace bluesky::planning
