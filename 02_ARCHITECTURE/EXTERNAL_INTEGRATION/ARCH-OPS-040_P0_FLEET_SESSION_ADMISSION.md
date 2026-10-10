# ARCH-OPS-040 — Fleet/session admission boundary

## Requirement

G2-10 requires multi-UAV system/component addressing, routing and collision prevention. ARCH-OPS-039 established the deterministic fleet addressing registry. ARCH-OPS-040 binds that registry to the executable transport/session admission boundary.

## Implemented slice

`MavlinkTransportChannelRuntime::registerChannel()` now registers the channel's existing vehicle/source/session identity and MAVLink system/component address with `FleetAddressingRuntime`.

A channel is rejected when the fleet registry reports an address or vehicle collision. This prevents two transport channels in the same runtime from claiming the same MAVLink system/component address.

The existing `MavlinkSessionRuntime` remains responsible for frame/session identity validation. Fleet addressing does not replace or duplicate the session runtime.

## Boundary

- FleetAddressingRuntime owns deterministic identity/address uniqueness.
- MavlinkTransportChannelRuntime performs admission of configured channels.
- MavlinkSessionRuntime validates incoming frame identity against the admitted channel/session.
- Physical transport routing and network-level collision avoidance remain outside this slice.
- MAVLink IDs are supplied by configuration/integration; this runtime does not allocate or rewrite them.

## Evidence

`SIL_FLEET_SESSION_ADMISSION`

The evidence is deterministic in-process admission and collision rejection. It does not demonstrate field/radio operation, dynamic network routing, ID allocation, or physical collision avoidance.
