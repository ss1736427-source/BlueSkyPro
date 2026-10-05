# ARCH-OPS-044 — Device Capability Runtime

**Status:** IMPLEMENTATION_BASELINE

## Objective

Provide a deterministic runtime registry for the capabilities discovered for an identified UAV, autopilot, component or integrated device.

## Scope

The runtime stores a normalized capability snapshot keyed by `device_id` and preserves:

- device identity reference;
- vehicle and autopilot references;
- firmware reference;
- protocol and protocol version;
- sorted capability identifiers;
- lifecycle state of the capability snapshot.

A capability identifier is an opaque BlueSky capability key such as `mission.upload`, `mission.readback`, `command.arm`, `telemetry.position`, or `parameter.sync`.

## Lifecycle

```
Unknown → Discovered → Validated
```

The runtime does not authenticate a device, negotiate transport, interpret vendor-specific messages, or claim that a capability is operationally safe. Those remain adapter/protocol boundaries.

## Deterministic rules

1. Device ID must be non-empty.
2. Capability identifiers must be non-empty.
3. Duplicate capability identifiers are normalized to one entry.
4. Re-registering an identical snapshot is idempotent.
5. Re-registering a different snapshot for the same device replaces the previous snapshot only through an explicit update operation.
6. Resolve operations return immutable snapshot copies.
7. Snapshot output is sorted deterministically by capability identifier.
8. The runtime is in-memory only; persistence and authorization are outside this boundary.

## Results

- `Registered`
- `AlreadyRegistered`
- `Updated`
- `InvalidSnapshot`
- `NotFound`

## Evidence target

`SIL_DEVICE_CAPABILITY_RUNTIME`

## Boundary

This runtime does not perform live MAVLink capability discovery. It provides the controlled state boundary that a future MAVLink/vendor adapter can populate from heartbeat, capability, firmware and protocol discovery.

## Closure

The capability runtime is consumed by ARCH-OPS-045 discovery and ARCH-OPS-046 compatibility. The historical next-gap statement is closed.

Bind capability discovery to the existing device identity and MAVLink session admission path, then verify capability-to-mission/command compatibility before execution.
