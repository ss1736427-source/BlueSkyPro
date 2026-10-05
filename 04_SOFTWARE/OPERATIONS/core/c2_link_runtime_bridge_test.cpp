#include "c2_link_runtime_bridge.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>

using namespace bluesky::operations;

static MavlinkTransportChannelSnapshot transportSnapshot(
    const char* id, MavlinkTransportChannelState state, int priority) {
    MavlinkTransportChannelSnapshot snapshot;
    snapshot.config.channel_id = id;
    snapshot.config.session_id = "SESSION";
    snapshot.config.vehicle_id = "UAV";
    snapshot.config.source_id = "ArduPilot:MAVLink2:1:1";
    snapshot.config.system_id = 1;
    snapshot.config.component_id = 1;
    snapshot.config.transport = MavlinkTransportType::Udp;
    snapshot.config.priority = priority;
    snapshot.state = state;
    return snapshot;
}

static void runtime_measurement_integration_test();

int main() {
    const auto active = transportSnapshot(
        "UDP-PRIMARY", MavlinkTransportChannelState::Connected, 10);
    const C2LinkMeasurement measurement{
        42.0, 0.02, 128.0, std::nullopt, true, true, 10000};

    const auto mapped = C2LinkRuntimeBridge::toChannelSnapshot(active, measurement);
    assert(mapped);
    assert(mapped->id == "UDP-PRIMARY");
    assert(mapped->transport == "UDP");
    assert(mapped->state == bluesky::c2::ChannelState::Active);
    assert(mapped->latencyMs == 42.0);
    assert(mapped->packetLoss == 0.02);
    assert(mapped->capacityKbps == 128.0);
    assert(mapped->integrityOk);
    assert(mapped->authenticated);
    assert(mapped->priority == 10);

    const auto degraded = transportSnapshot(
        "UDP-BACKUP", MavlinkTransportChannelState::Degraded, 5);
    const auto backup = C2LinkRuntimeBridge::toChannelSnapshot(degraded, measurement);
    assert(backup && backup->state == bluesky::c2::ChannelState::Degraded);

    bluesky::c2::C2ChannelManager manager;
    manager.registerChannel(*mapped);
    manager.registerChannel(*backup);
    const auto selected = manager.selectBest(true);
    assert(selected.status == bluesky::c2::ChannelSelectionStatus::Selected);
    assert(selected.channelId == "UDP-PRIMARY");

    auto failed = transportSnapshot(
        "UDP-PRIMARY", MavlinkTransportChannelState::Lost, 10);
    const auto failedMapped = C2LinkRuntimeBridge::toChannelSnapshot(failed, measurement);
    assert(failedMapped);
    assert(manager.update(*failedMapped));

    // A degraded channel is intentionally not qualified by the existing C2 manager.
    // Promote the backup to a standby transport state to test failover eligibility
    // without changing the manager's qualification policy in ARCH-OPS-034.
    auto standby = transportSnapshot(
        "UDP-BACKUP", MavlinkTransportChannelState::Connected, 5);
    const auto standbyMapped = C2LinkRuntimeBridge::toChannelSnapshot(standby, measurement);
    assert(standbyMapped && standbyMapped->state == bluesky::c2::ChannelState::Active);
    manager.update(*standbyMapped);

    assert(manager.requestFailover("UDP-PRIMARY", true));
    assert(manager.selectBest(true).channelId == "UDP-BACKUP");

    C2LinkMeasurement invalid = measurement;
    invalid.packet_loss = 1.5;
    assert(!C2LinkRuntimeBridge::toChannelSnapshot(active, invalid));

    invalid = measurement;
    invalid.measured_timestamp_ms = 0;
    assert(!C2LinkRuntimeBridge::toChannelSnapshot(active, invalid));

    runtime_measurement_integration_test();

    std::cout << "c2_link_runtime_bridge_test: PASS\n";
}


static void runtime_measurement_integration_test() {
    MavlinkTransportChannelSnapshot transport;
    transport.config.channel_id = "CH-RUNTIME";
    transport.config.transport = MavlinkTransportType::Udp;
    transport.stats.last_receive_timestamp_ms = 2100;
    transport.link_metrics = {};
    transport.link_metrics.observed_packets = 2;
    transport.link_metrics.inferred_lost_packets = 1;
    transport.link_latency.last_rtt_ms = 40;
    transport.link_bandwidth.transmitted_bytes = 200;
    transport.link_bandwidth.received_bytes = 300;
    transport.link_bandwidth.first_transmit_timestamp_ms = 1000;
    transport.link_bandwidth.last_transmit_timestamp_ms = 1100;
    transport.link_bandwidth.first_receive_timestamp_ms = 2000;
    transport.link_bandwidth.last_receive_timestamp_ms = 2100;

    const auto measurement =
        C2LinkRuntimeBridge::fromRuntimeSnapshot(transport);
    assert(measurement.latency_ms == 40.0);
    assert(measurement.packet_loss == (1.0 / 3.0));
    assert(measurement.observed_bandwidth_kbps);
    assert(*measurement.observed_bandwidth_kbps == 40.0);
    assert(measurement.measured_timestamp_ms == 2100);
    assert(measurement.capacity_kbps == 0.0);
}

