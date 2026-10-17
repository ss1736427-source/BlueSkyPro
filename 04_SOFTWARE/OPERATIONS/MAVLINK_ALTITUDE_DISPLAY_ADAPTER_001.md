# MAVLink Altitude Display Adapter

**ID:** OPS-ALT-ADAPTER-001  
**Status:** IMPLEMENTED — source-to-selector adapter contract; live HMI binding pending

## Behavior

The adapter consumes the distinct `SCALED_PRESSURE` and `DISTANCE_SENSOR` samples preserved by the MAVLink decoder. It does not use generic `GLOBAL_POSITION_INT.altitude_m` as either source.

A distance sensor is accepted as radio-altimeter AGL only if the vehicle configuration explicitly identifies the sensor ID and sensor type as the radio altimeter and the packet reports downward orientation. The reading must be valid, fresh, and have a nonzero vehicle boot-time counter.

Absolute static pressure is converted to ISA pressure altitude using 1013.25 hPa. Both sources must be fresh; when both are present, their receive timestamps must be within the configured coherence window before barometric switching is permitted.

The adapter exposes a timestamp-quality flag when the raw decoder used receive time as a fallback because the vehicle boot clock is not yet synchronized to the host clock.

## Verification boundary

Regression tests cover the 100 m boundary, sensor capability matching, invalid orientation, stale values, source timestamp skew and degraded fallback. The adapter returns a display decision only. It does not command the UAV or establish route clearance/flight readiness.

Live QML integration and real vehicle capability configuration remain separate work.
