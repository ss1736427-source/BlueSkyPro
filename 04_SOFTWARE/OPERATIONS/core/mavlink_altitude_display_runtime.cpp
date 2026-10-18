#include "mavlink_altitude_display_runtime.hpp"

#include <utility>

namespace bluesky::operations {

MavlinkAltitudeDisplayRuntime::MavlinkAltitudeDisplayRuntime(
    MavlinkAltitudeDisplaySensorConfig sensor_config,
    std::string vehicle_id, std::string source_id,
    std::uint8_t system_id, std::uint8_t component_id)
    : sensor_config_(std::move(sensor_config)),
      vehicle_id_(std::move(vehicle_id)),
      source_id_(std::move(source_id)),
      system_id_(system_id),
      component_id_(component_id) {}

MavlinkAltitudeDisplayRuntime::SourceKey
MavlinkAltitudeDisplayRuntime::sourceKey(
    const DecodedMavlinkMessage& message) {
    return {message.vehicle_id, message.source_id,
            message.system_id, message.component_id};
}

bool MavlinkAltitudeDisplayRuntime::advances(
    std::uint32_t incoming_boot_ms,
    std::int64_t incoming_received_ms,
    std::uint32_t cached_boot_ms,
    std::int64_t cached_received_ms) {
    if (incoming_boot_ms == 0 || incoming_received_ms <= 0) {
        return false;
    }
    // Both source boot-time and host receive-time must advance. A rebooted
    // source clock requires a new session/runtime; it must not silently replace
    // the current source sample in the same session.
    return incoming_boot_ms > cached_boot_ms &&
           incoming_received_ms > cached_received_ms;
}

bool MavlinkAltitudeDisplayRuntime::ingestAccepted(
    const DecodedMavlinkMessage& message) {
    if (!message.valid || vehicle_id_.empty() || source_id_.empty() ||
        system_id_ == 0 || component_id_ == 0 ||
        message.vehicle_id != vehicle_id_ || message.source_id != source_id_ ||
        message.system_id != system_id_ || message.component_id != component_id_ ||
        message.received_timestamp_ms <= 0 ||
        !message.source_time_boot_ms.has_value() ||
        *message.source_time_boot_ms == 0) {
        return false;
    }

    const auto key = sourceKey(message);
    const auto boot_ms = *message.source_time_boot_ms;

    if (message.kind == MavlinkMessageKind::DistanceSensor &&
        message.distance_sensor.has_value()) {
        const auto& sensor = *message.distance_sensor;
        if (!sensor.valid || !sensor.distance_m.has_value() ||
            sensor.source_time_boot_ms != boot_ms ||
            sensor.received_timestamp_ms != message.received_timestamp_ms) {
            return false;
        }

        auto& sensors = distance_by_source_[key];
        const auto cached = sensors.find(sensor.sensor_id);
        if (cached != sensors.end() &&
            !advances(boot_ms, message.received_timestamp_ms,
                      cached->second.sample.source_time_boot_ms,
                      cached->second.sample.received_timestamp_ms)) {
            return false;
        }

        sensors[sensor.sensor_id] = CachedDistance{
            sensor, message.timestamp_is_receive_time_fallback};
        return true;
    }

    if (message.kind == MavlinkMessageKind::ScaledPressure &&
        message.barometric_pressure.has_value()) {
        const auto& pressure = *message.barometric_pressure;
        if (!pressure.valid ||
            pressure.source_time_boot_ms != boot_ms ||
            pressure.received_timestamp_ms != message.received_timestamp_ms) {
            return false;
        }

        const auto cached = pressure_by_source_.find(key);
        if (cached != pressure_by_source_.end() &&
            !advances(boot_ms, message.received_timestamp_ms,
                      cached->second.sample.source_time_boot_ms,
                      cached->second.sample.received_timestamp_ms)) {
            return false;
        }

        pressure_by_source_[key] = CachedPressure{
            pressure, message.timestamp_is_receive_time_fallback};
        return true;
    }

    return false;
}

MavlinkAltitudeDisplayAdapterResult
MavlinkAltitudeDisplayRuntime::evaluate(std::int64_t now_ms) const {
    // Evaluate a complete same-source pair first. Never combine messages from
    // distinct vehicle/source/component identities.
    for (const auto& [key, pressure] : pressure_by_source_) {
        if (std::get<0>(key) != vehicle_id_ ||
            std::get<1>(key) != source_id_ ||
            std::get<2>(key) != system_id_ ||
            std::get<3>(key) != component_id_) {
            continue;
        }
        const auto distances = distance_by_source_.find(key);
        if (distances == distance_by_source_.end()) {
            continue;
        }

        MavlinkTelemetrySample sample;
        sample.vehicle_id = std::get<0>(key);
        sample.source_id = std::get<1>(key);
        sample.system_id = std::get<2>(key);
        sample.component_id = std::get<3>(key);
        sample.received_timestamp_ms = pressure.sample.received_timestamp_ms;
        sample.source_timestamp_ms = sample.received_timestamp_ms;
        sample.barometric_pressure = pressure.sample;
        sample.timestamp_is_receive_time_fallback =
            pressure.timestamp_quality_degraded;

        // Only the configured sensor may reach the selector. Non-configured
        // DISTANCE_SENSORs remain cached but cannot replace the selected source.
        if (sensor_config_.radio_altimeter_configured) {
            const auto selected = distances->second.find(
                sensor_config_.radio_altimeter_sensor_id);
            if (selected != distances->second.end()) {
                sample.distance_sensor = selected->second.sample;
                sample.timestamp_is_receive_time_fallback =
                    sample.timestamp_is_receive_time_fallback ||
                    selected->second.timestamp_quality_degraded;
            }
        }

        auto result = select_mavlink_altitude_display(sample, sensor_config_, now_ms);
        result.timestamp_quality_degraded = true;
        return result;
    }

    // A radio-altimeter reading is independently usable below the selector's
    // threshold. Do not require a barometric sample merely to display valid AGL.
    if (sensor_config_.radio_altimeter_configured) {
        for (const auto& [key, sensors] : distance_by_source_) {
            if (std::get<0>(key) != vehicle_id_ ||
                std::get<1>(key) != source_id_ ||
                std::get<2>(key) != system_id_ ||
                std::get<3>(key) != component_id_) {
                continue;
            }
            const auto selected = sensors.find(sensor_config_.radio_altimeter_sensor_id);
            if (selected == sensors.end())
                continue;

            MavlinkTelemetrySample sample;
            sample.vehicle_id = std::get<0>(key);
            sample.source_id = std::get<1>(key);
            sample.system_id = std::get<2>(key);
            sample.component_id = std::get<3>(key);
            sample.received_timestamp_ms = selected->second.sample.received_timestamp_ms;
            sample.source_timestamp_ms = sample.received_timestamp_ms;
            sample.distance_sensor = selected->second.sample;
            sample.timestamp_is_receive_time_fallback =
                selected->second.timestamp_quality_degraded;
            const auto result = select_mavlink_altitude_display(
                sample, sensor_config_, now_ms);
            if (result.decision.value_available) {
                auto qualified = result;
                qualified.timestamp_quality_degraded = true;
                return qualified;
            }
        }
    }

    // No paired sample: only the existing selector's explicitly degraded BARO
    // fallback is allowed. Generic GLOBAL_POSITION_INT altitude is never used.
    for (const auto& [key, pressure] : pressure_by_source_) {
        if ((!vehicle_id_.empty() && std::get<0>(key) != vehicle_id_) ||
            (!source_id_.empty() && std::get<1>(key) != source_id_) ||
            (system_id_ != 0 && std::get<2>(key) != system_id_) ||
            (component_id_ != 0 && std::get<3>(key) != component_id_)) {
            continue;
        }
        MavlinkTelemetrySample sample;
        sample.vehicle_id = std::get<0>(key);
        sample.source_id = std::get<1>(key);
        sample.system_id = std::get<2>(key);
        sample.component_id = std::get<3>(key);
        sample.received_timestamp_ms = pressure.sample.received_timestamp_ms;
        sample.source_timestamp_ms = sample.received_timestamp_ms;
        sample.barometric_pressure = pressure.sample;
        sample.timestamp_is_receive_time_fallback =
            pressure.timestamp_quality_degraded;
        auto result = select_mavlink_altitude_display(sample, sensor_config_, now_ms);
        result.timestamp_quality_degraded = true;
        return result;
    }

    return {};
}

void MavlinkAltitudeDisplayRuntime::clear() {
    distance_by_source_.clear();
    pressure_by_source_.clear();
}

bool MavlinkAltitudeDisplayRuntime::hasAnyAltitudeInput() const {
    return !distance_by_source_.empty() || !pressure_by_source_.empty();
}

} // namespace bluesky::operations
