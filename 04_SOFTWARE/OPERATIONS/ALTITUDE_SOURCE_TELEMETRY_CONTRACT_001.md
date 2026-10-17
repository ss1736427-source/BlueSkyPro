# Altitude Source Telemetry Contract

**ID:** OPS-ALT-TELEMETRY-001  
**Status:** IMPLEMENTED — typed conversion/validation boundary; MAVLink ingestion and HMI wiring remain pending

## Radio-altimeter input

A measurement may be treated as radio-altimeter AGL only when the vehicle/sensor configuration explicitly declares the sensor as a radio altimeter and its orientation is downward. A generic distance sensor or downward-facing rangefinder must not be silently reclassified as a radio altimeter.

The measurement must be finite, non-negative, valid and fresh, with source identity and normalized source/receipt timestamps.

## Barometric input

The barometric path accepts absolute static pressure in hPa and converts it to ISA pressure altitude using standard sea-level pressure **1013.25 hPa**. Differential/dynamic pressure is not accepted. The result is pressure altitude, not AGL and not terrain elevation.

The formula is an ISA conversion; it does not compensate for local QNH or non-standard atmospheric temperature. The UI must label the value as barometric standard-pressure altitude.

## Timestamp boundary

Source and receipt timestamps must first be normalized to a common monotonic clock domain. MAVLink `time_boot_ms` is time since the vehicle booted and must not be compared directly to a host wall-clock timestamp. The existing raw decoder still needs an explicit time-normalization path before these readings can be produced from live MAVLink packets.

## Verification and remaining integration

The C++ contract test covers sensor identity/orientation, invalid and stale data, standard-pressure conversion and malformed values. This commit does not yet parse MAVLink `DISTANCE_SENSOR` / `SCALED_PRESSURE`, establish vehicle capability automatically, or connect the selected source to QML. Those are separate adapter and UI integration steps.
