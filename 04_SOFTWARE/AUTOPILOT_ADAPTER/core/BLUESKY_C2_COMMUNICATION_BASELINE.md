# BlueSky PRO — C2 Communication Baseline

**EC:** EC-01 — C2 Communication
**Status:** IMPLEMENTATION BASELINE — P0

## Purpose

Provide a protocol-independent communication contract between BlueSky PRO and an aircraft endpoint. Concrete transport/protocol adapters shall implement this contract later without changing the upper-level evidence and control interfaces.

## Controlled boundary

```text
BlueSky PRO
    ↓
UniversalC2Interface
    ↓
C2 transport/protocol adapter
    ↓
C2 link
    ↓
UAV endpoint
```

The baseline does not claim a physical radio, satellite, cellular or other C2 transport implementation.

## Contract responsibilities

The interface exposes:

- endpoint identity;
- connection/link state;
- link metrics: latency, packet loss, jitter, bitrate and timestamps;
- authenticated session state;
- transmit frame acceptance;
- receive frame delivery;
- heartbeat exchange;
- explicit error codes and reasons.

## Safety boundary

Loss of C2 is represented as a link condition. Autonomous flight/failsafe policy is not silently implemented by this communication layer; it remains the responsibility of the flight-control/autopilot and safety layers.

## Current implementation

`c2_interface_baseline.hpp/.cpp` provides a deterministic loopback/prototype implementation for contract verification. It is not physical C2 evidence.

`c2_interface_baseline_test.cpp` verifies connection rejection before link establishment, connection, authentication, frame transmission/reception, heartbeat timestamps, authentication failure and disconnect state.

## Evidence status

This baseline closes the prototype implementation mechanism for the EC-01 C2 boundary. EC-01 remains `PARTIAL` in the evidence audit until an approved source adapter, controlled execution artifacts, domain analysis/result linkage, integrity-verified evidence package and physical/operational verification are available.
