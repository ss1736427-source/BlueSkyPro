# ARCH-OPS-039 — Fleet addressing runtime

Status: IMPLEMENTATION BASELINE — CI verification pending

## Scope

Close the executable portion of G2-10 for deterministic multi-UAV identity/address registration and local routing resolution.

## Behavior

- each registered vehicle has a non-zero MAVLink system/component address;
- vehicle identity, source identity and session identity remain separate fields;
- an address can belong to only one registered vehicle at a time;
- a vehicle can have only one active binding in this runtime registry;
- identical re-registration is idempotent;
- conflicting vehicle or address registration is rejected;
- address-to-vehicle and vehicle-to-address resolution are deterministic;
- unregistering a vehicle releases its address for a later registration.

## Boundary

This runtime is an addressing/identity registry, not a transport router and not a flight-safety system. It does not assign MAVLink IDs, alter autopilot configuration, perform network broadcast discovery, or claim physical collision avoidance.

## Evidence

SIL_FLEET_ADDRESSING_RUNTIME

No field/radio evidence, dynamic network routing, ID allocation protocol, or physical collision-avoidance evidence is claimed.
