# BlueSky PRO — Mission Compiler Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## 1. Purpose

The Mission Compiler converts the validated, autopilot-independent Mission Package into an execution representation accepted by a selected Autopilot Adapter.

The compiler is a translation layer. It must preserve mission semantics and must never silently relax safety, energy, regulatory or vehicle constraints.

## 2. Compilation pipeline

```text
VALIDATED MISSION PACKAGE
          ↓
SCHEMA / CONFIGURATION CHECK
          ↓
CAPABILITY CHECK
          ↓
NATIVE MISSION GENERATION
          ↓
NATIVE REPRESENTATION VALIDATION
          ↓
TRANSFER
          ↓
READ-BACK
          ↓
SEMANTIC COMPARISON
          ↓
LOAD VERIFIED
```

## 3. Compile contract

Input:

- immutable Mission Package version;
- target vehicle/configuration;
- target autopilot adapter/version;
- applicable protocol capabilities.

Output:

- native mission representation;
- compilation report;
- deterministic mapping/provenance;
- validation findings.

The compiler shall not modify the source Mission Package.

## 4. Semantic preservation

The compiler shall preserve all safety-critical and mission-critical semantics that the target supports.

At minimum, comparison shall cover where applicable:

- waypoint identity/order;
- position;
- altitude and reference;
- speed/trajectory constraints;
- mission actions;
- equipment actions;
- timing constraints;
- return/recovery behaviour;
- relevant geofence/operational constraints.

Unsupported semantics shall produce an explicit incompatibility result rather than silent approximation.

## 5. Native mappings

Autopilot-specific mappings remain outside the common mission domain:

```text
                  Mission Package
                         ↓
                 Mission Compiler
                   ↙          ↘
            ArduPilot        PX4
             mapping         mapping
                ↓              ↓
          native mission  native mission
```

The same source package must therefore be compilable for different autopilot targets without changing the mission definition.

## 6. Read-back verification

After transfer, the adapter shall retrieve the mission from the target where supported.

The verification process shall compare the read-back representation against the compiled expected representation and, where necessary, against the source Mission Package semantics.

Result states:

`MATCH / MISMATCH / INCOMPLETE / NOT_SUPPORTED / UNKNOWN`

`MISMATCH`, `INCOMPLETE` and `UNKNOWN` shall not be represented as successful load verification.

## 7. Determinism

For identical:

- Mission Package version;
- vehicle configuration version;
- adapter version;
- relevant compiler version;
- compilation inputs;

compilation shall produce the same semantic result.

Any intentional non-determinism must be explicitly identified and controlled.

## 8. Compatibility

The compiler shall reject a target when:

- a mandatory capability is absent;
- a required mission feature is unsupported;
- a required command cannot be represented safely;
- configuration is incompatible;
- the target protocol/adapter version is incompatible.

## 9. Multi-UAV compilation

A parent mission may contain multiple vehicle-specific packages. Each package is compiled independently against its assigned vehicle/autopilot configuration while preserving the parent mission correlation.

```text
PARENT MISSION
 ├─ Package A → Compiler → Autopilot A
 ├─ Package B → Compiler → Autopilot B
 └─ Package C → Compiler → Autopilot C
```

## 10. Failure handling

Compilation or verification failure shall produce a structured result containing:

- failure stage;
- affected mission element;
- target system;
- reason;
- severity;
- corrective action if determinable;
- package/version correlation.

## 11. Evidence

The compiler shall retain sufficient information to reproduce and audit the translation, including source package version, target configuration, adapter/compiler versions and verification outcome.

## 12. Acceptance criteria

The Mission Compiler is accepted when:

1. the same common Mission Package can target multiple autopilots;
2. native mappings are isolated from the mission domain;
3. mandatory unsupported features are rejected explicitly;
4. compilation does not mutate the source package;
5. semantic preservation is automatically checked;
6. read-back comparison is performed where supported;
7. mismatches block successful load verification;
8. compilation is deterministic for identical inputs;
9. multi-UAV packages can be compiled independently;
10. complete evidence is retained.

## 13. Implementation boundary

Concrete ArduPilot and PX4 mappings, MAVLink command encodings and protocol-specific serialization belong to their respective adapters. This document defines the common compilation behaviour and verification contract.
