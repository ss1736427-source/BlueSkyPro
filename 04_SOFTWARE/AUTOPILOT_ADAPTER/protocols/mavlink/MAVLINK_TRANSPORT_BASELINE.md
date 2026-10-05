# BlueSky PRO — MAVLink Transport Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## Purpose

Define the transport boundary used by the ArduPilot and PX4 adapters. MAVLink is treated as a protocol transport, not as the BlueSky mission model.

## Required transport services

- connection establishment and termination;
- message receive/send;
- heartbeat/liveness monitoring;
- sequence handling;
- timeout detection;
- link statistics;
- transport error reporting;
- safe reconnect;
- message routing to the appropriate adapter;
- source/system/component identity preservation.

## Separation

```text
BlueSky Core
    ↓
Universal Autopilot API
    ↓
MAVLink Transport
    ↓
Protocol Adapter
   ↙        ↘
ArduPilot    PX4
```

The transport layer shall not contain ArduPilot- or PX4-specific mission semantics.

## Session identity

A connected endpoint shall be identified by transport/session identity and MAVLink system/component identity. Reconnection shall create a new transport session while preserving the operational correlation needed by the runtime.

## Message handling

The implementation shall preserve:

- message type;
- source system/component;
- sequence information;
- receive timestamp;
- raw message where evidence/debugging requires it;
- normalized interpretation status.

Malformed or unsupported messages shall not be silently treated as valid telemetry or command results.

## Command path

```text
Universal Command
       ↓
Adapter translation
       ↓
MAVLink message
       ↓
Transport
       ↓
UAV
       ↓
ACK / result
       ↓
Transport
       ↓
Adapter
       ↓
Universal result
```

An acknowledgement confirms the protocol-level response only; execution state must be determined according to the command semantics.

## Link degradation

Transport shall expose measurable communication state to C2:

`HEALTHY / DEGRADED / LOST / RECOVERING`

The transport must not independently decide mission recovery. That decision belongs to the C2/runtime safety layer.

## Verification

Minimum automated tests:

1. connect/disconnect;
2. heartbeat loss;
3. reconnect;
4. malformed message;
5. unsupported message;
6. delayed message;
7. duplicate message;
8. sequence anomaly;
9. command timeout;
10. ACK reception;
11. simultaneous vehicle sessions;
12. channel failure during mission transfer.

## Acceptance

The transport is accepted when it provides deterministic, observable and testable communication behaviour without introducing autopilot-specific assumptions into BlueSky Core.
