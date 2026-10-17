#include "mavlink_altitude_display_runtime.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <cmath>
#include <iostream>

using namespace bluesky::operations;

namespace {

DecodedMavlinkMessage distanceMessage(std::uint8_t packet_sequence_unused,
                                       std::uint8_t sensor_id,
                                       std::uint32_t boot_ms,
                                       std::int64_t received_ms,
                                       double distance_m) {
    (void)packet_sequence_unused;
    DecodedMavlinkMessage m;
    m.kind = MavlinkMessageKind::DistanceSensor;
    m.vehicle_id = "UAV-ALT-01";
    m.source_id = "PX4:MAVLink2:1:1";
    m.system_id = 1;
    m.component_id = 1;
    m.source_timestamp_ms = received_ms; // receive-time fallback in decoder
    m.received_timestamp_ms = received_ms;
    m.timestamp_is_receive_time_fallback = true;
    m.source_time_boot_ms = boot_ms;
    m.valid = true;
    MavlinkDistanceSensorSample s;
    s.source_time_boot_ms = boot_ms;
    s.received_timestamp_ms = received_ms;
    s.minimum_distance_cm = 30;
    s.maximum_distance_cm = 10000;
    s.sensor_type = 3;
    s.sensor_id = sensor_id;
    s.orientation = kMavSensorOrientationPitch270;
    s.distance_m = distance_m;
    s.valid = true;
    m.distance_sensor = s;
    return m;
}

DecodedMavlinkMessage pressureMessage(std::uint32_t boot_ms,
                                     std::int64_t received_ms,
                                     double pressure_hpa) {
    DecodedMavlinkMessage m;
    m.kind = MavlinkMessageKind::ScaledPressure;
    m.vehicle_id = "UAV-ALT-01";
    m.source_id = "PX4:MAVLink2:1:1";
    m.system_id = 1;
    m.component_id = 1;
    m.source_timestamp_ms = received_ms;
    m.received_timestamp_ms = received_ms;
    m.timestamp_is_receive_time_fallback = true;
    m.source_time_boot_ms = boot_ms;
    m.valid = true;
    m.barometric_pressure = MavlinkBarometricPressureSample{
        boot_ms, received_ms, pressure_hpa, true};
    return m;
}

MavlinkAltitudeDisplaySensorConfig config() {
    return {true, 4, 3, 1000, 500};
}

void testConfiguredVehicleIdentityRejectsOtherVehicles() {
    MavlinkAltitudeDisplayRuntime runtime(
        config(), "UAV-ALT-01", "PX4:MAVLink2:1:1", 1, 1);
    auto other = distanceMessage(1, 4, 1000, 10000, 33.0);
    other.vehicle_id = "UAV-ALT-02";
    other.system_id = 2;
    assert(!runtime.ingestAccepted(other));
    assert(!runtime.hasAnyAltitudeInput());
}

void testInitialStateUnavailable() {
    MavlinkAltitudeDisplayRuntime runtime(config(), "UAV-ALT-01", "PX4:MAVLink2:1:1", 1, 1);
    assert(!runtime.hasAnyAltitudeInput());
    const auto d = runtime.evaluate(10000);
    assert(!d.decision.value_available);
    assert(d.decision.source == AltitudeDisplaySource::Unavailable);
}

void testConfiguredSensorCannotBeOverwrittenByOtherSensorId() {
    MavlinkAltitudeDisplayRuntime runtime(config(), "UAV-ALT-01", "PX4:MAVLink2:1:1", 1, 1);
    assert(runtime.ingestAccepted(distanceMessage(1, 4, 1000, 10000, 40.0)));
    // Another downward sensor arrives later; the configured sensor's reading
    // must stay independent even when the non-configured sensor is newer.
    assert(runtime.ingestAccepted(distanceMessage(2, 9, 2000, 10100, 80.0)));
    assert(runtime.ingestAccepted(pressureMessage(2100, 10110, 1008.0)));
    const auto d = runtime.evaluate(10120);
    assert(d.decision.source == AltitudeDisplaySource::RadioAltimeterAgl);
    assert(d.decision.value_available);
    assert(std::abs(d.decision.value_m - 40.0) < 1e-9);
    assert(d.timestamp_quality_degraded);
}

void testOutOfOrderAndDuplicateSamplesAreRejected() {
    MavlinkAltitudeDisplayRuntime runtime(config(), "UAV-ALT-01", "PX4:MAVLink2:1:1", 1, 1);
    assert(runtime.ingestAccepted(distanceMessage(1, 4, 1000, 10000, 40.0)));
    // Older boot time must not replace a newer sample, even if it arrived later.
    assert(!runtime.ingestAccepted(distanceMessage(2, 4, 900, 10100, 70.0)));
    // Boot time advancing while receive time moves backwards is also rejected.
    assert(!runtime.ingestAccepted(distanceMessage(3, 4, 1100, 9900, 90.0)));
    // Same boot tick is a duplicate measurement for this sensor.
    assert(!runtime.ingestAccepted(distanceMessage(4, 4, 1000, 10200, 60.0)));
    assert(runtime.ingestAccepted(pressureMessage(1200, 10210, 1008.0)));
    const auto d = runtime.evaluate(10220);
    assert(d.decision.source == AltitudeDisplaySource::RadioAltimeterAgl);
    assert(d.decision.value_m == 40.0);
}

void testBaroOnlyFallbackIsExplicitlyDegraded() {
    MavlinkAltitudeDisplayRuntime runtime(config(), "UAV-ALT-01", "PX4:MAVLink2:1:1", 1, 1);
    assert(runtime.ingestAccepted(pressureMessage(1000, 10000, 900.0)));
    const auto d = runtime.evaluate(10100);
    assert(d.decision.source ==
           AltitudeDisplaySource::BarometricStandardPressureDegraded);
    assert(d.decision.value_available);
    assert(d.decision.degraded);
    assert(d.timestamp_quality_degraded);
}

void testNoFallbackToGlobalPositionAltitude() {
    MavlinkAltitudeDisplayRuntime runtime(config(), "UAV-ALT-01", "PX4:MAVLink2:1:1", 1, 1);
    DecodedMavlinkMessage position;
    position.kind = MavlinkMessageKind::GlobalPositionInt;
    position.vehicle_id = "UAV-ALT-01";
    position.source_id = "PX4:MAVLink2:1:1";
    position.system_id = 1;
    position.component_id = 1;
    position.source_timestamp_ms = 10000;
    position.received_timestamp_ms = 10000;
    position.source_time_boot_ms = 1000;
    position.altitude_m = 500.0;
    position.valid = true;
    assert(!runtime.ingestAccepted(position));
    assert(!runtime.evaluate(10100).decision.value_available);
}

void testStaleSamplesBecomeUnavailable() {
    MavlinkAltitudeDisplayRuntime runtime(config(), "UAV-ALT-01", "PX4:MAVLink2:1:1", 1, 1);
    assert(runtime.ingestAccepted(distanceMessage(1, 4, 1000, 10000, 40.0)));
    assert(runtime.ingestAccepted(pressureMessage(1001, 10010, 1008.0)));
    const auto d = runtime.evaluate(12000);
    assert(!d.decision.value_available);
    assert(d.decision.source == AltitudeDisplaySource::Unavailable);
}

} // namespace

int main() {
    testConfiguredVehicleIdentityRejectsOtherVehicles();
    testInitialStateUnavailable();
    testConfiguredSensorCannotBeOverwrittenByOtherSensorId();
    testOutOfOrderAndDuplicateSamplesAreRejected();
    testBaroOnlyFallbackIsExplicitlyDegraded();
    testNoFallbackToGlobalPositionAltitude();
    testStaleSamplesBecomeUnavailable();
    std::cout << "mavlink_altitude_display_runtime_test: PASS\n";
    return 0;
}
