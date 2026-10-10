# BlueSky PRO — Compatibility Resolution Baseline

**Status:** IMPLEMENTATION BASELINE — P0

## 1. Purpose

Turn an integration incompatibility into a diagnosable and, where safely possible, resolvable condition rather than a terminal `INCOMPATIBLE` result.

## 2. Decision chain

```text
MISSION PACKAGE
      ↓
COMPATIBILITY CHECK
      ↓
   ┌──┴─────────────┐
   │                │
COMPATIBLE     INCOMPATIBLE
   │                ↓
   │        DIAGNOSE CAUSE
   │                ↓
   │       FIND SAFE RESOLUTION
   │          /          \
   │       AUTO          USER
   │        ↓              ↓
   │    RECOMPILE      ACTION REQUIRED
   │        ↓
   └──→ REVALIDATE ←──────┘
```

## 3. Incompatibility classes

The resolver shall classify failures at minimum as:

- missing vehicle capability;
- unsupported autopilot function;
- unsupported mission command;
- unsupported equipment function;
- incompatible vehicle configuration;
- incompatible autopilot/firmware version;
- incompatible C2 requirement;
- incompatible regulatory constraint;
- insufficient energy/resource margin;
- unsupported coordinate/trajectory semantics;
- timing/coordination incompatibility;
- unavailable required interface.

## 4. Root-cause diagnosis

The result shall identify:

- affected requirement or mission element;
- affected vehicle/configuration;
- affected adapter/protocol;
- detected capability mismatch;
- source and version of the capability information;
- severity;
- whether a safe resolution exists.

The operator-facing explanation shall be short and actionable; the full diagnostic record remains available for engineering/audit purposes.

## 5. Resolution candidates

The resolver may evaluate safe alternatives such as:

- another supported command representation;
- another mission geometry;
- another route/coverage pattern;
- another UAV from the available fleet;
- another equipment configuration;
- removal or modification of a non-mandatory mission feature;
- required configuration change;
- required software/firmware update.

A resolution is permitted only when its safety and semantic consequences are explicitly known and the resulting mission can be revalidated.

## 6. Never silently degrade

The system shall not silently replace a required mission capability with a lower-quality or lower-safety approximation.

If an alternative changes a relevant mission objective, safety margin, energy margin, timing, regulatory compliance or expected quality, that change shall be represented explicitly in the resolution result.

## 7. Automatic correction

Automatic correction is allowed only for pre-approved deterministic transformations with known safety semantics.

Examples may include protocol-specific command mapping or an equivalent supported representation that preserves the required mission semantics.

The resolver shall never invent a workaround solely to make a mission compile.

## 8. Candidate evaluation

Where several safe resolutions exist, candidates shall be evaluated using the same mission priorities and constraints used by planning/orchestration.

Relevant criteria may include:

- safety;
- mandatory constraints;
- energy reserve;
- equipment/mission quality;
- engine/propulsion resource;
- mission time;
- regulatory feasibility;
- fleet suitability.

Safety and mandatory constraints are gates, not tradeable optimisation objectives.

## 9. Recompile and revalidation

Any accepted correction shall produce a new derived Mission Package version.

```text
RESOLUTION
   ↓
NEW PACKAGE VERSION
   ↓
MISSION COMPILER
   ↓
STATIC VALIDATION
   ↓
SIL/HIL/REQUIRED VALIDATION
   ↓
RELEASE
```

The original package remains immutable and auditable.

## 10. Operator presentation

BlueSky should not expose protocol jargon unless required.

Preferred form:

> **UAV-02 cannot execute this part of the mission because the required function is not supported by its current configuration. BlueSky selected UAV-01 instead because it provides the required capability with sufficient energy reserve.**

The pilot may inspect the technical reason and corrective alternatives when needed.

## 11. Multi-UAV resolution

Compatibility is evaluated per vehicle. One vehicle may be rejected or reassigned without invalidating compatible vehicles in the same parent mission.

The orchestrator may redistribute work when this remains within safety, regulatory, energy and mission-quality constraints.

## 12. Status model

```text
COMPATIBLE
INCOMPATIBLE_DIAGNOSED
RESOLUTION_AVAILABLE
AUTO_CORRECTION_APPLIED
USER_ACTION_REQUIRED
RESOLVED
REVALIDATION_REQUIRED
RESOLVED_AND_VALIDATED
UNRESOLVED
```

## 13. Acceptance criteria

The compatibility resolver is accepted when:

1. incompatibilities have machine-readable root causes;
2. the affected mission/vehicle/configuration is identified;
3. safe resolution candidates can be generated where defined;
4. automatic corrections are restricted to approved transformations;
5. changes affecting mission meaning are never silent;
6. new package versions are created after correction;
7. corrected packages are recompiled and revalidated;
8. multi-UAV reassignment is supported;
9. operator explanations remain concise;
10. the complete diagnostic and resolution history is auditable.
