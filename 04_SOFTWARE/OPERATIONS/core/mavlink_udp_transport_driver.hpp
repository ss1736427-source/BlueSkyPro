#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
namespace bluesky::operations {
struct MavlinkUdpEndpoint { std::string host; std::uint16_t port{0}; };
struct MavlinkUdpTransportStats { std::uint64_t sent_packets{0}, received_packets{0}, send_failures{0}, receive_failures{0}; };
class MavlinkUdpTransportDriver final {
public:
    MavlinkUdpTransportDriver() = default; ~MavlinkUdpTransportDriver();
    MavlinkUdpTransportDriver(const MavlinkUdpTransportDriver&) = delete;
    MavlinkUdpTransportDriver& operator=(const MavlinkUdpTransportDriver&) = delete;
    bool open(const MavlinkUdpEndpoint& local); void close(); bool isOpen() const;
    std::optional<MavlinkUdpEndpoint> localEndpoint() const;
    bool sendTo(const MavlinkUdpEndpoint&, const std::vector<std::uint8_t>&);
    std::optional<std::vector<std::uint8_t>> receive(MavlinkUdpEndpoint* source = nullptr);
    const MavlinkUdpTransportStats& stats() const { return stats_; }
private:
#ifdef _WIN32
    std::uintptr_t socket_{static_cast<std::uintptr_t>(~0ULL)};
#else
    int socket_{-1};
#endif
    std::optional<MavlinkUdpEndpoint> local_endpoint_;
    MavlinkUdpTransportStats stats_;
    bool initialized_{false};
    static bool initializePlatform(); static void shutdownPlatform();
};
}