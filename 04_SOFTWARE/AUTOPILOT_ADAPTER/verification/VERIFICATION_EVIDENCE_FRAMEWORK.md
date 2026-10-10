# BlueSky PRO — Verification & Evidence Framework

**Status:** IMPLEMENTATION BASELINE — P0
**Purpose:** prepare the project to conduct long, evidence-driven verification campaigns without redesigning the system when physical testing begins.

## 1. Scope

The framework covers collection, storage, integrity protection, analysis inputs, export and traceability of verification data. It is designed for bench, SIL/HIL and real-UAV testing, while allowing the present prototype to operate with synthetic/demo data only.

No physical test result is created by the prototype. Until an actual test is performed, the verification result remains `PENDING` or `NOT_PERFORMED`.

The framework is not limited to the C2 communication channel. Every function, interface, source, operational condition or technical characteristic that may require objective verification evidence shall be assigned an evidence chain.

The complete domain coverage is maintained in `EVIDENCE_CHAIN_CATALOG.md`.

## 2. Controlled chain

`Requirement → Test Method → Test Case → Configuration → Execution → Source Data → Raw Data → Processed Data → Result → Evidence Package → Certification Reference`

This chain is common to all evidence domains. Domain-specific source adapters and parameters plug into the same controlled storage and export mechanism.

## 3. Test identity

Every execution receives a unique `test_run_id`. The record shall retain:

- requirement IDs;
- test method and revision;
- test case and revision;
- system/build version;
- vehicle identity;
- equipment configuration;
- autopilot/firmware/protocol versions;
- measurement equipment, where applicable;
- operator/organization fields required by the approved procedure;
- start/end time;
- location and environmental conditions, when applicable;
- execution status;
- evidence domain identifier(s).

## 4. Data layers

### 4.1 Raw Data — исходные данные

Values exactly as received from a source, with timestamp, parameter name, value, unit, source and optional quality/status.

Raw data is append-only for an active record and is never replaced by calculated values.

### 4.2 Processed Data — обработанные данные

Derived values such as minimum, maximum, average, duration, packet-loss statistics or other approved calculations. Each derived result references its source data and calculation definition.

### 4.3 Evidence — доказательства

Files and records that substantiate the result: raw logs, exported measurements, configuration snapshots, screen/video captures, reports and integrity manifest.

## 5. Prototype storage

The prototype uses a portable evidence directory:

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

`events.jsonl` is the primary append-only event stream. JSON Lines (JSONL — один JSON-объект на строку) allows long-running collection without keeping the whole log in memory.

## 6. Event minimum

Each event contains:

- `timestamp_ms`;
- `parameter`;
- `value`;
- `unit`;
- `source`;
- optional `quality`;
- optional `context`.

The model is intentionally extensible so new telemetry or measurement parameters do not require a redesign of the evidence container.

## 7. Evidence domains and source integration

The evidence layer shall provide chains for all domains listed in the catalog, not only communication:

- `EC-01` — C2 Communication Channel;
- `EC-02` — Autopilot Interface;
- `EC-03` — Navigation / GNSS / RTK / NTRIP;
- `EC-04` — Flight Control / Flight State;
- `EC-05` — Mission / Route Execution;
- `EC-06` — Installed Equipment;
- `EC-07` — Energy / Battery / Power;
- `EC-08` — Propulsion;
- `EC-09` — Aerodynamic / Performance Model;
- `EC-10` — Weather / Environmental Conditions;
- `EC-11` — Safety / Failsafe / Loss of Link;
- `EC-12` — Geographical Restrictions / Airspace / Geofence;
- `EC-13` — Time / Synchronisation;
- `EC-14` — Data Recording / Replay;
- `EC-15` — BlueSky Internal Functions / HMI;
- `EC-16` — External Interfaces / Integrations;
- `EC-17` — Communications Other Than C2;
- `EC-18` — Software Configuration / Version Control;
- `EC-19` — Security / Access Control;
- `EC-20` — Faults / Anomalies / Recovery.

**Source Adapter (адаптер источника)** translates each source into the common event model. The current supported source categories include:

- `AUTOPILOT_TELEMETRY` — telemetry from the autopilot;
- `C2_LINK` — command-and-control channel measurements;
- `EQUIPMENT` — installed equipment parameters and equipment logs;
- `GNSS_RTK_NTRIP` — navigation/reference data;
- `WEATHER` — weather observations;
- `MEASUREMENT_INSTRUMENT` — external calibrated instruments;
- `BLUESKY_EVENT` — internal BlueSky events;
- `OPERATOR_EVENT` — operator-entered factual events.

Additional source categories may be added without changing the common evidence storage contract.

The source adapter must preserve source identity and original timestamp. It must not silently transform a measured value into a different parameter or unit.

## 8. Flight Record integration

`Flight Record (полная запись полёта)` and `Test Record (запись испытания)` remain separate records with explicit linkage.

A physical or simulated flight used for verification shall carry:

- `test_run_id` — identifier of the verification execution;
- `flight_record_id` — identifier of the flight record;
- system/build version;
- vehicle identity;
- equipment configuration;
- relevant C2/autopilot log references;
- applicable evidence domain identifiers.

The verification record may reference a Flight Record, but must not overwrite it. This permits operational flight history and certification evidence to remain independently controlled.

## 9. Evidence by domain

Every domain follows the same storage, integrity and export mechanism. Only its source data and approved parameters differ.

Examples:

- C2: signal quality, latency, packet loss, command delivery, acknowledgement, loss/recovery;
- navigation: position, altitude, velocity, fix/quality, corrections and loss/recovery;
- flight control: modes, attitude, trajectory, command-versus-state and transitions;
- mission: approved/uploaded/read-back mission, route, waypoints, deviations and mission state;
- equipment: identity, version, status, health, operating parameters, failures and logs;
- energy: voltage, current, power, battery state, temperature, energy consumption and reserve;
- propulsion: engine/motor state, RPM, commands, temperatures and abnormal conditions;
- performance: speed, altitude, wind, energy consumption, endurance/range and model comparison;
- environment: wind, temperature, pressure, humidity and other approved conditions;
- safety/failsafe: trigger, detection, response, timing, state transition and recovery;
- geographical constraints: position, restriction source/version, boundaries, route intersection and system response;
- timing: source clocks, receipt time, synchronization state and offsets;
- recording/replay: completeness, sequence, timestamps, replay and integrity;
- HMI: operator action, command acceptance, warning, indication and screen evidence;
- integrations: interface identity/version, exchanged data, acknowledgement, error and recovery;
- software configuration: build, source revision, configuration and dependency versions;
- security: authentication, authorization, rejected access and security audit events;
- faults: fault condition, detection, response, recovery and resulting state.

The catalog is the coverage register. A listed domain is not considered implemented until the corresponding source, record, method/case reference, integrity and export linkage exist.

## 10. C2 and Equipment evidence

For communication testing, the evidence package shall preserve both:

1. BlueSky-observed C2 events and derived statistics;
2. original C2/radio/modem/equipment logs supplied by the equipment source.

For equipment testing, the same principle applies: the recorder stores the observed parameter stream and references the original equipment record where available.

The same source-preservation principle applies to every evidence domain in the catalog.

## 11. Configuration Record — запись конфигурации

Before execution, the test record shall capture the configuration identity needed to reproduce the test context. The record shall support at least:

- BlueSky software/build version;
- autopilot and firmware version;
- vehicle identity;
- installed equipment and versions/identifiers where applicable;
- communication configuration;
- measurement equipment;
- test method/test case revisions.

Configuration is evidence context, not a substitute for an approved configuration-control process.

## 12. Prototype storage and export

The prototype provides:

1. machine-readable JSON record;
2. CSV table for engineering analysis;
3. Markdown report for human review;
4. manifest for evidence integrity;
5. preserved raw event stream;
6. integrity anchor for the manifest;
7. an `evidence/` area for original supporting files.

Later certification-document generation shall consume these controlled records rather than manually re-entering measured values.

## 13. Evidence package lifecycle

`INIT → COLLECT → FINALIZE → VERIFY → EXPORT → ARCHIVE`

- `INIT` creates the unique test identity and controlled metadata.
- `COLLECT` appends raw events and associates supporting files.
- `FINALIZE` closes the execution and generates processed outputs.
- `VERIFY` checks completeness, hashes and status consistency.
- `EXPORT` produces the package for engineering and certification documentation.
- `ARCHIVE` preserves the package as the evidence baseline for that execution.

No step changes a physical test result without an explicit controlled action.

## 14. Integrity

At finalization the framework calculates SHA-256 hashes (криптографическая контрольная сумма) for controlled files and writes them to `manifest.json` together with file size and relative path.

A separate `manifest.sha256` file records the SHA-256 of the completed manifest. The manifest is therefore the integrity index for the package, while the sidecar hash provides a simple external integrity check.

## 15. Traceability

The framework is linked to the existing requirement matrix, especially:

- `SYS-005` — operational traceability;
- `SYS-C2-006` — record link events;
- `LOG-001` — acquire autopilot/telemetry logs;
- `LOG-002` — replay operational data;
- `LOG-003` — complete Flight Record;
- `VAL-001` — automated verification;
- `VAL-002..005` — staged verification through real UAV;
- `VAL-006` — regression after interface changes.

The implementation shall preserve a direct reference from each verification record to the applicable requirement, evidence domain and test method/case revision.

## 16. Separation of responsibilities

The collection mechanism records facts. The approved test procedure defines acceptance criteria. The certification package uses approved results and evidence. The logger must not invent, alter or silently downgrade results.

## 17. Future physical-test integration

Source adapters may later feed the same recorder from autopilot telemetry, C2 link statistics, equipment logs, GNSS/RTK/NTRIP data, weather measurements, external measurement instruments, operator events and BlueSky internal events.

The storage and export contract remains unchanged for all evidence domains.

## 18. Certification preparation rule

The framework prepares evidence; it does not declare regulatory compliance by itself. Applicable requirements, approved test methods, acceptance limits and final certification conclusions remain under the project verification/certification process.
