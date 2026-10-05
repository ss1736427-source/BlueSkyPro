#include "mavlink_transport_channel_runtime.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

static void crc(std::uint8_t d, std::uint16_t& c) {
    auto t = static_cast<std::uint8_t>(d ^ static_cast<std::uint8_t>(c & 255));
    auto t2 = static_cast<std::uint8_t>(t ^ static_cast<std::uint8_t>(t << 4));
    c = static_cast<std::uint16_t>((c >> 8) ^
        (static_cast<std::uint16_t>(t2) << 8) ^
        (static_cast<std::uint16_t>(t2) << 3) ^
        (static_cast<std::uint16_t>(t2) >> 4));
}

static std::vector<std::uint8_t> heartbeat(std::uint8_t seq, std::uint8_t health = 3, std::uint8_t system_id = 1) {
    std::vector<std::uint8_t> f{0xFD, 9, 0, 0, seq, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, health, 0, 0, 0};
    f[5] = system_id;
    std::uint16_t c = 0xffff;
    for (std::size_t i = 1; i < f.size() - 2; ++i) crc(f[i], c);
    crc(50, c);
    f[f.size() - 2] = static_cast<std::uint8_t>(c);
    f[f.size() - 1] = static_cast<std::uint8_t>(c >> 8);
    return f;
}

using namespace bluesky::operations;
static void put_i64(std::vector<std::uint8_t>& f, std::size_t offset, std::int64_t value) {
    const auto u = static_cast<std::uint64_t>(value);
    for (std::size_t i = 0; i < 8; ++i)
        f[offset + i] = static_cast<std::uint8_t>(u >> (8 * i));
}

static std::vector<std::uint8_t> timesync(std::uint8_t seq, std::int64_t tc1, std::int64_t ts1) {
    std::vector<std::uint8_t> f(10 + 16 + 2, 0);
    f[0] = 0xFD; f[1] = 16; f[4] = seq; f[5] = 1; f[6] = 1;
    f[7] = 111; f[8] = 0; f[9] = 0;
    put_i64(f, 10, tc1); put_i64(f, 18, ts1);
    std::uint16_t c = 0xffff;
    for (std::size_t i = 1; i < f.size() - 2; ++i) crc(f[i], c);
    crc(34, c);
    f[f.size() - 2] = static_cast<std::uint8_t>(c);
    f[f.size() - 1] = static_cast<std::uint8_t>(c >> 8);
    return f;
}


static MavlinkTransportChannelConfig config(const char* channel, const char* session,
    const char* vehicle, const char* source, std::uint8_t sysid) {
    MavlinkTransportChannelConfig c;
    c.channel_id = channel; c.session_id = session; c.vehicle_id = vehicle;
    c.source_id = source; c.dialect = MavlinkDialect::ArduPilot;
    c.system_id = sysid; c.component_id = 1;
    c.transport = MavlinkTransportType::DeterministicMemory;
    return c;
}

int main() {
    MavlinkTransportChannelRuntime r(3000);
    assert(r.registerChannel(config("CH-A","SESSION-A","UAV-A","ArduPilot:MAVLink2:1:1",1)));
    assert(r.registerChannel(config("CH-B","SESSION-B","UAV-B","ArduPilot:MAVLink2:2:1",2)));
    auto duplicateAddress = config("CH-C","SESSION-C","UAV-C","ArduPilot:MAVLink2:3:1",1);
    assert(!r.registerChannel(duplicateAddress));
    assert(r.connect("CH-A")); assert(r.connect("CH-B"));

    auto a = r.snapshot("CH-A"); auto b = r.snapshot("CH-B");
    assert(a && b);
    auto frame_a = heartbeat(10); auto frame_b = heartbeat(200, 3, 2);
    assert(r.send("CH-A",1000,frame_a));
    assert(r.routeIncomingFrame(1000,frame_a).has_value());
    const auto routed_b = r.routeIncomingFrame(1001,frame_b);
    assert(routed_b && *routed_b == "CH-B");
    auto unknown = heartbeat(201);
    unknown[5] = 99;
    assert(!r.routeIncomingFrame(1002,unknown).has_value());
    assert(r.receive("CH-A").has_value()); assert(r.receive("CH-B").has_value());
    a=r.snapshot("CH-A"); b=r.snapshot("CH-B");
    assert(a->link_metrics.observed_packets==1 && a->link_metrics.inferred_lost_packets==0);
    assert(a->link_bandwidth.transmitted_bytes == frame_a.size());
    assert(a->link_bandwidth.received_bytes == frame_a.size());
    assert(b->link_metrics.last_sequence==200);

    assert(r.fail("CH-A")); assert(!r.send("CH-A",2000,frame_a));
    assert(r.reconnect("CH-A")); a=r.snapshot("CH-A");
    assert(a->link_metrics.observed_packets==0 && !a->link_metrics.sequence_initialized);
    assert(a->link_bandwidth.transmitted_bytes == 0 && a->link_bandwidth.received_bytes == 0);

    auto recovered=heartbeat(20);
    assert(r.injectReceive("CH-A",5000,recovered)); assert(r.receive("CH-A").has_value());
    a=r.snapshot("CH-A"); assert(a->link_metrics.observed_packets==1 && a->link_metrics.last_sequence==20);
    auto ts_request = timesync(21, 0, 7000000);
    auto ts_response = timesync(22, 123, 7000000);
    assert(r.send("CH-A", 7000, ts_request));
    assert(r.injectReceive("CH-A", 7075, ts_response));
    assert(r.receive("CH-A").has_value());
    a = r.snapshot("CH-A");
    assert(a->link_latency.probes_sent == 1);
    assert(a->link_latency.responses_received == 1);
    assert(a->link_latency.last_rtt_ms && *a->link_latency.last_rtt_ms == 75);

    MavlinkTransportChannelRuntime udp_runtime(3000);
    auto udp=config("UDP-CH","UDP-SESSION","UAV-UDP","ArduPilot:MAVLink2:1:1",1);
    udp.transport=MavlinkTransportType::Udp; udp.udp_local=MavlinkUdpEndpoint{"127.0.0.1",0};
    assert(udp_runtime.registerChannel(udp)); assert(udp_runtime.connect("UDP-CH"));
    auto udp_snapshot=udp_runtime.snapshot("UDP-CH");
    assert(udp_snapshot && udp_snapshot->config.udp_local);

    MavlinkUdpTransportDriver external_sender;
    assert(external_sender.open({"127.0.0.1",0}));
    const auto sender_endpoint=external_sender.localEndpoint(); assert(sender_endpoint);
    assert(udp_runtime.setUdpIngressPeer("UDP-CH",*sender_endpoint));
    assert(external_sender.sendTo(*udp_snapshot->config.udp_local,heartbeat(42)));
    assert(udp_runtime.pollReceive("UDP-CH",6000));
    udp_snapshot=udp_runtime.snapshot("UDP-CH");
    assert(udp_snapshot->link_metrics.observed_packets==1);
    assert(udp_snapshot->link_bandwidth.received_bytes == heartbeat(42).size());

    assert(external_sender.sendTo(*udp_snapshot->config.udp_local,heartbeat(44)));
    assert(udp_runtime.pollReceive("UDP-CH",6010));
    udp_snapshot=udp_runtime.snapshot("UDP-CH");
    assert(udp_snapshot->link_metrics.inferred_lost_packets==1);
    assert(udp_snapshot->link_metrics.packetLossRatio() == (1.0/3.0));

    assert(external_sender.sendTo(*udp_snapshot->config.udp_local,heartbeat(44)));
    assert(!udp_runtime.pollReceive("UDP-CH",6020));
    udp_snapshot=udp_runtime.snapshot("UDP-CH");
    assert(udp_snapshot->link_metrics.duplicate_packets==0);

    MavlinkUdpTransportDriver unexpected_sender;
    assert(unexpected_sender.open({"127.0.0.1",0}));
    assert(unexpected_sender.sendTo(*udp_snapshot->config.udp_local,heartbeat(45)));
    assert(!udp_runtime.pollReceive("UDP-CH",6030));
    udp_snapshot=udp_runtime.snapshot("UDP-CH");
    assert(udp_snapshot->stats.rejected_frames>=1);
    assert(udp_snapshot->link_metrics.observed_packets==2);

    assert(udp_runtime.tickSession("UDP-SESSION",9020).link_state==MavlinkLinkState::Lost);
    assert(udp_runtime.reconnect("UDP-CH"));
    udp_snapshot=udp_runtime.snapshot("UDP-CH");
    assert(udp_snapshot->link_metrics.observed_packets==0);
    assert(udp_snapshot->link_bandwidth.transmitted_bytes==0 && udp_snapshot->link_bandwidth.received_bytes==0);

    const auto recovered_local=udp_snapshot->config.udp_local;
    MavlinkUdpTransportDriver external_receiver;
    assert(external_receiver.open({"127.0.0.1",0}));
    const auto receiver_endpoint=external_receiver.localEndpoint(); assert(receiver_endpoint);
    assert(udp_runtime.setUdpRemote("UDP-CH",*receiver_endpoint));
    assert(udp_runtime.send("UDP-CH",6100,heartbeat(43)));
    MavlinkUdpEndpoint source; const auto outbound=external_receiver.receive(&source);
    assert(outbound && *outbound==heartbeat(43));
    udp_snapshot=udp_runtime.snapshot("UDP-CH");
    assert(udp_snapshot->link_bandwidth.transmitted_bytes == heartbeat(43).size());

    assert(udp_runtime.setUdpIngressPeer("UDP-CH",*sender_endpoint));
    assert(recovered_local && external_sender.sendTo(*recovered_local,heartbeat(1)));
    assert(udp_runtime.pollReceive("UDP-CH",10000));
    udp_snapshot=udp_runtime.snapshot("UDP-CH");
    assert(udp_snapshot->link_metrics.observed_packets==1 && udp_snapshot->link_metrics.last_sequence==1);

    unexpected_sender.close(); udp_runtime.disconnect("UDP-CH");
    external_sender.close(); external_receiver.close();
    std::cout<<"mavlink_transport_channel_runtime_test: PASS\n";
}
