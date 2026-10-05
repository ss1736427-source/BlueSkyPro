# ARCH-OPS-041 — Fleet-addressed frame routing

## Requirement

G2-10 requires multi-UAV system/component addressing, routing and collision prevention. ARCH-OPS-039 established deterministic address ownership and ARCH-OPS-040 bound it to channel admission.

## Implemented slice

MavlinkTransportChannelRuntime::routeIncomingFrame() extracts the MAVLink system/component address from an already received MAVLink v1/v2 frame, resolves that address through the existing fleet registry, and admits the frame to the uniquely bound transport channel.

Unknown addresses are rejected. The route is deterministic because address ownership is unique by construction.

## Authority boundaries

- FleetAddressingRuntime owns address uniqueness and address-to-vehicle resolution.
- MavlinkTransportChannelRuntime owns channel admission and frame queueing.
- MavlinkSessionRuntime remains the authority for MAVLink frame/session validation after routing.
- No new address allocation, address rewriting, broadcast discovery or physical network routing is introduced.
- No C2 channel selection or failover policy is duplicated.

## Evidence

SIL_FLEET_ADDRESS_ROUTING

The evidence demonstrates deterministic in-process routing for registered MAVLink addresses and rejection of an unknown address. It does not demonstrate radio/field routing or physical network collision avoidance.
