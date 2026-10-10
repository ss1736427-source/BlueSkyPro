# BlueSky PRO — ArduPilot System Boundary and Responsibility Model

**Status:** ARCHITECTURE DECISION BASELINE
**Scope:** ArduPilot-based UAV integration
**Decision:** BlueSky shall integrate with ArduPilot without reproducing ArduPilot's onboard flight-control functions or unnecessarily duplicating a full ArduPilot Ground Control Station (GCS).

## 1. Why this boundary is required

ArduPilot is not a complete ground-control application. It is open-source autopilot firmware that runs on flight-controller hardware. The ArduPilot ecosystem also includes separate Ground Control Station (GCS) software such as Mission Planner and QGroundControl. The GCS is used to configure, test, tune, plan and operate the vehicle, while the flight controller executes flight-control functions onboard.

Therefore BlueSky must distinguish three different layers:

1. **ArduPilot flight-controller firmware** — onboard flight-control execution.
2. **ArduPilot GCS/configuration tooling** — setup, calibration, tuning, detailed parameter management and other vehicle-specific engineering functions.
3. **BlueSky PRO** — mission, operational planning, optimization, supervisory operation, fleet coordination, external data integration and Flight Record.

BlueSky shall not create a second implementation of layer 1 or blindly reproduce layer 2.

## 2. External reference model

ArduPilot documentation describes the autopilot as firmware running on a controller. The controller receives inputs from sensors, RC and MAVLink sources and produces outputs to motors, ESCs, servos and other actuators. ArduPilot also supports autonomous missions stored on the flight controller and executed by the onboard firmware.

The official Mission Planner documentation describes Mission Planner as a full-featured GCS that can load firmware, configure and tune vehicles, plan autonomous missions, download/analyze logs and support simulation.

MAVLink is the communication interface used by ArduPilot for GCS and companion-computer communication. ArduPilot can also route MAVLink between telemetry ports when multiple MAVLink components are present.

## 3. Functional system map

```text
                         OPERATIONAL / MISSION LEVEL
┌───────────────────────────────────────────────────────────────────┐
│                         BLUESKY PRO                               │
│                                                                   │
│  Task / Mission Definition                                        │
│  Route & Energy Optimization                                       │
│  Weather / Airspace / Terrain / Traffic                           │
│  Multi-UAV Coordination                                            │
│  Mission Validation / Release                                     │
│  Supervisory Monitoring                                            │
│  Operational Contingency / Replanning                             │
│  Flight Record / Post-flight / Corrections                        │
└──────────────────────────────┬────────────────────────────────────┘
                               │
                      operational interface
                               │
                               ▼
┌───────────────────────────────────────────────────────────────────┐
│              ARDUPILOT INTEGRATION / MAVLink                     │
│                                                                   │
│  Discovery / Identity / Capability                               │
│  Mission transfer / read-back / verification                     │
│  Telemetry normalization                                          │
│  Operational command exchange                                     │
│  Link/session handling                                            │
│  Explicit compatibility and evidence                              │
└──────────────────────────────┬────────────────────────────────────┘
                               │ MAVLink
                               ▼
                 ┌────────────────────────────┐
                 │     ARDUPILOT FIRMWARE     │
                 │   ON FLIGHT CONTROLLER     │
                 │                            │
                 │ State estimation           │
                 │ Flight modes               │
                 │ Navigation execution       │
                 │ Stabilization / control    │
                 │ Onboard failsafes           │
                 │ Mission execution          │
                 │ Sensor processing          │
                 │ Output generation          │
                 └──────────────┬─────────────┘
                                │
              ┌─────────────────┼─────────────────┐
              ▼                 ▼                 ▼
           Sensors          ESC / Motors       Servos / I/O
              │                 │                 │
              └─────────────────┴─────────────────┘
                                │
                                ▼
                              UAV
```

## 4. The GCS is a separate concern

A conventional ArduPilot deployment may also contain a GCS:

```text
             ENGINEERING / DETAILED VEHICLE SETUP
                         Mission Planner
                              │
                              │ MAVLink
                              ▼
                        ArduPilot FCS
                              │
                              ▼
                             UAV
```

BlueSky does not need to reproduce all GCS functionality. In particular, BlueSky shall not automatically become a replacement for the vehicle manufacturer's/ArduPilot engineering workstation merely because it can communicate over MAVLink.

## 5. Responsibility allocation

| Function | BlueSky | ArduPilot FCS | ArduPilot GCS / engineering tool |
|---|---|---|---|
| Operational task definition | OWNER | — | — |
| Mission optimization | OWNER | — | — |
| Weather / airspace / terrain constraints | OWNER | — | — |
| Multi-UAV mission coordination | OWNER | — | — |
| Mission semantic validation | OWNER | accepts executable representation | may assist |
| Mission transfer | ORCHESTRATES | receives/stores | can also perform |
| Mission read-back / operational verification | OWNER | source of actual onboard mission | can assist |
| Telemetry consumption | OWNER | SOURCE | displays |
| Operational monitoring | OWNER | provides state | displays |
| Supervisory contingency decision | OWNER within defined authority | executes flight response | may issue commands |
| Stabilization | — | OWNER | — |
| Low-level flight control | — | OWNER | — |
| Onboard failsafe | — | OWNER | configures/observes |
| Sensor/actuator execution | — | OWNER | — |
| Firmware installation | — | target firmware | ENGINEERING TOOL |
| Detailed vehicle configuration | only required operational subset | owns runtime behavior | PRIMARY ENGINEERING TOOL |
| Calibration / tuning | — unless a future explicit product requirement is approved | executes resulting parameters | PRIMARY ENGINEERING TOOL |
| Low-level parameter administration | not a default BlueSky responsibility | consumes parameters | PRIMARY ENGINEERING TOOL |
| Detailed log analysis tooling | Flight Record / operational analysis | generates logs | can provide detailed tooling |
| MAVLink protocol handling | through integration boundary | native | native |

## 6. What BlueSky should actually integrate

BlueSky needs a **small, purpose-built operational interface**, not the whole ArduPilot feature surface.

### Required operational functions

- discover and identify the connected vehicle/FCS;
- establish and monitor the communication session;
- obtain capability/version information;
- obtain normalized health and flight state;
- transfer a BlueSky mission into an ArduPilot-compatible representation;
- read the mission back;
- semantically verify that the onboard mission matches the released BlueSky mission;
- receive telemetry required for operational monitoring;
- issue only the operational commands required by the BlueSky authority model;
- observe/coordinate onboard failsafe and contingency states;
- acquire flight logs needed for the BlueSky Flight Record;
- retain traceability of source, timestamps, mission version and adapter/protocol version.

## 7. What BlueSky should NOT duplicate by default

The following remain outside the BlueSky operational core unless a separate product requirement explicitly establishes a need:

- firmware flashing;
- complete ArduPilot parameter editor;
- PID/controller tuning workflows;
- sensor calibration wizards;
- hardware-specific board setup;
- exhaustive ArduPilot diagnostic/configuration screens;
- direct low-level actuator control;
- reimplementation of ArduPilot flight modes;
- reimplementation of onboard navigation/stabilization;
- reimplementation of onboard failsafe algorithms.

If any such function is later required, it must be introduced as an explicitly bounded **engineering/configuration integration**, not silently added to the Universal Mission API.

## 8. Important distinction: GCS replacement vs operational integration

BlueSky may ultimately provide enough GCS-like operational functionality that an operator does not need Mission Planner during normal mission execution. That does **not** mean BlueSky should reproduce all Mission Planner engineering functionality.

The intended split is:

```text
ENGINEERING / VEHICLE COMMISSIONING
        │
        ▼
Mission Planner / vendor engineering tool
        │
        │ establishes and maintains aircraft/FCS baseline
        ▼
┌─────────────────────────────────────┐
│       ARDUPILOT FLIGHT CONTROLLER   │
└──────────────────┬──────────────────┘
                   ▲
                   │ operational mission / state / telemetry
                   │
                   ▼
             BLUE SKY PRO
        normal mission operations
```

The customer should not be required to manually integrate BlueSky, but the BlueSky product boundary must remain clean: operational integration is a product capability; full autopilot engineering/configuration is a separate concern.

## 9. Authority model

```text
                    BLUE SKY
             Mission / Operations
                      │
             supervisory authority
                      │
                      ▼
                ARDUPILOT FCS
       immediate flight-control authority
                      │
          ┌───────────┴───────────┐
          ▼                       ▼
      actuators                 failsafe

```

Loss of BlueSky connectivity must not imply that BlueSky owns stabilization or that the aircraft loses its onboard safety behavior. The onboard FCS remains responsible for its configured flight-control and failsafe behavior.

## 10. Mission lifecycle with the correct boundary

```text
BLUE SKY
  │
  ├─ create task
  ├─ build route
  ├─ optimize
  ├─ validate constraints
  ├─ release mission version
  │
  ▼
INTEGRATION BOUNDARY
  │
  ├─ identify FCS
  ├─ capability check
  ├─ translate mission
  ├─ upload
  ├─ read-back
  └─ semantic verify
  │
  ▼
ARDUPILOT
  │
  ├─ accept/store mission
  ├─ execute onboard mission
  ├─ stabilize/navigate/control
  └─ apply onboard failsafe
  │
  ▼
UAV
  │
  └─ physical flight
  │
  ▲
  │ telemetry / state / logs
  │
BLUE SKY
```

## 11. Architectural consequence for the Universal Autopilot API

The Universal Autopilot API shall be interpreted as an **operational integration contract**, not as a universal copy of every autopilot configuration API.

The API should expose only normalized capabilities needed by BlueSky's product responsibilities. Protocol-specific and engineering-specific functions remain behind adapter boundaries and are not promoted into the BlueSky core without a documented product requirement.

In particular, `parameter/configuration` functionality shall be treated as a controlled integration capability, not as a mandate for BlueSky to reproduce a complete ArduPilot configuration station.

## 12. Consequence for ArduPilot PH5 work

Before adding further ArduPilot transport/SITL implementation, the next architecture gate is to verify that the selected integration scope follows this boundary.

The ArduPilot reference adapter must demonstrate the minimum operational lifecycle needed by BlueSky. It must not grow into a second Mission Planner.

Evidence shall remain explicit:

- `SIL_FIXTURE_ONLY` for controlled fixtures;
- `SITL` only when actual ArduPilot SITL is executed;
- `HIL` only when HIL evidence exists;
- real-aircraft evidence only after actual representative-aircraft testing.

## 13. Sources reviewed

Primary ArduPilot documentation reviewed on 2026-09-16:

- ArduPilot documentation overview: https://ardupilot.org/ardupilot/
- Simple Overview of ArduPilot Operation: https://ardupilot.org/copter/docs/common-basic-operation.html
- Autopilot Inputs and Outputs: https://www.ardupilot.ardupilot.org/copter/docs/common-flight-controller-io.html
- MAVLink Interface: https://en.ardupilot.org/dev/docs/mavlink-commands.html
- MAVLink Basics: https://ardupilot.org/dev/docs/mavlink-basics.html
- MAVLink Routing in ArduPilot: https://en.ardupilot.org/dev/docs/mavlink-routing-in-ardupilot.html
- Companion Computers: https://ardupilot.org/dev/docs/companion-computers.html
- Mission Planner Overview: https://ardupilot.org/planner/docs/mission-planner-overview.html

These sources distinguish the onboard autopilot, MAVLink integration, companion/GCS communication, and Mission Planner's separate GCS/configuration role.
