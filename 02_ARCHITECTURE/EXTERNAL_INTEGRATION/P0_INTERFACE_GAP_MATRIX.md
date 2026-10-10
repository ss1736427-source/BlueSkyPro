# BlueSky PRO — P0 External Integration Gap Matrix

**Status:** WORKING BASELINE — GAP CLOSURE

## Purpose

This matrix compares the current BlueSky PRO architecture baseline with practical ground-control capabilities represented by Mission Planner and QGroundControl, and identifies the interfaces that must exist for a real operational BlueSky deployment.

## Baseline rule

BlueSky is not required to reproduce Mission Planner feature-for-feature. The comparison is used to identify operational interfaces and lifecycle functions that a complete system cannot omit.

## P0 matrix

| Domain | Practical competitor capability | BlueSky current architectural position | Gap | Required action |
|---|---|---|---|---|
| Vehicle connection | USB/serial/telemetry/network connection | C2 and vehicle state are defined conceptually | OPEN | Implement IF-AUTOPILOT connection/session lifecycle |
| Vehicle discovery | Heartbeat, system/component identity, firmware and capabilities | Vehicle state/capability concepts exist | OPEN | Implement discovery, identity and capability handshake |
| Autopilot integration | Direct ArduPilot/PX4 communication | Adapter boundary and Registry are defined | OPEN | Implement first MAVLink adapter |
| MAVLink session | Telemetry, commands, mission, parameters | MAVLink named as required interface | OPEN | Implement dialect/version/message policy and routing |
| Command control | Arm, disarm, takeoff, land, RTL, modes, mission control | Supervisory C2 and command contract are baselined | OPEN | Implement command lifecycle, ACK, retry, timeout and safe failure |
| Manual/override control | RC/joystick/manual override and mode authority | Authority model is explicit as an interface contract | OPEN | Implement pilot input, override, authority arbitration and loss behavior |
| Mission upload/download | Upload mission and read back from vehicle | Mission Graph and translation contract exist | OPEN | Implement mission translator, upload/download and integrity verification |
| Parameter management | Read/write/save/restore/compare parameters | Configuration contract and baseline exist | OPEN | Implement parameter synchronization and configuration baseline handling |
| Vehicle configuration | Firmware, frame, sensors, radio, safety, modes, power, tuning | ADMIN/configuration exists | PARTIAL | Define and implement BlueSky-owned vs engineering-tool functions |
| Calibration | Accelerometer, compass, radio, actuator/sensor calibration | Readiness/configuration concepts exist | OPEN | Define calibration interface or explicit external-tool boundary |
| Safety configuration | Failsafe, geofence, rally, operational limits | Safety architecture exists | PARTIAL | Implement reconciliation between BlueSky policy and autopilot settings |
| Pre-arm/health | Autopilot pre-arm checks and health status | Readiness Gate exists | PARTIAL | Map autopilot health/pre-arm reasons into BlueSky readiness model |
| Telemetry | Position, attitude, speed, battery, GPS, mode, health | Telemetry/UAV state concepts exist | PARTIAL | Implement normalized telemetry schema, rate and quality semantics |
| C2 | Link monitoring and loss behavior | C2 interface/framework is defined | OPEN | Implement primary/backup links, heartbeat, quality, loss/recovery state machine |
| Time synchronization | Ground/vehicle timestamps | Not explicit | OPEN | Define common timebase and timestamp quality |
| Log acquisition | TLOG and onboard DataFlash/ULog download/analysis | Flight Record exists conceptually | PARTIAL | Define source-specific log acquisition adapters |
| Simulation | SITL/HITL integration | Digital Twin/simulation architecture exists | PARTIAL | Run the same external interface stack against simulated autopilot |
| Equipment | Camera/gimbal/equipment actions and state | Equipment interface and adapter boundary are defined | OPEN | Implement Equipment command/state/data adapter |
| Video | Live video and recording | Video identified as P1 | OPEN | Define stream transport, recording, loss/recovery and mission association |
| Companion computer | MAVLink/data exchange with onboard computer | Concept identified | OPEN | Define onboard data/command interface |
| Peripheral bus | DroneCAN/UAVCAN and other vehicle buses | Not formalized as interface layer | OPEN | Define peripheral adapter strategy and diagnostics boundary |
| Airspace/ATM | External airspace data and operational submission | Airspace/ATM/FPL identified | OPEN | Define source/API, submission, response, rejection and correction lifecycle |

## Minimum P0 operational path

```text
Aircraft registered
    ↓
Connection established
    ↓
Vehicle discovery / heartbeat
    ↓
Autopilot + firmware + capabilities identified
    ↓
Configuration synchronized
    ↓
Health / pre-arm state verified
    ↓
BlueSky mission validated
    ↓
Mission translated for vehicle
    ↓
Mission uploaded
    ↓
Mission read-back / integrity verified
    ↓
C2 + pilot authority established
    ↓
ARM / START command
    ↓
Flight execution
    ↓
Telemetry + C2 + command ACK + health
    ↓
Manual override / contingency / RTL / LAND as required
    ↓
Mission complete
    ↓
Onboard logs acquired
    ↓
Flight Record closed
```

## Current implementation status

The following P0 interface boundaries are already defined/baselined and shall not be recreated:

1. `IF-AUTOPILOT` — vehicle connection, identification and adapter boundary.
2. `IF-MAVLINK` — MAVLink session, message set, dialect/version policy and routing.
3. `IF-COMMAND` — command lifecycle, acknowledgement and failure semantics.
4. `IF-MISSION` — mission translation, upload/download and verification.
5. `IF-PARAMETER` — parameter synchronization and configuration baseline.
6. `IF-C2` — command/control transport and loss/recovery behavior.
7. `IF-VEHICLE-STATE` — normalized state/health model.
8. `IF-SAFETY-CONFIG` — reconciliation with autopilot failsafe/geofence/rally/limits.
9. `IF-AIRSPACE-ATM` — operational airspace/FPL/authorization exchange.
10. `IF-PILOT-CONTROL` — RC/joystick/manual override and authority arbitration.
11. `IF-LOG` — onboard/ground log acquisition and Flight Record mapping.

The current deterministic implementation path is to close the implementation gaps under these existing contracts, beginning with the adapter registry and universal Vehicle/Equipment integration boundaries.

## Non-goal

Do not clone Mission Planner's UI or expose raw autopilot complexity to the normal pilot workflow. BlueSky should absorb complexity through adapters, configuration baselines, validation and automation while retaining an engineering/maintenance path for deep diagnostics.

## External evidence

Mission Planner explicitly separates CONNECT, DATA, PLAN, SETUP, CONFIG and SIMULATION and supports firmware loading, vehicle configuration, autonomous mission planning, logs and live video. citeturn1search0turn1search6

Its setup path includes mandatory accelerometer/radio/servo configuration, DroneCAN/UAVCAN access, MAVLink inspection, signing and MAVLink forwarding. citeturn1search8

Mission Planner handles both onboard DataFlash logs and ground-station telemetry logs, including download, analysis and replay. citeturn1search1turn1search2

Mission Planner SITL can be controlled and configured through the same operational application used for mission setup and parameter changes. citeturn1search3

MAVLink command, mission and parameter functions must therefore be treated as explicit protocols with acknowledgement, retry and synchronization semantics, not as a generic data transport.
