#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bluesky::equipment {

enum class AvailabilityState {
    Unknown,
    Supported,
    NotSupported,
    Verified,
    NotVerified,
    Degraded
};

enum class EquipmentState {
    Unknown,
    Off,
    Initializing,
    Ready,
    Busy,
    Degraded,
    Failed
};

enum class ErrorCode {
    None,
    NotConnected,
    AuthenticationFailed,
    Unsupported,
    IncompatibleVersion,
    CapabilityMissing,
    InvalidState,
    SafetyRejected,
    CommandRejected,
    Timeout,
    LinkLost,
    ProtocolError,
    ConfigurationMismatch,
    InternalAdapterError
};

struct Identity {
    std::string equipmentId;
    std::string profileId;
    std::string manufacturer;
    std::string model;
    std::string serialNumber;
};

struct Capability {
    std::string id;
    AvailabilityState availability{AvailabilityState::Unknown};
};

struct Telemetry {
    std::string equipmentId;
    std::string data;
    std::int64_t sourceTimestampMs{0};
    std::int64_t adapterTimestampMs{0};
    bool valid{false};
    bool stale{false};
    std::string quality;
};

struct DataOutput {
    std::string outputId;
    std::string type;
    std::string format;
};

struct ActionResult {
    std::string actionId;
    std::string equipmentId;
    bool accepted{false};
    ErrorCode error{ErrorCode::None};
    std::string reason;
    std::int64_t sourceTimestampMs{0};
    std::int64_t adapterTimestampMs{0};
};

class UniversalEquipmentAdapter {
public:
    virtual ~UniversalEquipmentAdapter() = default;

    virtual Identity identify() const = 0;
    virtual std::string readProfile() const = 0;
    virtual std::string readConfiguration() const = 0;
    virtual std::vector<Capability> readCapabilities() const = 0;
    virtual EquipmentState readState() const = 0;
    virtual std::optional<Telemetry> readTelemetry() const = 0;
    virtual ActionResult translateAction(const std::string& action) = 0;
    virtual std::vector<DataOutput> readDataOutputs() const = 0;
};

} // namespace bluesky::equipment
