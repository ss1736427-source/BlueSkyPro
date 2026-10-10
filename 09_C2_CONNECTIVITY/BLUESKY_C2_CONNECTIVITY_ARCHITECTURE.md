# BlueSky PRO — C2 / Connectivity Architecture

**Status:** ACCEPTED — architecture baseline

## 1. Purpose

BlueSky PRO must operate as a universal mission-planning and control system above heterogeneous UAV autopilots and communication infrastructures. C2/Connectivity is therefore a separate integration layer between BlueSky mission services and the vehicle/autopilot, with support for multiple communication paths, loss/recovery, routing, health monitoring and safe degradation.

The design principle is:

> BlueSky plans and coordinates the mission; the autopilot executes vehicle-level flight control and onboard failsafe behavior; the C2 layer provides the resilient communication path and exposes its actual state to BlueSky.

## 2. Logical architecture

```text
                         BLUESKY PRO
                              │
                 Mission / Fleet / Planning Core
                              │
                    C2 ABSTRACTION LAYER
                              │
             ┌────────────────┼────────────────┐
             │                │                │
        C2 Channel A      C2 Channel B     C2 Channel C
        primary link     backup link       optional link
             │                │                │
             └────────────────┼────────────────┘
                              │
                       C2 ROUTER / MANAGER
                              │
                  AUTOPILOT ADAPTER LAYER
                              │
              ┌───────────────┼────────────────┐
              │               │                │
          ArduPilot          PX4             OEM
              │               │                │
           MAVLink         MAVLink       OEM protocol
              │               │                │
             UAV-1           UAV-2            UAV-3
```

The BlueSky core must not contain autopilot-specific logic. Protocol-specific behavior belongs in adapters/connectors.

## 3. Communication abstraction

C2 must expose a common internal interface independent of physical transport and autopilot protocol.

At minimum the abstraction covers:

- connection establishment;
- authentication/identity;
- heartbeat/liveness;
- telemetry reception;
- command transmission;
- acknowledgement and timeout handling;
- mission upload/download;
- parameter exchange where supported;
- status and health;
- link quality;
- channel availability;
- time synchronization;
- event/error reporting;
- reconnect and recovery state.

MAVLink is a primary interoperability path because it is used by both ArduPilot and PX4 for GCS/vehicle communication and defines routing, commands, telemetry and multiple microservices. MAVLink 2 also provides message signing for source authentication. citeturn0search5turn0search7turn0search0

## 4. Multiple physical links

The architecture must not assume that one radio equals one C2 connection.

Possible transports include, depending on UAV and operational environment:

- serial/UART telemetry radio;
- Wi-Fi;
- Ethernet/IP;
- cellular IP;
- satellite/IP;
- long-range radio/modem;
- relay/mesh/networked vehicle link;
- OEM-specific transport.

PX4 explicitly supports MAVLink over serial and UDP/network connections and documents multiple telemetry radio/modem configurations. citeturn0search4turn0search14turn0search15

BlueSky must treat transport and protocol as separate dimensions:

```text
TRANSPORT
UART / UDP / IP / SAT / RADIO / CELLULAR / OEM
             ↓
       C2 TRANSPORT ADAPTER
             ↓
       PROTOCOL ADAPTER
             ↓
       BLUE SKY C2 API
```

## 5. Link manager

A dedicated C2 Link Manager evaluates available paths continuously.

It tracks at least:

- link up/down;
- latency;
- packet loss;
- jitter;
- bandwidth;
- signal/link quality where available;
- heartbeat age;
- command acknowledgement state;
- data freshness;
- authentication state.

The manager can select the preferred available channel according to configured policy and actual link health.

The policy must be deterministic and auditable. AI may recommend or optimise non-safety-critical communication policies, but it must not bypass hard C2 safety rules.

## 6. Loss of C2

C2 loss is not equivalent to loss of vehicle control. The autopilot must retain responsibility for immediate onboard failsafe behavior.

BlueSky detects and classifies the loss, records the event, and applies the mission-level response allowed by the vehicle/autopilot and operational configuration.

```text
C2 LOSS
   ↓
DETECT
   ↓
CLASSIFY
   ├── transient
   ├── degraded
   └── confirmed loss
   ↓
TRY RECOVERY / ALTERNATE LINK
   ↓
IF RECOVERED → RESUME / RECONCILE
   ↓
IF NOT RECOVERED → AUTOPILOT FAILSAFE / MISSION CONTINGENCY
```

ArduPilot and PX4 both implement their own data-link/GCS failsafe mechanisms. BlueSky must integrate with those mechanisms rather than duplicate or conflict with them. citeturn0search1turn0search3turn0search11turn0search16

## 7. Recovery and reconnection

Reconnection is a controlled state transition, not simply reopening a socket.

After reconnection BlueSky must reconcile:

- vehicle identity;
- autopilot state;
- mission version;
- current mission item;
- vehicle position;
- home/reference point;
- battery/energy state;
- active failsafe state;
- payload state;
- pending commands;
- telemetry freshness;
- time synchronization.

MAVLink routing guidance explicitly notes that after an autopilot/system reboot routing information may need to be cleared and parameters/home position re-fetched. BlueSky therefore requires an explicit post-reconnect synchronization procedure. citeturn0search5

## 8. Command safety

Commands sent from BlueSky must pass a command gate before transmission.

```text
BLUE SKY COMMAND
       ↓
TARGET / IDENTITY CHECK
       ↓
MISSION STATE CHECK
       ↓
SAFETY / AUTHORIZATION CHECK
       ↓
PROTOCOL ADAPTER
       ↓
AUTOPILOT
       ↓
ACK / RESULT
       ↓
BLUE SKY LOG
```

Commands that affect flight safety, mode, arming, navigation or mission execution require explicit acknowledgement semantics where supported.

MAVLink Command Protocol provides acknowledgement/retransmission semantics for commands that require them; production MAVLink deployments should also use message signing. citeturn0search7turn0search0

## 9. Autopilot responsibility boundary

BlueSky must not attempt to replace the autopilot's real-time stabilization/control loop.

### BlueSky responsibilities

- mission planning;
- route and trajectory generation;
- fleet coordination;
- mission-level replanning;
- C2 management;
- payload mission coordination;
- monitoring;
- energy/resource prediction;
- operational validation;
- mission state and logging.

### Autopilot responsibilities

- real-time flight control;
- actuator control;
- state estimation;
- onboard navigation/control loops;
- immediate vehicle-level failsafes;
- vehicle-specific emergency behavior;
- execution of accepted mission/setpoints.

The exact boundary is adapter-specific and must be documented for every supported autopilot family.

## 10. Universal autopilot integration

The first-class integration target is a common BlueSky Autopilot Interface with adapters for:

- ArduPilot;
- PX4;
- OEM/autopilot-specific implementations.

The adapter must translate between the common BlueSky mission/state model and the capabilities actually supported by the target autopilot.

Unsupported capabilities must be reported explicitly rather than silently approximated.

```text
BlueSky Mission Model
        │
        ▼
Autopilot Capability Query
        │
        ├── supported
        ├── partially supported
        └── unsupported
        │
        ▼
Adapter Mapping
        │
        ▼
Autopilot Mission / Commands
```

## 11. Multi-UAV C2

For group missions each UAV has an independent identity, link state, autopilot adapter and telemetry stream while BlueSky maintains a mission-level coordination state.

```text
                 GROUP MISSION
                       │
                   BLUE SKY
                       │
             ┌─────────┼─────────┐
             ↓         ↓         ↓
           UAV-01    UAV-02    UAV-03
             │         │         │
          C2 state   C2 state   C2 state
             │         │         │
          adapter    adapter    adapter
             └─────────┼─────────┘
                       ↓
                 COORDINATION
```

Loss of one vehicle link must not automatically imply loss of the entire mission. BlueSky must determine whether the remaining fleet can continue, whether the affected UAV follows its onboard contingency, and whether mission replanning is required.

## 12. Payload and companion components

C2 architecture must support communication not only with the flight controller but, where applicable, with companion computers, cameras, gimbals and other mission components.

MAVLink documentation explicitly models systems as containing multiple components such as autopilot, camera and servos and provides addressing/routing by system and component IDs. PX4 also uses MAVLink for communication with companion computers and MAVLink-enabled cameras. citeturn0search5turn0search7

Therefore BlueSky must maintain separate identities/capability models for vehicle and payload components where the protocol supports them.

## 13. Security

C2 is a safety and security boundary.

Required architectural capabilities include:

- authenticated vehicle identity;
- authenticated command origin;
- message integrity;
- replay protection where supported;
- key management;
- secure configuration;
- authorization by operator role;
- audit logging;
- secure update path;
- protection against accidental or unauthorized command injection.

MAVLink 2 message signing is available for authentication/integrity of MAVLink messages; PX4 warns that unsigned MAVLink messages are unauthenticated and can permit dangerous commands. citeturn0search0turn0search7

## 14. C2 state machine

```text
DISCONNECTED
     ↓
CONNECTING
     ↓
AUTHENTICATING
     ↓
SYNCHRONIZING
     ↓
CONNECTED
     ↓
DEGRADED
     ↓
RECOVERING
     ↓
CONNECTED

CONNECTED → C2 LOST → FAILSAFE / CONTINGENCY
                         ↓
                     RECOVERING
                         ↓
                     SYNCHRONIZING
```

Every transition must be logged with timestamp, vehicle ID, active channel, reason and resulting state.

## 15. Performance principle

C2 must not become a bottleneck for mission preparation or execution.

- telemetry processing is asynchronous;
- independent vehicle links are processed concurrently;
- link switching must not block unrelated UAVs;
- mission preparation must not wait for low-priority telemetry;
- stale data must be explicitly marked;
- only affected mission segments should be recalculated after a communication/state change.

## 16. Required integration test scope

Every autopilot adapter must be validated in:

1. protocol simulation;
2. SITL;
3. HIL where applicable;
4. hardware bench test;
5. real UAV flight test;
6. loss/recovery scenarios;
7. channel switching scenarios;
8. delayed/lost/duplicated messages;
9. autopilot reboot/reconnect;
10. mission upload interruption;
11. command acknowledgement failure;
12. low-battery/C2 contingency interaction;
13. multi-UAV communication degradation.

PX4 documents SITL/HITL testing of failsafe behavior; BlueSky should incorporate equivalent adapter-level verification into its operational validation chain. citeturn0search16

## 17. Design rule

> **BlueSky is not tied to one radio, one data link, one protocol, one autopilot or one UAV manufacturer.**
>
> **The integration layer absorbs heterogeneity; the BlueSky Core operates on a common mission, vehicle, payload and state model.**

This principle is mandatory for the BlueSky universal-fleet strategy and expansion to additional autopilots, UAV platforms and external C2 infrastructures.
