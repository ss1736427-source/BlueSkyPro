#pragma once
#include "universal_measurement_instrument_adapter.hpp"
namespace bluesky::measurement {
class MeasurementInstrumentAdapterBaseline final : public UniversalMeasurementInstrumentAdapter { public: std::string getSourceId() const override; MeasurementRecord readMeasurement() const override; MeasurementResult acceptMeasurement(const MeasurementRecord& measurement) override; private: MeasurementRecord last_{}; };
} // namespace bluesky::measurement
