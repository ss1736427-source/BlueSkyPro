# BlueSky PRO — Evidence Coverage Gap Audit

**Status:** WORKING BASELINE — P0
**Purpose:** determine whether each evidence-bearing domain has an actual end-to-end mechanism, rather than merely an entry in the evidence catalog.

## 1. Assessment rule

A domain is **IMPLEMENTED** only when all required links exist:

`Requirement → Test Method → Test Case → Configuration → Execution → Source Data → Raw Data → Processed Data → Result → Evidence Package → Certification Reference`

A domain is **PARTIAL** when the common collection/storage/export mechanism exists but one or more domain-specific source, method, execution, analysis, or certification links are not yet implemented.

A domain is **MISSING** when the required mechanism is absent.

Prototype data (`DEMONSTRATION`, `DEVELOPMENT`, `REPLAY`, `BENCH`) is never treated as physical verification evidence.

## 2. Current baseline assessment

| Domain | Source channel | Common recorder/export | Domain source adapter | Method/Test Case linkage | Analysis/Result linkage | Certification linkage | Status |
|---|---|---|---|---|---|---|---|
| EC-01 | SRC-01 C2_LINK | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-02 | SRC-02 AUTOPILOT_TELEMETRY | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-03 | SRC-06 GNSS_RTK_NTRIP | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-04 | SRC-03 FLIGHT_CONTROL | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-05 | SRC-04 MISSION | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-06 | SRC-05 EQUIPMENT | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-07 | SRC-08 ENERGY_POWER | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-08 | SRC-09 PROPULSION | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-09 | SRC-10 PERFORMANCE_MODEL | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-10 | SRC-07 WEATHER | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-11 | SRC-19 FAULT_RECOVERY + C2/autopilot | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-12 | SRC-11 AIRSPACE_GEOFENCE | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-13 | SRC-12 TIME_SYNCHRONIZATION | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-14 | SRC-13 DATA_RECORDING_REPLAY | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-15 | SRC-14 BLUESKY_EVENT + SRC-16 HMI | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-16 | SRC-17 EXTERNAL_INTEGRATION | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-17 | additional communication interfaces | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-18 | build/configuration data | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-19 | SRC-18 SECURITY_ACCESS | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |
| EC-20 | SRC-19 FAULT_RECOVERY | YES | PARTIAL | PARTIAL | PARTIAL | YES | PARTIAL |

**Interpretation:** the common evidence infrastructure is implemented, but the 20 domains must not yet be represented as fully implemented physical evidence chains. The correct baseline status is therefore `PARTIAL` until each domain is connected to its real or approved source and its verification artifacts.

## 3. Mandatory domain completion checklist

For each domain, the implementation work is complete only when:

1. stable source identifier exists;
2. Source Adapter exists and emits the common event format;
3. timestamp and source provenance are preserved;
4. parameter/value/unit/quality fields are available;
5. configuration identifier is linked;
6. `test_run_id` and/or `flight_record_id` is linked where applicable;
7. raw source data is preserved unchanged;
8. domain-specific processing/analysis is defined;
9. result status is derived according to the approved method;
10. evidence package is generated;
11. integrity manifest is generated;
12. certification export references the original evidence;
13. automated verification covers the chain;
14. prototype/bench/physical/approved evidence class is explicit.

## 4. Required records

Every verification run shall be capable of producing:

- `record.json` — test/session identity and configuration;
- `events.jsonl` — chronological source events;
- `processed.csv` — machine-readable processed parameters;
- `evidence/` — original logs and supporting evidence;
- `manifest.json` — package inventory and identifiers;
- `manifest.sha256` — integrity control;
- `report.md` — human-readable result report.

## 5. Certification documentation connection

The system shall not manually re-enter measured values into certification documentation where the value can be referenced from controlled evidence. Certification export shall carry identifiers back to the source package.

The authoritative acceptance decision remains outside the recorder: BlueSky records facts and provenance; the approved verification process determines whether those facts demonstrate compliance.

## 6. Prototype-first implementation order

Priority for the working demonstration prototype:

1. `EC-14` Data Recording / Replay;
2. `EC-18` Software Configuration / Version Control;
3. `EC-15` BlueSky Events / HMI;
4. `EC-05` Mission / Route Execution;
5. `EC-02` Autopilot Interface;
6. `EC-01` C2 Communication;
7. `EC-06` Equipment;
8. `EC-03` Navigation;
9. `EC-07` Energy / Power;
10. `EC-11` Safety / Failsafe;
11. remaining domains through the same common mechanism.

This order is for prototype integration. It does not define the future certification test order.

## 7. No artificial completion

A catalog entry, synthetic demonstration, replay, or bench fixture does not by itself establish physical compliance. Status shall advance only when the corresponding implementation and approved verification evidence exist.

## 8. Evidence data lifecycle

The common lifecycle is defined in `VERIFICATION_DATA_LIFECYCLE.md`. All domains use the same sequence from requirement and test method through source data capture, raw-data preservation, processed data and analysis, result, evidence package and certification reference.

## 9. Certification export

The controlled export mechanism is implemented by the project evidence export tooling, including `certification_export.py`, and is governed by the verification/evidence integration contract. Export must preserve provenance, evidence class, configuration identity and integrity information. It is a preparation mechanism for certification documentation, not a substitute for formal acceptance.

## 10. Extension rule

If requirements, safety analysis, engineering analysis, external interfaces, equipment, or certification planning introduce a new evidence-bearing source not represented by EC-01…EC-20, a new evidence domain and source channel shall be added before implementation is considered complete. The common evidence contract remains unchanged.

## 11. EC-14 implementation reconciliation

The EC-14 implementation baseline now includes a concrete C++ recorder/replay mechanism, Source Adapter conversion, and automated contract tests:

- `core/flight_evidence_recorder.hpp` — session recorder and replay reader;
- `core/flight_evidence_recorder_test.cpp` — multi-source collection, replay and raw-data preservation test;
- `core/evidence_source_adapter.hpp` — conversion from source-specific records to the common `EvidenceEvent` format without altering the source record;
- `core/evidence_source_adapter_test.cpp` — adapter field-preservation and recorder persistence test;
- `CMakeLists.txt` — CTest registration.

The Source Adapter test has been executed from the normal Release build and passed all diagnostic checks (source fields preserved, adapted event persisted, and raw reference persisted). A diagnostic copy also executes successfully. This confirms the implementation path `Source Record → Source Adapter → Common EvidenceEvent → Recorder` for the current prototype test fixture.

This closes the previously missing **implementation mechanism and prototype adapter integration** gap for EC-14. It does **not** close the domain as physical verification evidence: controlled build execution evidence, complete certification-package generation/integrity verification and physical/operational verification remain outstanding. The Release/CTest `0xC0000409` issue observed earlier is a separate test-runner/runtime condition and shall not be treated as evidence of functional failure of the Source Adapter without reproducing it after the diagnostic test revision.

## 12. EC-18 implementation reconciliation

The EC-18 implementation baseline now includes a controlled software configuration record:

- `SOFTWARE_CONFIGURATION_BASELINE.md` — controlled configuration identity and change-control rules;
- `software_configuration_baseline.json` — machine-readable P0 configuration baseline;
- baseline identifies repository, controlled branch, source revision, software release state, build system, C++ standard and primary build configuration.

This establishes the prototype configuration-control mechanism and provides a stable configuration identifier for linkage into the common evidence lifecycle. EC-18 remains **PARTIAL** because execution-level artifact capture, toolchain identity capture where required, automated configuration verification, and complete verification/certification linkage have not yet been demonstrated.

## 13. EC-15 implementation reconciliation

The EC-15 baseline includes the BlueSky Event/HMI contract and automated contract test:

- `core/bluesky_event_hmi_contract.hpp` — event severity/state model and required event fields;
- `core/bluesky_event_hmi_contract_test.cpp` — contract validation test;
- `CMakeLists.txt` — CTest registration.

The recovered `main` baseline was built/tested locally in Release configuration and `bluesky_event_hmi_contract_test` passed: **1/1 tests passed, 0 failed**.

This confirms the current prototype contract and automated test baseline for the Event/HMI interface. EC-15 remains **PARTIAL** because real HMI/event-source integration, domain-specific evidence linkage, complete certification-package generation/integrity verification and physical/operational verification remain outstanding.

## 14. EC-06 implementation reconciliation

The EC-06 implementation baseline now includes:

- `core/equipment_adapter_baseline.hpp` / `.cpp` — concrete Equipment Adapter;
- equipment identity, profile/configuration references, capabilities, state, telemetry, action translation and data-output handling;
- `core/equipment_adapter_baseline_test.cpp` — automated contract/behavior test;
- CMake/CTest registration;
- `core/EC06_STATUS.md` — controlled implementation-status boundary.

The baseline is a prototype/bench mechanism only. EC-06 remains **PARTIAL** because approved real equipment source integration, controlled execution evidence, domain processing/result linkage, integrity-controlled evidence package, certification export verification and physical/operational verification remain outstanding.
