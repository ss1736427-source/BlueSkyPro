#include "measurement_instrument_adapter_baseline.hpp"
#include <cassert>
int main(){using namespace bluesky::measurement; MeasurementInstrumentAdapterBaseline a; auto m=a.readMeasurement(); assert(m.sourceId=="SRC-20"); assert(m.valid); auto r=a.acceptMeasurement(m); assert(r.accepted); m.sourceId="BAD"; r=a.acceptMeasurement(m); assert(!r.accepted&&r.error==MeasurementError::InvalidData); m.sourceId="SRC-20"; m.stale=true; r=a.acceptMeasurement(m); assert(!r.accepted&&r.error==MeasurementError::StaleData); return 0;}
