# BlueSky PRO — Mission Package Baseline

**Status:** ARCHITECTURE BASELINE — P0 integration contract

## 1. Purpose

Mission Package is the controlled executable representation produced by BlueSky after mission planning, allocation, optimization and validation. It is the boundary object between the BlueSky mission domain and a concrete execution adapter.

```text
MISSION MODEL
   ↓
PLANNING / ORCHESTRATION
   ↓
VALIDATED MISSION SOLUTION
   ↓
MISSION PACKAGE
   ↓
AUTOPILOT ADAPTER
   ↓
AUTOPILOT / UAV
```

## 2. Separation of concerns

The package shall distinguish:

- mission intent;
- executable vehicle-specific content;
- safety and regulatory constraints;
- validation result;
- provenance and versions;
- execution state.

The autopilot adapter translates the package into native autopilot commands. The core mission model remains autopilot-independent.

## 3. Package identity and versioning

Every package shall contain a unique package identifier and immutable version information sufficient to establish exactly what was approved for execution.

Minimum provenance:

- mission ID/version;
- vehicle configuration ID/version;
- equipment configuration ID/version;
- autopilot adapter ID/version;
- algorithm/orchestrator version;
- environmental-data versions/timestamps;
- regulatory-data/status version/timestamp;
- validation result/version;
- creation timestamp.

## 4. Coordinate and reference conventions

The package shall explicitly identify all coordinate, altitude, time and reference conventions required for unambiguous interpretation.

No adapter may silently reinterpret coordinate frames, altitude references, units or time semantics.

## 5. Mission content

Depending on mission type, the package may contain:

- mission objective;
- assigned UAV/subtask;
- ordered route/trajectory;
- waypoints;
- navigation parameters;
- action/command sequence;
- equipment actions;
- timing constraints;
- speed/altitude targets;
- geofencing/operational constraints;
- recovery/return behaviour;
- multi-UAV coordination data.

Only content applicable to the selected vehicle and equipment configuration shall be executable.

## 6. Safety and energy information

The package shall carry the relevant validated limits and decision evidence, including where applicable:

- minimum energy reserve;
- predicted mission consumption;
- recovery/return requirement;
- operational limits;
- mandatory safety constraints;
- validation state.

A package must not become executable by merely bypassing a safety or energy validation field.

## 7. Validation state

The package shall have an explicit lifecycle state:

```text
DRAFT
  ↓
PLANNED
  ↓
VALIDATED
  ↓
RELEASED
  ↓
UPLOADED
  ↓
READ_BACK_VERIFIED
  ↓
EXECUTING
  ↓
COMPLETED / ABORTED
```

Only an appropriately validated and released package may enter execution.

## 8. Upload contract

The Autopilot Adapter shall report at minimum:

- connection/session identity;
- target vehicle identity;
- package identity/version;
- upload start/end;
- accepted/rejected status;
- command/mission acknowledgement;
- validation errors;
- timeout or communication failure.

## 9. Read-back verification

After upload, BlueSky shall read back the mission from the target system where supported and compare it against the approved package.

Verification shall detect at minimum:

- missing items;
- reordered items;
- changed coordinates;
- changed altitude/reference;
- changed commands/parameters;
- changed equipment actions;
- changed mission metadata where safety-relevant.

A mismatch in safety-critical executable content blocks release to execution.

## 10. Native autopilot translation

The adapter is responsible for translating the common package into the native execution representation of:

```text
COMMON MISSION PACKAGE
        ↓
AUTOPILOT ADAPTER
   ┌────┼────┐
   ↓    ↓    ↓
ArduPilot PX4 OEM
```

Native protocol details must remain inside the adapter boundary.

## 11. Idempotency and duplicate protection

Package transfer shall use stable package/version identifiers and execution safeguards so that retrying a transfer cannot silently create an unintended duplicate mission or alter the approved mission.

## 12. Failure handling

Failures shall be classified at minimum as:

- transport failure;
- target incompatibility;
- package validation failure;
- partial upload;
- read-back mismatch;
- timeout;
- target rejection;
- stale package/data.

A failed or ambiguous transfer shall not be represented to the operator as successfully loaded.

## 13. Multi-UAV missions

A parent mission may contain multiple vehicle-specific mission packages.

```text
PARENT MISSION
   ├── PACKAGE UAV-01
   ├── PACKAGE UAV-02
   └── PACKAGE UAV-03
```

Each package has its own vehicle, autopilot, equipment, validation and execution identity while remaining linked to the parent mission.

## 14. Change control

Any change to executable mission content after validation shall invalidate the affected release state and trigger the required revalidation sequence.

Environmental changes that affect feasibility may trigger replanning and generation of a new package version.

## 15. Acceptance criteria

The Mission Package contract is complete when:

1. a mission can be represented independently of a specific autopilot;
2. the package identifies exact versions and provenance;
3. coordinate/reference semantics are explicit;
4. safety and energy constraints are represented;
5. the package can be translated by an adapter into native autopilot form;
6. upload status is deterministic and observable;
7. read-back verification is supported where the target permits it;
8. mismatches block unsafe execution;
9. multi-UAV packages remain linked to a parent mission;
10. any executable change causes appropriate revalidation.

## 16. Implementation boundary

This document defines the common package contract. Concrete serialization format, schemas, ArduPilot/PX4/OEM mappings and transport mechanisms shall be separately versioned implementation artifacts and verified against the target systems.
