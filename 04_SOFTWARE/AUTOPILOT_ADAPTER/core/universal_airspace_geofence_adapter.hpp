#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::airspace {

enum class AirspaceState { Unknown, Clear, Warning, Restricted, Fault };
enum class AirspaceQuality { Unknown, Invalid, Valid, Stale, Degraded };
enum class AirspaceDecision { Unknown, Allowed, Warning, Rejected };
enum class AirspaceError { None, InvalidData, StaleData, ConfigurationMismatch };

struct AirspaceSample {
    std::string sourceId;
    std::int64_t sourceTimestampMs{0};
    double latitudeDeg{0.0};
    double longitudeDeg{0.0};
    double altitudeM{0.0};
    bool insideRestriction{false};
    bool clearancePresent{false};
    AirspaceDecision decision{AirspaceDecision::Unknown};
    AirspaceState state{AirspaceState::Unknown};
    AirspaceQuality quality{AirspaceQuality::Unknown};
    bool valid{false};
    bool stale{false};
};

struct AirspaceResult {
    bool accepted{false};
    AirspaceError error{AirspaceError::None};
    std::string reason;
};

class UniversalAirspaceGeofenceAdapter {
public:
    virtual ~UniversalAirspaceGeofenceAdapter() = default;
    virtual std::string getSourceId() const = 0;
    virtual std::string getAirspaceIdentity() const = 0;
    virtual std::string getConfigurationVersion() const = 0;
    virtual AirspaceState getState() const = 0;
    virtual AirspaceQuality getQuality() const = 0;
    virtual std::optional<AirspaceSample> readSample() const = 0;
    virtual AirspaceResult acceptSample(const AirspaceSample& sample) = 0;
};

} // namespace bluesky::airspace
