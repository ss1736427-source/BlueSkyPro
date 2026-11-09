#include "model/flight_safety_constraints.hpp"

#include <cassert>
#include <cmath>

using namespace bluesky::planning;

static FlightSafetyEnvelope envelope()
{
    FlightSafetyEnvelope e;
    e.vehicle_model_id = "TEST-UAV";
    e.model_version = "2";
    e.configuration_id = "CFG-01";
    e.source_reference = "validated-flight-test-report";
    e.speed_reference = "TAS";
    e.evidence = FlightSafetyEvidence::FlightTestValidated;
    e.motion_class = MotionClass::FixedWing;
    e.minimum_speed_mps = 10.0;
    e.maximum_speed_mps = 25.0;
    e.maximum_bank_angle_rad = 0.7853981633974483;
    e.maximum_lateral_acceleration_mps2 = 5.0;
    return e;
}

static FlightSafetySegmentState segment(
    double speed, double bank, double acceleration)
{
    return {"TURN-01", speed, bank, acceleration};
}

int main()
{
    auto e = envelope();

    // Interior operating point: allowed, no maximum-regime warning.
    auto ordinary = FlightSafetyConstraintValidator::validate(
        e, {segment(18.0, 0.35, 2.0)});
    assert(ordinary.status == FlightSafetyStatus::Allowed);
    assert(!ordinary.maximum_regime_warning_required);
    assert(ordinary.maximum_regime_uses.empty());

    // Exact permitted maxima are allowed, but all used maxima are reported.
    auto atMaximum = FlightSafetyConstraintValidator::validate(
        e, {segment(25.0, *e.maximum_bank_angle_rad, 5.0)});
    assert(atMaximum.status == FlightSafetyStatus::Allowed);
    assert(atMaximum.maximum_regime_warning_required);
    assert(atMaximum.maximum_regime_uses.size() == 3);
    assert(atMaximum.operator_warning.find("максимально допустимыми режимами") != std::string::npos);

    // Report only the parameter that actually reaches its maximum.
    auto oneMaximum = FlightSafetyConstraintValidator::validate(
        e, {segment(25.0, 0.4, 2.0)});
    assert(oneMaximum.status == FlightSafetyStatus::Allowed);
    assert(oneMaximum.maximum_regime_uses.size() == 1);
    assert(oneMaximum.maximum_regime_uses[0].parameter == FlightSafetyParameter::MaximumSpeed);

    // Approaching a maximum is not a maximum-regime warning.
    auto nearMaximum = FlightSafetyConstraintValidator::validate(
        e, {segment(24.9, 0.78, 4.9)});
    assert(nearMaximum.status == FlightSafetyStatus::Allowed);
    assert(!nearMaximum.maximum_regime_warning_required);

    // Exceeding any maximum is rejected; never clipped or silently accepted.
    auto overspeed = FlightSafetyConstraintValidator::validate(
        e, {segment(25.01, 0.3, 1.0)});
    assert(overspeed.status == FlightSafetyStatus::Rejected);
    assert(!overspeed.maximum_regime_warning_required);
    assert(!overspeed.findings.empty());

    auto overbank = FlightSafetyConstraintValidator::validate(
        e, {segment(20.0, 0.79, 1.0)});
    assert(overbank.status == FlightSafetyStatus::Rejected);

    auto overAcceleration = FlightSafetyConstraintValidator::validate(
        e, {segment(20.0, 0.3, 5.1)});
    assert(overAcceleration.status == FlightSafetyStatus::Rejected);

    // Minimum safe speed is a hard lower bound, not a maximum-regime warning.
    auto underspeed = FlightSafetyConstraintValidator::validate(
        e, {segment(9.9, 0.3, 1.0)});
    assert(underspeed.status == FlightSafetyStatus::Rejected);
    assert(!underspeed.maximum_regime_warning_required);

    // A rejected candidate at a maximum does not emit the selected-plan warning.\n    auto maxAndOver = FlightSafetyConstraintValidator::validate(\n        e, {segment(26.0, *e.maximum_bank_angle_rad, 5.0)});\n    assert(maxAndOver.status == FlightSafetyStatus::Rejected);\n    assert(!maxAndOver.maximum_regime_warning_required);\n    assert(maxAndOver.maximum_regime_uses.empty());\n\n    // Missing or advisory-only evidence is Unknown and cannot pass the hard gate.
    auto missingSource = e;
    missingSource.source_reference.clear();
    assert(FlightSafetyConstraintValidator::validate(
        missingSource, {segment(18.0, 0.3, 1.0)}).status == FlightSafetyStatus::Unknown);

    auto missingFixedWingBankLimit = e;\n    missingFixedWingBankLimit.maximum_bank_angle_rad.reset();\n    assert(FlightSafetyConstraintValidator::validate(\n        missingFixedWingBankLimit, {segment(18.0, 0.3, 1.0)}).status == FlightSafetyStatus::Unknown);\n\n    auto multi = e;\n    multi.motion_class = MotionClass::Multirotor;\n    multi.maximum_bank_angle_rad.reset();\n    multi.maximum_lateral_acceleration_mps2 = 4.0;\n    auto multiState = segment(18.0, 0.3, 4.0);\n    auto multiResult = FlightSafetyConstraintValidator::validate(multi, {multiState});\n    assert(multiResult.status == FlightSafetyStatus::Allowed);\n    assert(multiResult.maximum_regime_warning_required);\n    assert(multiResult.maximum_regime_uses.size() == 1);\n    assert(multiResult.maximum_regime_uses[0].parameter == FlightSafetyParameter::MaximumLateralAcceleration);\n\n    auto advisory = e;
    advisory.evidence = FlightSafetyEvidence::DerivedScenario;
    assert(FlightSafetyConstraintValidator::validate(
        advisory, {segment(18.0, 0.3, 1.0)}).status == FlightSafetyStatus::Unknown);

    auto missingBank = segment(18.0, 0.3, 1.0);
    missingBank.bank_angle_rad.reset();
    assert(FlightSafetyConstraintValidator::validate(
        e, {missingBank}).status == FlightSafetyStatus::Unknown);

    auto duplicateSegment = FlightSafetyConstraintValidator::validate(
        e, {segment(18.0, 0.3, 1.0), segment(18.0, 0.3, 1.0)});
    assert(duplicateSegment.status == FlightSafetyStatus::Unknown);

    return 0;
}
