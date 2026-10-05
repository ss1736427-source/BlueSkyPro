#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace bluesky::time_sync {

enum class TimeSyncState { Unknown, Synchronized, Warning, Unsynchronized, Fault };
enum class TimeSyncQuality { Unknown, Invalid, Valid, Stale, Degraded };
enum class TimeSyncError { None, InvalidData, StaleData, ConfigurationMismatch };

struct TimeSyncSample {
    std::string sourceId;
    std::int64_t sourceTimestampMs{0};
    std::int64_t receiptTimestampMs{0};
    double offsetMs{0.0};
    double uncertaintyMs{0.0};
    TimeSyncState state{TimeSyncState::Unknown};
    TimeSyncQuality quality{TimeSyncQuality::Unknown};
    bool valid{false};
    bool stale{false};
};

struct TimeSyncResult {
    bool accepted{false};
    TimeSyncError error{TimeSyncError::None};
    std::string reason;
};

class UniversalTimeSynchronizationAdapter {
public:
    virtual ~UniversalTimeSynchronizationAdapter() = default;
    virtual std::string getSourceId() const = 0;
    virtual std::string getTimeReferenceIdentity() const = 0;
    virtual std::string getConfigurationVersion() const = 0;
    virtual TimeSyncState getState() const = 0;
    virtual TimeSyncQuality getQuality() const = 0;
    virtual std::optional<TimeSyncSample> readSample() const = 0;
    virtual TimeSyncResult acceptSample(const TimeSyncSample& sample) = 0;
};

} // namespace bluesky::time_sync
