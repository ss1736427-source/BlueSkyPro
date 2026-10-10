# ARCH-OPS-040 — Fleet/session admission boundary

Status: IMPLEMENTATION BASELINE — CI verification pending

## Scope

Bind the ARCH-OPS-039 deterministic fleet addressing registry to transport-channel admission without duplicating MAVLink session validation or becoming a network router.

## Behavior

- channel registration supplies existing vehicle/source/session/system/component identity to FleetAddressingRuntime;
- duplicate MAVLink system/component addresses are rejected across channels in the same runtime;
- conflicting vehicle bindings are rejected;
- accepted channels continue through the existing MavlinkSessionRuntime boundary;
- no MAVLink ID allocation or rewriting is performed.

## Evidence

SIL_FLEET_SESSION_ADMISSION

No field/radio, dynamic routing, broadcast discovery, ID allocation protocol, or physical collision-avoidance evidence is claimed.
