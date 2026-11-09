#include "model/vehicle_motion_model.hpp"

#include <cassert>
#include <cmath>

using namespace bluesky::planning;

int main() {
    VehicleMotionModel fixed;
    fixed.model_id = "WINGTRA-ONE-GEN2";
    fixed.model_version = "scenario-1";
    fixed.configuration_id = "reference-config";
    fixed.motion_class = MotionClass::FixedWing;
    fixed.evidence = MotionEvidence::DerivedScenario;
    fixed.reference_speed_mps = 16.0;
    fixed.minimum_speed_mps = 12.0;
    fixed.maximum_speed_mps = 20.0;
    fixed.bank_angle_rad = 20.0 * 3.14159265358979323846 / 180.0;
    fixed.speed_reference = "TAS";
    fixed.source_reference = "manufacturer cruise speed; assumed TAS; assumed bank";

    const auto fixedResult = VehicleMotionModelEvaluator::estimateTurnRadius(fixed);
    assert(fixedResult.valid);
    assert(!fixedResult.may_satisfy_motion_hard_gate);
    assert(std::abs(fixedResult.air_relative_radius_m - 71.7) < 0.2);

    VehicleMotionModel multi = fixed;
    multi.model_id = "DJI-M350-RTK";
    multi.motion_class = MotionClass::Multirotor;
    multi.evidence = MotionEvidence::DerivedScenario;
    multi.reference_speed_mps = 23.0;
    multi.minimum_speed_mps = 1.0;
    multi.maximum_speed_mps = 23.0;
    multi.lateral_acceleration_mps2 = 2.0;
    multi.speed_reference = "AIR_RELATIVE";
    multi.source_reference = "published maximum speed; assumed sustained lateral acceleration";

    const auto multiResult = VehicleMotionModelEvaluator::estimateTurnRadius(multi);
    assert(multiResult.valid);
    assert(!multiResult.may_satisfy_motion_hard_gate);
    assert(std::abs(multiResult.air_relative_radius_m - 264.5) < 0.01);

    auto validated = fixed;
    validated.evidence = MotionEvidence::FlightTestValidated;
    const auto validatedResult = VehicleMotionModelEvaluator::estimateTurnRadius(validated);
    assert(validatedResult.valid);
    assert(validatedResult.may_satisfy_motion_hard_gate);

    auto badSpeed = fixed;
    badSpeed.reference_speed_mps = 25.0;
    assert(!VehicleMotionModelEvaluator::estimateTurnRadius(badSpeed).valid);

    auto missingDatum = fixed;
    missingDatum.speed_reference.clear();
    assert(!VehicleMotionModelEvaluator::estimateTurnRadius(missingDatum).valid);

    auto badAccel = multi;
    badAccel.lateral_acceleration_mps2 = 0.0;
    assert(!VehicleMotionModelEvaluator::estimateTurnRadius(badAccel).valid);

    return 0;
}
