# BlueSky PRO — C2 / Connectivity System Specification

**Status:** WORKING BASELINE
**Position in lifecycle:** PHASE 5 — C2 / Connectivity

## 1. Purpose

Define the complete command-and-control connectivity layer between BlueSky PRO, ground infrastructure and UAV/autopilot systems, independent of a particular radio, modem, network or transport.

## 2. Architectural rule

BlueSky mission logic shall not depend directly on a communication transport. All C2 paths terminate at the C2 abstraction layer.

```text
BLUE SKY CORE
     │
 C2 SERVICE
     │
 LINK MANAGER
     │
 ┌───┼──────────────┐
 │   │              │
RF  IP/Cellular   Local/Other
 │   │              │
 └───┼──────────────┘
     │
Transport / Protocol Adapter
     │
Autopilot / Companion / UAV
```

## 3. C2 channels

The implementation shall be able to represent multiple concurrent links where hardware and operating conditions permit:

- primary RF/data link;
- secondary/backup RF link;
- cellular/IP;
- local wired/service link;
- relay/mesh/network path;
- OEM-specific transport.

The architecture does not assume that every UAV has all channels.

## 4. Link Manager

The Link Manager is responsible for:

- discovery and registration of links;
- link identity and configuration;
- availability;
- latency;
- packet loss;
- throughput/bandwidth;
- signal/quality indicators;
- health state;
- priority;
- primary/backup role;
- failover decision;
- recovery decision;
- event logging.

## 5. Link state machine

```text
OFFLINE
   ↓
CONNECTING
   ↓
CONNECTED
   ↓
DEGRADED
   ↓
LOST
   ↓
RECOVERING
   └────────→ CONNECTED
```

Transitions shall be event-driven and recorded.

## 6. Failover

Failover shall be policy-driven rather than hard-coded for one radio technology.

Example:

```text
PRIMARY HEALTHY
      ↓
PRIMARY DEGRADED
      ↓
BACKUP QUALIFIED?
   ┌──YES──→ SWITCH
   │            ↓
   │       VERIFY CONTROL
   │            ↓
   │         ACTIVE
   │
   └──NO──→ CONTINGENCY POLICY
```

The system shall distinguish:

- C2 loss;
- telemetry loss;
- video loss;
- payload-control loss;
- total external communication loss.

A video failure shall not be represented as total C2 failure.

## 7. Recovery

When a failed link returns, BlueSky shall not switch back blindly. Recovery shall consider:

- link quality;
- stability/time hysteresis;
- current mission state;
- safety state;
- active control authority;
- operator policy.

The transition back to a preferred link shall be explicitly acknowledged and logged.

## 8. Command path

```text
Operator / Mission Engine
          ↓
Command validation
          ↓
C2 Service
          ↓
Link selection
          ↓
Transport adapter
          ↓
Autopilot adapter
          ↓
UAV/FCS
          ↓
ACK / state change
          ↓
BlueSky
```

Commands shall have a tracked lifecycle:

`CREATED → VALIDATED → SENT → ACKNOWLEDGED / REJECTED / TIMEOUT / CANCELLED`.

No command shall be considered successful merely because bytes were transmitted.

## 9. Telemetry path

Telemetry shall be normalized independently of transport:

```text
UAV/FCS
   ↓
Transport
   ↓
Protocol adapter
   ↓
C2 Service
   ↓
Normalized Vehicle State
   ↓
Mission / Safety / UI / Flight Record
```

The source, timestamp, quality and communication state shall remain available to downstream services.

## 10. Priority

C2 traffic shall support classes appropriate to the operation, at minimum:

1. safety/control;
2. vehicle state/health;
3. mission management;
4. payload control;
5. video/data;
6. bulk transfer/logs.

Congestion shall not silently discard safety-critical traffic in favor of lower-priority data.

## 11. Loss-of-communication behavior

BlueSky shall supervise loss of communication but shall not replace the onboard aircraft failsafe logic.

```text
C2 LOST
   ↓
Detect + timestamp
   ↓
Determine scope of loss
   ↓
Attempt approved recovery/failover
   ↓
If unavailable → onboard contingency remains authoritative
   ↓
BlueSky continues monitoring when data returns
```

The exact aircraft action on complete C2 loss is defined by the verified vehicle/FCS profile and operational configuration.

## 12. Security

Each transport shall define its applicable:

- authentication;
- message integrity;
- encryption;
- key management;
- identity/authorization;
- replay protection where required;
- audit requirements.

For MAVLink-based integrations, protocol-specific security features shall be represented separately from transport security.

## 13. Multi-UAV

Each vehicle has an independent C2 session and link state. The Fleet/HUB layer aggregates these sessions while preserving per-vehicle identity.

```text
              BLUE SKY C2
          ┌──────┼──────┐
          ↓      ↓      ↓
        UAV-01 UAV-02 UAV-03
        links   links   links
```

A failure on one vehicle shall not automatically invalidate the C2 state of another vehicle.

## 14. Offline / intermittent connectivity

BlueSky shall support continued local operation when external network access is unavailable, subject to the capabilities and safety authority of the onboard UAV/FCS.

The local system shall preserve mission state and queued non-time-critical synchronization data. Reconnection shall trigger controlled synchronization rather than indiscriminate overwrite.

## 15. Flight Record

The following shall be recorded per vehicle:

- link transitions;
- link quality;
- failovers;
- recoveries;
- command state;
- acknowledgements/results;
- communication outages;
- relevant security events;
- synchronization events.

## 16. Verification

The C2 layer shall be tested in:

- unit tests;
- integration tests;
- SIL/SITL;
- HIL;
- bench/ground tests;
- real UAV tests.

Mandatory scenarios include:

1. normal primary link;
2. primary degradation;
3. primary loss;
4. successful backup takeover;
5. failed backup takeover;
6. primary recovery;
7. telemetry-only loss;
8. video-only loss;
9. command timeout/rejection;
10. total communication loss;
11. reconnect and state synchronization;
12. simultaneous operation of multiple UAVs.

## 17. Gate

PHASE 5 is complete only when a representative BlueSky vehicle can maintain, lose, fail over, recover and resume C2 operation through the supported connectivity configuration, with command/telemetry state tracking and reproducible test evidence.
