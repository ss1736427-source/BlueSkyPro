#include "measurement_instrument_adapter_baseline.hpp"
namespace bluesky::measurement {
std::string MeasurementInstrumentAdapterBaseline::getSourceId() const { return "SRC-20"; }
MeasurementRecord MeasurementInstrumentAdapterBaseline::readMeasurement() const { MeasurementRecord r; r.sourceId=getSourceId(); r.instrumentId="INST-20"; r.parameter="value"; r.unit="unit"; r.timestampUtcMs=0; r.type=MeasurementType::Other; r.quality=MeasurementQuality::Valid; r.valid=true; r.stale=false; return r; }
MeasurementResult MeasurementInstrumentAdapterBaseline::acceptMeasurement(const MeasurementRecord& m) { MeasurementResult r; if(m.sourceId!=getSourceId()||m.instrumentId.empty()||m.parameter.empty()||m.unit.empty()){r.error=MeasurementError::InvalidData;r.reason="invalid measurement identity";return r;} if(!m.valid||m.quality==MeasurementQuality::Invalid){r.error=MeasurementError::InvalidData;r.reason="invalid measurement";return r;} if(m.stale||m.quality==MeasurementQuality::Stale){r.error=MeasurementError::StaleData;r.reason="stale measurement";return r;} last_=m;r.accepted=true;return r; }
} // namespace bluesky::measurement
