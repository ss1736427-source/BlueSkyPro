#pragma once

#include "universal_time_synchronization_adapter.hpp"

namespace bluesky::time_sync {

class TimeSynchronizationAdapterBaseline final : public UniversalTimeSynchronizationAdapter {
public:
    explicit TimeSynchronizationAdapterBaseline(std::string sourceId = "SRC-12", std::string timeReferenceIdentity = "DEMO-TIME");

    std::string getSourceId() const override;
    std::string getTimeReferenceIdentity() const override;
    std::string getConfigurationVersion() const override;
    TimeSyncState getState() const override;
    TimeSyncQuality getQuality() const override;
    std::optional<TimeSyncSample> readSample() const override;
    TimeSyncResult acceptSample(const TimeSyncSample& sample) override;

private:
    std::string sourceId_;
    std::string timeReferenceIdentity_;
    std::optional<TimeSyncSample> sample_;
};

} // namespace bluesky::time_sync
