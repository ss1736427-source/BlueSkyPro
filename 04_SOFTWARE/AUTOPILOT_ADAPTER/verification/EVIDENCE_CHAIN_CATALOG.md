# BlueSky PRO — Evidence Chain Catalog

**Status:** IMPLEMENTATION BASELINE — P0
**Purpose:** define the complete evidence-chain coverage for functions and interfaces that may require objective verification records. This catalog is an implementation structure; applicable regulatory requirements, approved methods and acceptance criteria remain controlled by the project certification process.

## 1. Common chain

Every evidence-bearing verification domain shall follow the same chain:

`Requirement → Verification Method → Test Case → Configuration → Execution → Source Data → Raw Record → Analysis → Result → Evidence Package → Certification Reference`

The chain is common; only the data sources and parameters change by domain.

## 2. Evidence domains

### EC-01 — C2 Communication Channel

**C2 (Command and Control — канал командного управления и телеметрии)**

`C2 Requirement → C2 Test Method → C2 Test Case → C2 Configuration → C2 Execution → Radio/Modem + BlueSky Data → Raw C2 Record → Link Analysis → Result → C2 Evidence Package → Certification Reference`

Typical evidence: link state, latency, packet loss, signal quality, command delivery, acknowledgement, loss/recovery events, original modem/radio logs.

### EC-02 — Autopilot Interface

`Autopilot Requirement → Interface Verification Method → Interface Test Case → Autopilot Configuration → Execution → Autopilot Protocol Data → Raw Record → Protocol/State Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: connection state, protocol version, commands, acknowledgements, execution states, errors, telemetry exchange and read-back data.

### EC-03 — Navigation / GNSS / RTK / NTRIP

`Navigation Requirement → Navigation Test Method → Navigation Test Case → Navigation Configuration → Execution → GNSS/RTK/NTRIP Data → Raw Navigation Record → Position/Quality Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: position, altitude, velocity, fix type, satellite/quality indicators, correction state, loss/recovery events, reference-system data and independent measurements where required.

### EC-04 — Flight Control / Flight State

`Flight-Control Requirement → Flight Test Method → Flight Test Case → Vehicle Configuration → Execution → Autopilot + Flight Record → Raw Flight Data → Flight-State Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: flight mode, attitude, trajectory, commanded versus actual state, state transitions, control events, timing and anomalies.

### EC-05 — Mission / Route Execution

`Mission Requirement → Mission Verification Method → Mission Test Case → Approved Mission + Configuration → Execution → Mission/Flight Data → Raw Mission Record → Route/Execution Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: approved mission, uploaded mission, read-back mission, comparison result, waypoint sequence, route deviations, mission state, start/pause/resume/abort events and completion status.

### EC-06 — Installed Equipment

`Equipment Requirement → Equipment Verification Method → Equipment Test Case → Equipment Configuration → Execution → Equipment Data → Raw Equipment Record → Equipment Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: equipment identity, firmware/version, status, operating parameters, health, failures, commands, responses and original equipment logs.

**Equipment** is the controlled project term; do not replace it with `payload` in project documentation where the controlled terminology requires equipment.

### EC-07 — Energy / Battery / Power

`Energy Requirement → Energy Test Method → Energy Test Case → Battery/Power Configuration → Execution → Power-System Data → Raw Energy Record → Consumption/Reserve Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: voltage, current, power, battery state, temperature, degradation coefficient when defined, energy consumed, reserve, alarms and shutdown/failsafe events.

### EC-08 — Propulsion

`Propulsion Requirement → Propulsion Test Method → Propulsion Test Case → Propulsion Configuration → Execution → Engine/Motor Data → Raw Propulsion Record → Performance/Health Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: RPM, throttle/command, engine or motor state, temperatures, pressures where applicable, power, abnormal conditions, start/stop behaviour and equipment logs.

### EC-09 — Aerodynamic / Performance Model

`Performance Requirement → Performance Verification Method → Performance Test Case → Vehicle + Equipment Configuration → Execution → Flight/Environmental Data → Raw Performance Record → Performance Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: mass/configuration, speed, altitude, wind, energy consumption, endurance, range, climb/descent performance and model-versus-observation comparisons.

### EC-10 — Weather / Environmental Conditions

`Environmental Requirement → Environmental Test Method → Environmental Test Case → Environmental Configuration → Execution → Weather/Measurement Data → Raw Environmental Record → Condition Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: wind speed/direction, temperature, pressure, humidity and other parameters required by the approved test procedure; source, timestamp and measurement method are preserved.

### EC-11 — Safety / Failsafe / Loss of Link

`Safety Requirement → Safety Verification Method → Safety Test Case → Safety Configuration → Execution → Autopilot + C2 + Flight Data → Raw Safety Record → Failure/Recovery Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: trigger condition, detected condition, system response, timing, mode transition, return/hold/land behaviour as applicable, communication state and recovery.

### EC-12 — Geographical Restrictions / Airspace / Geofence

`Airspace/Safety Requirement → Geographical Verification Method → Geographical Test Case → Map/Restriction Configuration → Execution → Position + Restriction Data → Raw Geographical Record → Compliance Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: coordinate system, restriction source/version, boundaries, vehicle position, route intersection/clearance, warning/rejection events and operator acknowledgement where applicable.

### EC-13 — Time / Synchronisation

`Timing Requirement → Timing Verification Method → Timing Test Case → Time Configuration → Execution → System/Source Clocks → Raw Timing Record → Synchronisation Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: source timestamps, local receipt timestamps, clock offsets, synchronisation state and timing anomalies.

### EC-14 — Data Recording / Replay

`Recording Requirement → Recording Verification Method → Recording Test Case → Recording Configuration → Execution → System Logs → Raw Record → Completeness/Replay Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: log completeness, timestamps, sequence integrity, replay capability, missing-data detection, file integrity and reproducibility.

### EC-15 — BlueSky Internal Functions / HMI

`System/HMI Requirement → HMI Verification Method → HMI Test Case → Software Configuration → Execution → BlueSky Events + Screen Evidence → Raw Event Record → Behaviour Analysis → Result → Evidence Package → Certification Reference`

**HMI (Human-Machine Interface — интерфейс взаимодействия человека с системой)** evidence may include command acceptance, warnings, mode transitions, operator actions, screen captures and system event records where the approved verification requires them.

### EC-16 — External Interfaces / Integrations

`Integration Requirement → Integration Verification Method → Integration Test Case → Integration Configuration → Execution → External Interface Data → Raw Integration Record → Exchange/Failure Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: interface identity/version, input/output messages, timestamps, acknowledgements, errors, rejected data, recovery and original external-system records.

### EC-17 — Communications Other Than C2

`Communication Requirement → Communication Verification Method → Communication Test Case → Network/Interface Configuration → Execution → Interface Data → Raw Communication Record → Performance/Availability Analysis → Result → Evidence Package → Certification Reference`

This covers additional operational communications that are distinct from the primary C2 link, for example data services, ground-network interfaces or other approved communication paths. Each channel receives its own source identity and evidence chain.

### EC-18 — Software Configuration / Version Control

`Configuration Requirement → Configuration Verification Method → Configuration Test Case → Controlled Baseline → Execution → Build/Configuration Data → Raw Configuration Record → Consistency Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: software version, build identifier, source revision, configuration identifiers, firmware versions, equipment versions and dependency versions.

### EC-19 — Security / Access Control

`Security Requirement → Security Verification Method → Security Test Case → Security Configuration → Execution → Security-System Data → Raw Security Record → Security Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: authentication, authorization, role permissions, rejected access, security events, configuration and relevant audit records when required by the approved verification scope.

### EC-20 — Faults / Anomalies / Recovery

`Reliability/Safety Requirement → Fault Verification Method → Fault Test Case → Fault-Injection/Normal Configuration → Execution → System Fault Data → Raw Fault Record → Detection/Recovery Analysis → Result → Evidence Package → Certification Reference`

Typical evidence: injected or observed fault, detection, system response, recovery time, resulting state, operator notification and source logs.

## 3. Common evidence object

Every domain uses the same minimum event structure:

- timestamp;
- parameter identifier;
- value;
- unit;
- source;
- quality/status, where available;
- context, where required.

The domain-specific chain must never change the common storage contract.

## 4. Evidence package contents

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

The `evidence/` directory may contain original logs, measurement files, configuration snapshots, photographs, screen/video records and other supporting material required by the approved test procedure.

## 5. Certification-document export

The export mechanism shall support two levels:

**Engineering export** — complete machine-readable and human-readable results for analysis.

**Certification export** — controlled tables/reports referencing the requirement, method, execution, result and evidence package without manual re-entry of measured values wherever possible.

The export must preserve identifiers and integrity references so that every reported result can be traced back to its source evidence.

## 6. Coverage rule

A verification domain is not considered implemented merely because its name appears in this catalog. It is implemented when its source, record fields, test method/case references, storage, integrity and export linkage exist in the software/documentation chain.

The catalog therefore acts as a coverage register for future implementation work.

## 7. Prototype rule

The prototype may demonstrate every chain with synthetic, replayed or bench data. Such data is explicitly classified and cannot be presented as physical verification evidence.

The objective is to demonstrate the complete mechanism now and replace individual sources with real equipment during the later verification campaign.
