# ARCH-OPS-042 — Time synchronization

## Requirement

G2-01 requires a common UTC/timebase, timestamp quality and synchronization status.

## Implemented slice

TimeSynchronizationRuntime provides the executable synchronization state boundary. It estimates remote clock offset using the midpoint between local send and receive timestamps, records the corresponding RTT, and exposes synchronization freshness.

## Authority boundaries

- TimeSynchronizationRuntime owns synchronization observation state.
- LinkLatencyRuntime remains responsible for link RTT measurement.
- NormalizedTelemetryState remains the canonical telemetry state model.
- No UTC claim is made without an independently established UTC reference.
- No C2 channel selection or failover policy is introduced.

## Evidence

SIL_TIME_SYNCHRONIZATION_RUNTIME

The evidence demonstrates deterministic local/remote clock-offset estimation and stale/recovery behavior. It does not demonstrate GNSS/PTP/NTP source accuracy or field synchronization.
