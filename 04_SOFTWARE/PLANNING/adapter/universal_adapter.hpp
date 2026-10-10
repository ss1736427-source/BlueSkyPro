#pragma once

#include <string>
#include <vector>

namespace bluesky::planning::adapter {

enum class ErrorCode {
    Unsupported,
    InvalidRequest,
    Timeout,
    NoConnection,
    StaleData,
    InvalidData,
    ProtocolError,
    DeviceError,
    VersionMismatch,
    AuthenticationError,
    ConfigurationError,
};

struct Error {
    ErrorCode code{};
    std::string message;
};

struct ResponseEnvelope {
    std::string request_id;
    std::string adapter_id;
    std::string adapter_version;
    std::string object_id;
    std::string object_version;
    std::string timestamp;
    std::string validity;
    std::string quality;
    std::string source;
};

struct AdapterMetadata {
    std::string adapter_id;
    std::string adapter_version;
    std::string vendor;
    std::string protocol;
    std::string contract_version;
    std::string schema_version;
    std::vector<std::string> supported_vehicle_profiles;
    std::vector<std::string> supported_equipment_profiles;
    std::vector<std::string> capabilities;
    std::string configuration_schema_version;
};

struct CanonicalCommand {
    std::string request_id;
    std::string object_id;
    std::string operation;
};

struct ExternalCommand {
    std::string request_id;
    std::string object_id;
    std::string operation;
};

class UniversalAdapter {
public:
    virtual ~UniversalAdapter() = default;

    virtual AdapterMetadata metadata() const = 0;
    virtual std::vector<std::string> discover() = 0;
    virtual std::string identify() = 0;
    virtual ResponseEnvelope readVehicleProfile() = 0;
    virtual ResponseEnvelope readConfiguration() = 0;
    virtual ResponseEnvelope readCapabilities() = 0;
    virtual ResponseEnvelope readState() = 0;
    virtual ResponseEnvelope readHealth() = 0;
    virtual ResponseEnvelope readC2State() = 0;
    virtual ResponseEnvelope subscribeTelemetry() = 0;
    virtual ResponseEnvelope readEquipment() = 0;
    virtual ExternalCommand translateCommand(const CanonicalCommand& command) = 0;
    virtual Error mapError(ErrorCode code, std::string message) const = 0;

    virtual std::string identifyEquipment() = 0;
    virtual ResponseEnvelope readEquipmentProfile() = 0;
    virtual ResponseEnvelope readEquipmentConfiguration() = 0;
    virtual ResponseEnvelope readEquipmentCapabilities() = 0;
    virtual ResponseEnvelope readEquipmentState() = 0;
    virtual ResponseEnvelope readEquipmentTelemetry() = 0;
    virtual ExternalCommand translateAction(const CanonicalCommand& command) = 0;
    virtual ResponseEnvelope readDataOutputs() = 0;
};

} // namespace bluesky::planning::adapter
