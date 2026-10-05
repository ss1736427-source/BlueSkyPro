# ARCH-OPS-041 — Status

**Status:** IMPLEMENTED — awaiting exact-HEAD CI verification

## Scope

Fleet-addressed MAVLink frame routing is implemented in the existing transport-channel runtime without introducing a second registry, router authority, address allocator, or C2 selection mechanism.

## Evidence target

SIL_FLEET_ADDRESS_ROUTING

## Verification cases

- registered address routes to the bound channel;
- a second registered vehicle with a different address routes independently;
- an unknown address is rejected;
- session validation remains downstream of routing;
- existing transport, latency, bandwidth, and fleet-admission tests remain in the same contract-test target.

## Explicit non-claims

No physical/radio routing, dynamic address allocation, broadcast discovery, or physical collision avoidance is claimed by this slice.
