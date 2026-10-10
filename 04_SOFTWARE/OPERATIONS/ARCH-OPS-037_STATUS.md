# ARCH-OPS-037 — Per-channel bandwidth runtime

Status: IMPLEMENTATION BASELINE — CI verification pending

## Scope

Adds deterministic per-channel observed transport throughput for the executable C2 channel runtime.

## Runtime behavior

- Records bytes successfully handed to the transport on transmit.
- Records bytes only after a received frame is accepted by the MAVLink session boundary.
- Maintains independent transmit and receive byte totals and observation intervals.
- Exposes average observed transmit/receive rates in bytes per second when a valid elapsed interval exists.
- Rejects zero-byte observations and non-positive timestamps.
- Rejects timestamp regressions without mutating accumulated totals.
- Resets bandwidth state on channel reconnect.
- Does not infer radio capacity, impose thresholds, or select/fail over channels.

## Terminology boundary

This slice measures observed throughput, not the physical maximum capacity of a radio or network link. A future transport-specific capacity source may be integrated without changing this runtime contract.

## Evidence

SIL_PER_CHANNEL_OBSERVED_BANDWIDTH

No field, radio, serial, SITL, HIL, physical, or certification evidence is claimed.
