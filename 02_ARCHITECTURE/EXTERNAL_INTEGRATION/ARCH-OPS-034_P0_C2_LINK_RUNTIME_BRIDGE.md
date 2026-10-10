# ARCH-OPS-034 — P0 C2 Link Runtime Bridge

## Purpose
Connect the executable transport-channel runtime to the existing deterministic C2 Channel Manager without introducing a second selection or failover implementation.

## Boundary
MavlinkTransportChannelRuntime → C2LinkRuntimeBridge → C2ChannelManager

## Responsibilities
- Transport runtime: connection, session, endpoint, heartbeat and lifecycle state.
- Measurement provider: latency, packet loss, capacity, integrity and authentication measurements.
- Bridge: state/metric translation and validation only.
- C2 Channel Manager: qualification, deterministic selection and failover eligibility.

## Important constraint
The bridge does not perform a physical channel switch. It also does not invent safety thresholds or claim that a transport is operational merely because a socket is connected.

## Multi-link rationale
MAVLink explicitly notes that sequence numbers are per channel and cannot by themselves deduplicate redundant multi-channel traffic. Therefore BlueSky keeps channel identity in the C2 layer and feeds each channel independently into qualification/selection.

## Evidence limitation
SIL_C2_TRANSPORT_MANAGER_BRIDGE only. No radio, cellular, serial, field-network, SITL/HIL, physical-flight or certification evidence.
