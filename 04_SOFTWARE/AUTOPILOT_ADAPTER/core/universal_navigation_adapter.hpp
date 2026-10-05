#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::navigation {

enum class FixType { Unknown, NoFix, Fix2D, Fix3D, RTKFloat, RTKFixed };
enum class NavigationQuality { Unknown, Invalid, Degraded, Usable, Verified };
enum class CorrectionState { Unknown, Unavailable, Connecting, Receiving, Stale, Healthy };

enum class NavigationError {
    None,
    NoData,
    InvalidData,
    StaleData,
    SourceUnavailable,
    ConfigurationMismatch,
    Unsupported,
    InternalAdapterError
};

struct Position {
    double latitudeDeg{0.0};
    double longitudeDeg{0.0};
    double altitudeM{0.0};
};

struct Velocity {
    double northMps{0.0};
    double eastMps{0.0};
    double downMps{0.0};
};

struct Attitude {
    double rollDeg{0.0};
    double pitchDeg{0.0};
    double yawDeg{0.0};
};

struct NavigationSample {
    std::string sourceId;
    std::string timestampUtc;
    std::int64_t sourceTimestampMs{0};
    std::int64_t adapterTimestampMs{0};
    Position position;
    Velocity velocity;
    Attitude attitude;
    FixType fixType{FixType::Unknown};
    NavigationQuality quality{NavigationQuality::Unknown};
    CorrectionState correctionState{CorrectionState::Unknown};
    int satellites{0};
    double horizontalAccuracyM{0.0};
    double verticalAccuracyM{0.0};
    bool valid{false};
    bool stale{false};
};

struct NavigationResult {
    bool accepted{false};
    NavigationError error{NavigationError::None};
    std::string reason;
};

class UniversalNavigationAdapter {
public:
    virtual ~UniversalNavigationAdapter() = default;

    virtual std::string getSourceId() const = 0;
    virtual std::string getReceiverIdentity() const = 0;
    virtual std::string getFirmwareVersion() const = 0;
    virtual std::string getProtocolVersion() const = 0;

    virtual NavigationQuality getQuality() const = 0;
    virtual CorrectionState getCorrectionState() const = 0;
    virtual std::optional<NavigationSample> readSample() const = 0;
    virtual NavigationResult acceptSample(const NavigationSample& sample) = 0;
};

} // namespace bluesky::navigation
