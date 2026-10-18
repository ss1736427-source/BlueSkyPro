#include "mavlink_altitude_display_adapter.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <iostream>

using namespace bluesky::operations;

namespace {

MavlinkTelemetrySample sampleAt(std::int64_t received_ms,
                                double pressure_hpa,
                                double radio_height_m) {
    MavlinkTelemetrySample sample;
    sample.vehicle_id = "UAV-ALT-01";
    sample.source_id = "PX4:MAVLink2:1:1";
    sample.timestamp_is_receive_time_fallback = true;

    sample.barometric_pressure = MavlinkBarometricPressureSample{
        5000, received_ms, pressure_hpa, true};

    MavlinkDistanceSensorSample distance;
    distance.source_time_boot_ms = 5000;
    distance.received_timestamp_ms = received_ms;
    distance.minimum_distance_cm = 30;
    distance.maximum_distance_cm = 15000;
    distance.sensor_type = 3;
    distance.sensor_id = 4;
    distance.orientation = kMavSensorOrientationPitch270;
    distance.distance_m = radio_height_m;
    distance.valid = true;
    sample.distance_sensor = distance;
    return sample;
}

MavlinkAltitudeDisplaySensorConfig config() {
    return {true, 4, 3, 1000, 500};
}

void testBelowAndAt100mSelectRadioAltimeter() {
    auto sample = sampleAt(9900, 900.0, 99.9);
    auto result = select_mavlink_altitude_display(sample, config(), 10000);
    assert(result.decision.source == AltitudeDisplaySource::RadioAltimeterAgl);
    assert(result.decision.value_m == 99.9);
    assert(result.radio_altimeter_input_usable);
    assert(result.timestamp_quality_degraded);

    sample.distance_sensor->distance_m = 100.0;
    result = select_mavlink_altitude_display(sample, config(), 10000);
    assert(result.decision.source == AltitudeDisplaySource::RadioAltimeterAgl);
    assert(result.decision.value_m == 100.0);
}

void testAbove100mSelectsStandardPressureAltitude() {
    const auto sample = sampleAt(9900, 900.0, 100.1);
    const auto result = select_mavlink_altitude_display(sample, config(), 10000);
    assert(result.decision.source ==
           AltitudeDisplaySource::BarometricStandardPressure);
    assert(result.barometric_input_usable);
    assert(result.decision.value_m > 900.0);

    // A radio-altimeter reading outside the sensor's declared range is not
    // accepted even when the upstream decoded sample is incorrectly marked valid.
    auto outOfRange = sample;
    outOfRange.distance_sensor->maximum_distance_cm = 10000;
    const auto degraded = select_mavlink_altitude_display(outOfRange, config(), 10000);
    assert(!degraded.radio_altimeter_input_usable);
    assert(degraded.decision.source ==
           AltitudeDisplaySource::BarometricStandardPressureDegraded);
    assert(degraded.decision.degraded);
}

void testUnconfiguredOrMismatchedSensorIsNotRadioAltimeter() {
    const auto sample = sampleAt(9900, 900.0, 20.0);
    auto cfg = config();
    cfg.radio_altimeter_configured = false;
    auto result = select_mavlink_altitude_display(sample, cfg, 10000);
    assert(!result.radio_altimeter_input_usable);
    assert(result.decision.source ==
           AltitudeDisplaySource::BarometricStandardPressureDegraded);

    cfg = config();
    cfg.radio_altimeter_sensor_id = 99;
    result = select_mavlink_altitude_display(sample, cfg, 10000);
    assert(!result.radio_altimeter_input_usable);
}

void testStaleAndIncoherentInputsAreNotSilentlyAccepted() {
    auto sample = sampleAt(8000, 900.0, 120.0);
    auto result = select_mavlink_altitude_display(sample, config(), 10000);
    assert(!result.radio_altimeter_input_usable);
    assert(!result.barometric_input_usable);
    assert(result.decision.source == AltitudeDisplaySource::Unavailable);

    sample = sampleAt(9900, 900.0, 120.0);
    sample.barometric_pressure->received_timestamp_ms = 8000;
    result = select_mavlink_altitude_display(sample, config(), 10000);
    assert(result.radio_altimeter_input_usable);
    assert(!result.barometric_input_usable);
    assert(result.decision.source == AltitudeDisplaySource::Unavailable);
}

void testBadOrientationOrInvalidDistanceRejected() {
    auto sample = sampleAt(9900, 900.0, 50.0);
    sample.distance_sensor->orientation = 0;
    auto result = select_mavlink_altitude_display(sample, config(), 10000);
    assert(!result.radio_altimeter_input_usable);

    sample = sampleAt(9900, 900.0, 50.0);
    sample.distance_sensor->valid = false;
    result = select_mavlink_altitude_display(sample, config(), 10000);
    assert(!result.radio_altimeter_input_usable);
}

} // namespace

int main() {
    testBelowAndAt100mSelectRadioAltimeter();
    testAbove100mSelectsStandardPressureAltitude();
    testUnconfiguredOrMismatchedSensorIsNotRadioAltimeter();
    testStaleAndIncoherentInputsAreNotSilentlyAccepted();
    testBadOrientationOrInvalidDistanceRejected();
    std::cout << "mavlink_altitude_display_adapter_test: PASS\n";
    return 0;
}
