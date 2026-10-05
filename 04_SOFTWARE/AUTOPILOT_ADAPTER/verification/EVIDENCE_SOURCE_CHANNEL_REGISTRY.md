# BlueSky PRO — Evidence Source & Channel Registry

**Status:** IMPLEMENTATION BASELINE — P0

## Purpose

This registry defines the evidence-bearing source channels that the Verification & Evidence Framework can collect from. It is broader than communication: any system function, interface, equipment, environment, measurement, event, or control source that may later be required to support verification or certification evidence is represented here.

The registry is not a claim that every listed source is already connected to real hardware. Prototype sources may be synthetic, replayed, bench, or simulated. Physical verification status is established only by an approved verification procedure.

## Common chain

`Requirement → Verification Method → Test Case → Configuration → Execution → Source Channel → Raw Data → Analysis → Result → Evidence Package → Certification Reference`

## Source-channel coverage

| ID | Source channel | What it provides | Typical evidence | Prototype source | Physical source later |
|---|---|---|---|---|---|
| SRC-01 | C2_LINK | command-and-control facts | latency, loss, signal quality, command/ACK, link loss/recovery | demo C2 | radio/modem/network |
| SRC-02 | AUTOPILOT_TELEMETRY | autopilot state and telemetry | mode, position, attitude, status, errors | demo telemetry | real autopilot |
| SRC-03 | FLIGHT_CONTROL | flight-control execution facts | commands, state transitions, commanded/actual state | demo controller | autopilot/control system |
| SRC-04 | MISSION | mission definition and execution | mission version, waypoints, states, deviations, completion | demo mission | flight/mission subsystem |
| SRC-05 | EQUIPMENT | installed equipment facts | identity, version, status, health, measured values, commands/responses | demo equipment | actual installed equipment |
| SRC-06 | GNSS_RTK_NTRIP | navigation/reference facts | position, altitude, fix, quality, corrections | replay/demo | GNSS/RTK/NTRIP sources |
| SRC-07 | WEATHER | environmental conditions | wind, temperature, pressure, humidity and required conditions | demo/replay | weather station / approved source |
| SRC-08 | ENERGY_POWER | energy and electrical state | voltage, current, power, battery state, temperature, reserve | demo power | BMS/power sensors |
| SRC-09 | PROPULSION | propulsion state | RPM, command, temperature, pressure where applicable, power, start/stop | demo propulsion | engine/motor controller/sensors |
| SRC-10 | PERFORMANCE_MODEL | calculated performance quantities | endurance, range, energy model, predicted/observed comparison | calculation/replay | validated model + flight data |
| SRC-11 | AIRSPACE_GEOFENCE | restriction and geographic facts | boundaries, position, clearance, warning/rejection events | demo map data | approved airspace/restriction sources |
| SRC-12 | TIME_SYNCHRONIZATION | time-reference facts | source time, receipt time, offset, synchronization state | system clock | GNSS/NTP/PTP/approved reference |
| SRC-13 | DATA_RECORDING_REPLAY | recording and replay facts | completeness, ordering, replay, missing-data detection, integrity | recorded fixture | operational log system |
| SRC-14 | BLUESKY_EVENT | internal BlueSky facts | accepted command, state change, warning, calculation event | prototype event bus | operational BlueSky |
| SRC-15 | OPERATOR_EVENT | factual human observations/actions | start/stop, acknowledgement, anomaly note, inspection result | operator input | controlled test record |
| SRC-16 | HMI | human-machine interface behaviour | displayed state, warning, command acceptance, screen evidence | prototype UI | operational HMI |
| SRC-17 | EXTERNAL_INTEGRATION | external-system exchange | input/output messages, ACK, rejection, timeout, recovery | fixture/mock | actual external system |
| SRC-18 | SECURITY_ACCESS | authentication/authorization facts | login, role decision, denied action, security event, audit record | prototype security | operational security subsystem |
| SRC-19 | FAULT_RECOVERY | failure and recovery facts | fault trigger, detection, response, timing, recovery, final state | injected/demo fault | controlled physical/operational test |
| SRC-20 | MEASUREMENT_INSTRUMENT | independent measurement facts | calibrated instrument readings and measurement metadata | fixture | calibrated test equipment |

## Rule: complete evidence coverage

The evidence layer must not be designed around one preferred channel. A new source is added when a requirement, verification method, test procedure, engineering analysis, safety case, or certification evidence package needs an objective fact that is not already represented.

Each source must have:

1. a stable source identifier;
2. a Source Adapter (адаптер источника);
3. timestamp and source provenance;
4. parameter/value/unit representation;
5. quality/status where available;
6. configuration linkage;
7. test/flight linkage where applicable;
8. raw-data preservation;
9. integrity information;
10. export linkage to the evidence package.

## Rule: channels are independent but combinable

A single test may collect from several channels simultaneously. For example, a loss-of-link test can combine `C2_LINK`, `AUTOPILOT_TELEMETRY`, `FLIGHT_CONTROL`, `TIME_SYNCHRONIZATION`, `BLUESKY_EVENT`, `OPERATOR_EVENT` and `MEASUREMENT_INSTRUMENT` records under one `test_run_id`.

No source channel owns the verification result. Sources provide facts; the approved verification method defines how those facts are evaluated.

## Rule: prototype versus certification evidence

The same collection mechanism is used in the prototype and in later physical tests, but the data class remains explicit:

- `DEMONSTRATION` — demonstration only;
- `DEVELOPMENT` — engineering/development data;
- `REPLAY` — reproduced from recorded data;
- `BENCH` — bench-test data;
- `PHYSICAL_TEST` — data from an executed physical test;
- `APPROVED_EVIDENCE` — evidence accepted by the responsible verification/certification process.

Only the responsible verification/certification process can promote a record to accepted evidence.

## Intended export

All source channels feed the same evidence package structure:

```text
TEST_RUN_ID/
  record.json
  events.jsonl
  processed.csv
  evidence/
  manifest.json
  manifest.sha256
  report.md
```

The certification export uses the identifiers to reference the original data rather than manually retyping measured values.
