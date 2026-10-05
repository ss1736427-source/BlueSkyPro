#pragma once

#include "universal_performance_model_adapter.hpp"

namespace bluesky::performance {

class PerformanceModelAdapterBaseline final : public UniversalPerformanceModelAdapter {
public:
    explicit PerformanceModelAdapterBaseline(std::string sourceId = "SRC-10", std::string modelIdentity = "DEMO-PERFORMANCE-MODEL");

    std::string getSourceId() const override;
    std::string getModelIdentity() const override;
    std::string getConfigurationVersion() const override;
    PerformanceState getState() const override;
    PerformanceQuality getQuality() const override;
    std::optional<PerformanceSample> readSample() const override;
    PerformanceResult acceptSample(const PerformanceSample& sample) override;

private:
    std::string sourceId_;
    std::string modelIdentity_;
    std::optional<PerformanceSample> sample_;
};

} // namespace bluesky::performance
