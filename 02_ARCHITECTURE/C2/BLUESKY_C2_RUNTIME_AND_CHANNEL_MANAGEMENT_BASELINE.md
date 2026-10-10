# BlueSky PRO — C2 Runtime and Channel Management Baseline

**Status:** ARCHITECTURE BASELINE — P0 integration contract

## 1. Purpose

Define the universal C2 runtime layer between BlueSky and heterogeneous communication systems. BlueSky shall remain independent of the physical/network transport while maintaining deterministic operational behaviour during degradation, loss and recovery of connectivity.

## 2. Position

```text
BLUE SKY CORE
     ↓
C2 CONTROL API
     ↓
C2 SESSION / CHANNEL MANAGER
     ↓
┌──────────┬──────────┬──────────┬──────────┐
│ Channel A│ Channel B│ Channel C│  OEM C2   │
└──────────┴──────────┴──────────┴──────────┘
     ↓
AUTOPILOT ADAPTER
     ↓
UAV
```

## 3. Channel abstraction

A channel is represented by a common interface independent of its transport technology.

Minimum channel properties:

- channel ID;
- transport type;
- endpoint/session identity;
- availability;
- health;
- latency;
- loss rate;
- bandwidth/capacity where relevant;
- integrity/authentication state;
- priority/preference;
- last successful communication timestamp.

## 4. C2 states

The runtime shall maintain explicit states at both session and channel level:

```text
AVAILABLE
DEGRADED
UNAVAILABLE
RECOVERING
ACTIVE
STANDBY
FAILED
```

The system shall distinguish loss of a single channel from loss of the overall C2 session.

## 5. Channel selection

The Channel Manager shall select the best available channel according to the configured operational policy and current measured conditions.

Selection must consider, as applicable:

- availability;
- latency;
- packet loss;
- capacity;
- integrity/security state;
- mission phase;
- command criticality;
- vehicle requirements.

The system shall not switch channels merely because a transient metric changes if doing so would create instability or command ambiguity.

## 6. Failover

When the active channel becomes unsuitable, BlueSky shall execute the defined failover procedure:

```text
ACTIVE
  ↓
DEGRADATION DETECTED
  ↓
VALIDATE ALTERNATIVE
  ↓
SWITCH
  ↓
VERIFY COMMUNICATION
  ↓
ACTIVE ON NEW CHANNEL
```

A switch shall preserve session identity and mission state where technically supported.

## 7. Loss of all C2

Loss of all C2 is a system state, not merely a UI alert.

The BlueSky runtime shall:

1. detect loss according to defined timeout/health criteria;
2. record the event;
3. stop issuing commands that cannot be confirmed;
4. transition responsibility to the vehicle/autopilot failsafe behaviour;
5. continue monitoring any surviving one-way data path where available;
6. attempt recovery according to the operational policy;
7. reconcile actual vehicle state after communication returns.

BlueSky shall never assume that a command was executed solely because it was transmitted before link loss.

## 8. Command integrity

Commands shall have:

- unique command/transaction identity;
- mission/package version association where relevant;
- timestamp or ordering information;
- acknowledgement state;
- timeout state;
- retry policy;
- duplicate protection.

An unconfirmed command shall remain `UNKNOWN`, not `EXECUTED`.

## 9. Telemetry integrity

The C2 layer shall expose normalized communication health and telemetry provenance to the runtime.

Data shall include sufficient metadata to distinguish:

- current data;
- stale data;
- delayed data;
- missing data;
- reconstructed/derived data.

## 10. Recovery and reconciliation

After communication recovery:

```text
LINK RECOVERED
      ↓
SESSION RECONNECT
      ↓
VEHICLE STATE READ
      ↓
MISSION STATE READ
      ↓
COMPARE WITH BLUESKY STATE
      ↓
RECONCILE
      ↓
RESUME / REPLAN / ABORT
```

BlueSky shall not blindly resend the previous command or mission without determining the actual vehicle state.

## 11. Interaction with Autopilot Adapter

C2 transports communication. The Autopilot Adapter interprets vehicle protocol semantics.

```text
C2 CHANNEL
   ↓
C2 SESSION
   ↓
AUTOPILOT ADAPTER
   ↓
VEHICLE COMMAND / TELEMETRY
```

Transport-specific details remain inside C2 drivers; autopilot-specific semantics remain inside autopilot adapters.

## 12. Multi-UAV operation

C2 state shall be maintained independently for each vehicle while exposing aggregate mission connectivity to the coordinator.

One UAV losing C2 shall not automatically imply loss of C2 for the other vehicles.

The mission coordinator shall be able to continue, modify or terminate the group mission according to the safety policy and current state.

## 13. Safety boundary

C2 does not replace vehicle failsafe logic. The autopilot/UAV remains responsible for autonomous behaviour required when communication is unavailable.

BlueSky is responsible for detecting, coordinating and recording the communication condition and for applying the approved operational policy when control is available.

## 14. Acceptance criteria

The C2 contract is complete when:

1. multiple communication transports can be represented through one interface;
2. active/standby state is explicit;
3. channel degradation and loss are detected;
4. failover is deterministic and observable;
5. command acknowledgement and uncertainty are represented;
6. complete C2 loss produces defined behaviour;
7. recovery performs state reconciliation;
8. multi-UAV C2 states are independent;
9. C2 transport and autopilot semantics remain separated;
10. all safety-critical transitions are logged and testable.

## 15. Implementation and verification

Concrete C2 drivers shall be separately versioned and tested. Verification shall cover normal operation, degradation, loss, failover, recovery, duplicate/late messages, stale telemetry and simultaneous multi-UAV communication faults.

SIL/HIL and real-UAV tests shall demonstrate that BlueSky and vehicle failsafe behaviour remain consistent under communication failures.
