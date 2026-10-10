#pragma once
#include <cstdint>
#include <string>
namespace bluesky::measurement {
enum class MeasurementQuality { Unknown, Valid, Stale, Invalid, Degraded };
enum class MeasurementType { Unknown, Position, Altitude, Speed, Heading, Distance, Temperature, Pressure, Voltage, Current, Energy, Time, Other };
enum class MeasurementError { None, InvalidData, StaleData, Unsupported, SensorFault, InternalAdapterError };
struct MeasurementRecord { std::string sourceId; std::string instrumentId; std::string parameter; std::string unit; std::int64_t timestampUtcMs{0}; double value{0.0}; MeasurementType type{MeasurementType::Unknown}; MeasurementQuality quality{MeasurementQuality::Unknown}; bool valid{false}; bool stale{false}; };
struct MeasurementResult { bool accepted{false}; MeasurementError error{MeasurementError::None}; std::string reason; };
class UniversalMeasurementInstrumentAdapter { public: virtual ~UniversalMeasurementInstrumentAdapter() = default; virtual std::string getSourceId() const = 0; virtual MeasurementRecord readMeasurement() const = 0; virtual MeasurementResult acceptMeasurement(const MeasurementRecord& measurement) = 0; };
} // namespace bluesky::measurement
