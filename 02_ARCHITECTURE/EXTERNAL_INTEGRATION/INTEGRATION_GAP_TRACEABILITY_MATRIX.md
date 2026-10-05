# BlueSky PRO — Integration Gap & Traceability Matrix

**Status:** WORKING BASELINE
**Purpose:** control completeness of universal external integration and operational readiness.

## 1. Rule

Every externally visible operational capability must have a defined:

`BlueSky capability → external object/interface → protocol/transport → adapter → vehicle/system → verification → operational evidence`

The customer receives the finished integration; customer-specific integration development is not the normal operating model.

## 2. Coverage matrix

| Domain | BlueSky requirement | Interface / protocol class | Adapter required | Verification | Priority |
|---|---|---|---|---|---|
| Autopilot | discover FCS, firmware, vehicle type | MAVLink / vendor API | Yes | connect + capability test | P0 |
| Autopilot | telemetry/state | MAVLink / vendor telemetry | Yes | state consistency + replay | P0 |
| Autopilot | commands + ACK | MAVLink Command Protocol / native | Yes | command/ACK/failure tests | P0 |
| Autopilot | mission upload/download | MAVLink Mission Protocol / native | Yes | upload + read-back + semantic compare | P0 |
| Autopilot | parameters | MAVLink Parameter Protocol / native | Yes | snapshot/write/read-back | P0 |
| Autopilot | safety/failsafe | vehicle-specific | Yes | SITL/HIL/real UAV | P0 |
| Autopilot | logs | MAVLink FTP/log protocol / native | Yes | acquisition + integrity + replay | P0 |
| C2 | multiple links | RF/IP/serial/other transport | Yes | link-loss/recovery/failover | P0 |
| C2 | degraded link | link telemetry | Yes | latency/bandwidth/loss tests | P0 |
| C2 | failover | transport abstraction | Yes | automated scenario test | P0 |
| C2 | secure communications | protocol/transport security | Yes | security verification | P0 |
| Vehicle | aircraft profile | normalized profile | Yes | compatibility validation | P0 |
| Vehicle | capability model | profile + discovery | Yes | capability regression | P0 |
| Vehicle | performance limits | aircraft data/config | Profile | planning validation | P0 |
| Payload | camera/control | MAVLink/native/SDK | Yes | command + telemetry test | P1 |
| Payload | gimbal | MAVLink/native/SDK | Yes | angle/mode/command test | P1 |
| Payload | LiDAR/sensors | vendor/API/CAN/Ethernet | Yes | data integrity test | P1 |
| Video | live stream | RTP/RTSP/WebRTC/vendor | Yes | latency/continuity test | P1 |
| GNSS/RTK | position/navigation | GNSS/RTK/NTRIP/vendor | Yes | accuracy/quality/fallback | P1 |
| Peripheral | onboard bus | DroneCAN/UAVCAN/serial/Ethernet | Yes | device discovery + data test | P1 |
| Companion | onboard compute | API/message bus/IPC | Yes | command/data integration | P1 |
| GIS/Terrain | maps/DEM/obstacles | provider/API/file | Yes | source/version/freshness | P1 |
| Weather | forecast/actual | API/data service | Yes | freshness/quality + replay | P1 |
| Traffic | ADS-B/traffic | MAVLink/ASTM/vendor/API | Yes | ingestion + stale-data tests | P1 |
| Airspace | restrictions/NOTAM/AIP | aeronautical data/API | Yes | data validation + freshness | P0/P1 |
| ATM/FPL | flight plan submission | jurisdiction-specific API/exchange | Yes | submission/status/rejection loop | P0 |
| Authorization | approval state | jurisdiction-specific system | Yes | positive authorization gate | P0 |
| Reporting | closure/occurrence | jurisdiction-specific | Yes | end-to-end record test | P1 |
| Logs | unified Flight Record | normalized event/data model | Yes | complete lifecycle replay | P0 |
| Security | identity/access/audit | PKI/auth/signing/vendor | Yes | security test | P0 |
| Maintenance | aircraft/battery/resources | maintenance API/model | Yes | lifecycle consistency | P1 |
| Enterprise | customer data/API | REST/API/file/event | Yes | contract tests | P2 |

## 3. Autopilot minimum capability contract

The first reference adapters are ArduPilot and PX4. MAVLink is the first common protocol layer, but protocol compatibility alone is not sufficient: supported mission commands and parameters can differ by vehicle/FCS. ArduPilot explicitly documents that only a subset of MAVLink mission commands is implemented per vehicle type, while PX4 exposes multiple MAVLink microservices including Command, Mission, Parameter, Gimbal, Payload, Time Synchronization, Traffic Management and others. citeturn0search0turn0search4

Therefore each adapter must implement capability discovery and explicit unsupported-state handling.

### Minimum contract

`Identity → Version → Capabilities → Health → Configuration → Mission → Command → Telemetry → Safety → Logs`

## 4. C2 minimum contract

`Discover → Connect → Authenticate → Establish primary → Monitor → Degrade → Failover → Recover → Restore preferred path`

Loss of video or payload connectivity must be distinguishable from loss of C2. Complete communication loss must be distinguishable from loss of a single service.

## 5. Mission integrity

A successful network transfer does not equal mission verification.

```text
BlueSky Mission
      ↓
Capability check
      ↓
Vehicle-specific translation
      ↓
Upload
      ↓
Read-back
      ↓
Semantic comparison
      ↓
Verified mission identity/version
```

ArduPilot documents that mission commands are a vehicle-dependent subset of MAVLink commands; unsupported commands may be dropped. This makes read-back and semantic verification a mandatory BlueSky control. citeturn0search0turn0search5

## 6. Operational readiness evidence

A vehicle is `READY FOR FLIGHT` only when mandatory gates pass:

- compatible vehicle/FCS/firmware;
- verified configuration baseline;
- required C2 available;
- autopilot health/pre-arm status acceptable;
- mission validated and verified on vehicle;
- applicable airspace/authorization status valid;
- weather/terrain/traffic constraints acceptable;
- payload ready;
- required integration verification current.

Each gate produces a machine-readable result and reason.

## 7. Validation chain

```text
Requirement
  ↓
Unit test
  ↓
Integration test
  ↓
SIL/SITL
  ↓
HIL
  ↓
Bench/Ground
  ↓
Real UAV
  ↓
Flight Record / Evidence
  ↓
Release
```

Critical scenarios shall include normal mission execution, command rejection, telemetry loss, C2 loss, link failover/recovery, stale external data, autopilot failsafe activation, payload failure, mission mismatch and recovery/RTL/landing.

## 8. Definition of Done — integration

An integration is **DONE** only when all of the following are true:

1. Protocol/SDK/API contract documented.
2. Adapter implemented.
3. Vehicle/system profile implemented.
4. Capability mapping complete.
5. Mission semantics mapped.
6. Commands and ACK/result states mapped.
7. Telemetry normalized.
8. Configuration baseline controlled.
9. Failure and timeout behavior defined.
10. Security requirements defined.
11. Logs and Flight Record mapped.
12. SIL/SITL tests pass.
13. HIL/bench tests pass where applicable.
14. Real-UAV acceptance tests pass where applicable.
15. Regression suite registered.
16. Known limitations explicitly recorded.

## 9. Current product risk

The principal risk is not the number of supported protocols. It is incomplete semantic mapping between BlueSky's mission/safety model and the capabilities actually implemented by a particular FCS, vehicle and payload.

Therefore BlueSky shall prioritize **verified adapters and capability-driven integration** over superficial protocol compatibility.
