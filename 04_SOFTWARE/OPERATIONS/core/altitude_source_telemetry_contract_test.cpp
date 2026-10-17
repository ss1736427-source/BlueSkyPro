#include "altitude_source_telemetry_contract.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace bluesky::operations;

namespace {

AltitudeSensorProvenance provenance(const char* source) {
    return {source, 1000, 1000, true, true};
}

void testConfiguredDownwardRadioAltimeterAccepted() {
    RadioAltimeterInput input;
    input.height_agl_m = 37.5;
    input.sensor_id = "RA-1";
    input.configured_as_radio_altimeter = true;
    input.downward_facing = true;
    input.provenance = provenance("uav-1:ra-1");

    const auto reading = make_radio_altimeter_reading(input);
    assert(reading.valid && reading.fresh);
    assert(reading.value_m == 37.5);
    assert(reading.source_id == "uav-1:ra-1");
}

void testGenericRangefinderCannotMasqueradeAsRadioAltimeter() {
    RadioAltimeterInput input;
    input.height_agl_m = 25.0;
    input.sensor_id = "RANGE-1";
    input.configured_as_radio_altimeter = false;
    input.downward_facing = true;
    input.provenance = provenance("uav-1:range-1");

    const auto reading = make_radio_altimeter_reading(input);
    assert(!reading.valid);
}

void testRadioAltimeterRequiresDownwardOrientationAndValidHeight() {
    auto input = RadioAltimeterInput{20.0, "RA-1", true, false,
                                     provenance("uav-1:ra-1")};
    assert(!make_radio_altimeter_reading(input).valid);

    input.downward_facing = true;
    input.height_agl_m = -1.0;
    assert(!make_radio_altimeter_reading(input).valid);

    input.height_agl_m = std::numeric_limits<double>::infinity();
    assert(!make_radio_altimeter_reading(input).valid);
}

void testInvalidOrStaleProvenanceIsRejected() {
    auto input = RadioAltimeterInput{20.0, "RA-1", true, true,
                                     provenance("uav-1:ra-1")};
    input.provenance.fresh = false;
    assert(!make_radio_altimeter_reading(input).valid);

    input.provenance = provenance("uav-1:ra-1");
    input.provenance.source_timestamp_ms = 2000;
    input.provenance.received_timestamp_ms = 1000;
    assert(!make_radio_altimeter_reading(input).valid);
}

void testStandardPressureReferenceAndConversion() {
    BarometricPressureInput input;
    input.static_pressure_hpa = kStandardSeaLevelPressureHpa;
    input.sensor_id = "BARO-1";
    input.provenance = provenance("uav-1:baro-1");

    const auto reading = make_standard_pressure_altitude_reading(input);
    assert(reading.valid && reading.fresh);
    assert(std::abs(reading.value_m) < 1e-6);
    assert(reading.source_id == "uav-1:baro-1");

    input.static_pressure_hpa = 898.745; // approximately 1 km ISA pressure
    const auto one_km = make_standard_pressure_altitude_reading(input);
    assert(one_km.valid);
    assert(one_km.value_m > 900.0 && one_km.value_m < 1100.0);
}

void testInvalidPressureInputsAreRejected() {
    BarometricPressureInput input;
    input.static_pressure_hpa = 0.0;
    input.sensor_id = "BARO-1";
    input.provenance = provenance("uav-1:baro-1");
    assert(!make_standard_pressure_altitude_reading(input).valid);

    input.static_pressure_hpa = std::numeric_limits<double>::quiet_NaN();
    assert(!make_standard_pressure_altitude_reading(input).valid);

    input.static_pressure_hpa = 1013.25;
    input.provenance.valid = false;
    assert(!make_standard_pressure_altitude_reading(input).valid);
}

} // namespace

int main() {
    testConfiguredDownwardRadioAltimeterAccepted();
    testGenericRangefinderCannotMasqueradeAsRadioAltimeter();
    testRadioAltimeterRequiresDownwardOrientationAndValidHeight();
    testInvalidOrStaleProvenanceIsRejected();
    testStandardPressureReferenceAndConversion();
    testInvalidPressureInputsAreRejected();
    std::cout << "altitude_source_telemetry_contract_test: PASS\n";
    return 0;
}
