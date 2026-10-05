---
id: C2-SYSTEM-REQUIREMENTS-BASELINE-001
type: system_requirements_baseline
status: draft_for_agreement
jurisdiction: Russian Federation
system: BlueSky PRO
source_basis: REGULATORY_SOURCE_REGISTER.md
---

# BLUE SKY PRO — C2 SYSTEM REQUIREMENTS BASELINE 001

## 1. Purpose

This document allocates the C2-related system obligations already identified in the controlled Regulatory Source Register into candidate BlueSky system requirements.

It does not replace the Master Requirements Register and does not constitute regulatory approval or an approved certification basis.

## 2. Requirement status rule

All requirements in this document are `DRAFT` until accepted into `MASTER_REQUIREMENTS_REGISTER.md` through controlled change.

No regulatory numerical threshold is invented here. Where the applicable value is not yet established, the requirement is qualitative and the acceptance criterion remains `TBD`.

## 3. C2 boundary

BlueSky supports two integration profiles:

- `DIRECT-C2` — C2 function/interface within the BlueSky-controlled system boundary;
- `PROVIDER-C2` — C2 service supplied by an external provider, with BlueSky responsible for the defined interface and system-side functions only.

BlueSky shall not silently assume provider, network, radio-spectrum, or external infrastructure responsibilities unless those responsibilities are explicitly included in the approved system scope.

## 4. Candidate requirements and controlled source allocation

| ID | Requirement | Regulatory clause mapping | Existing authoritative coverage | Allocation | Verification | Status |
|---|---|---|---|---|---|---|
| C2-REQ-001 | BlueSky shall maintain an explicit C2 operating mode for each mission and shall make the applicable mode available to functions that depend on C2 state. | C2-142-001, C2-142-006 | SYS-REQ-083 + existing C2 architecture records | FLIGHT / C2 | Inspection + Test | DRAFT |
| C2-REQ-002 | BlueSky shall monitor the configured C2 path and determine at least the states NORMAL, DEGRADED and LOST according to controlled system criteria. | C2-142-018, C2-142-020 | SYS-REQ-086, SYS-REQ-093 | C2 / INTEGRATION | Analysis + Test | DRAFT |
| C2-REQ-003 | BlueSky shall process the C2 quality/availability parameters required by the approved operational and certification baseline. | C2-142-004, C2-142-011, C2-142-013, C2-142-014, C2-142-019, C2-142-025 | SYS-REQ-091 where applicable; quantitative values remain TBD | C2 / CONFIGURATION | Inspection + Test | DRAFT |
| C2-REQ-004 | For PROVIDER-C2, BlueSky shall validate the availability and applicable operating boundary of the external C2 service before and during mission execution. | C2-142-003, C2-142-008, C2-142-010, C2-142-012 | Existing provider/interface allocation; no provider-side requirement assigned to BlueSky | INTEGRATION / ICD | Inspection + Integration Test | DRAFT |
| C2-REQ-005 | BlueSky shall distinguish C2 degradation/loss from available diagnostic information about the initiating subsystem or interface, where such information is provided. | C2-142-024 | SYS-REQ-086, SYS-REQ-093 + diagnostics/safety allocation | C2 / SAFETY / HUB | Analysis + Test | DRAFT |
| C2-REQ-006 | BlueSky shall execute the approved response to C2 degradation or loss defined by the applicable Safety Requirements Baseline and mission state. | C2-142-023, C2-142-024 | SYS-REQ-081, SYS-REQ-082, SYS-REQ-086, SYS-REQ-093 | FLIGHT / SAFETY | Scenario Test | DRAFT |
| C2-REQ-007 | BlueSky shall record C2 state transitions, loss/restoration events, relevant diagnostics and automatic safety actions in the controlled mission/flight event record. | C2-142-021, C2-142-024 | Existing event/logging records | HUB / DATA | Test + Log Inspection | DRAFT |
| C2-REQ-008 | BlueSky shall maintain a controlled time reference sufficient to order C2 commands, telemetry, state transitions, notifications and associated evidence. | C2-142-005, C2-142-021 | Existing common time/reference allocation | SYSTEM / HUB | Analysis + Test | DRAFT |
| C2-REQ-009 | BlueSky shall prevent an AI, optimization or mission-planning function from bypassing the established safety authority chain when C2 conditions change. | C2-142-023, C2-142-024 + existing safety architecture | SYS-REQ-085 + ARCH-DEC-007, ARCH-DEC-016, ARCH-DEC-017 | SAFETY / AI / FLIGHT | Analysis + Scenario Test | DRAFT |
| C2-REQ-010 | BlueSky shall expose C2 state and relevant warnings to the operator through the controlled HMI without requiring access to internal implementation details. | C2-142-015, C2-142-020 | Existing HMI/C2 allocation | HMI / C2 | Inspection + Usability/Safety Test | DRAFT |

## 5. Source relationship

Primary source identified in the project register:

`REG-013 — Приказ Минтранса России от 24.04.2025 №142`

The Regulatory Source Register identifies direct BlueSky impacts including C2 mode identification, C2 monitoring, safe response to degradation/loss, controlled C2 parameters, C2 data/interface control, time model, event recording and information-security architecture. Applicability remains dependent on the approved system/certification scope.

The detailed clause references above are working allocations from `C2_CLAUSE_LEVEL_MAPPING_001.md`. They are not a declaration that the corresponding candidate requirement has been baselined.

## 6. Allocation to existing architecture

```text
C2-REQ-001..004 → C2 / INTEGRATION
C2-REQ-005..008 → C2 / HUB / SAFETY
C2-REQ-009      → SAFETY / AI / FLIGHT
C2-REQ-010      → HMI / C2
```

The existing project authority chain remains unchanged:

```text
MISSION
→ VALIDATION
→ READINESS
→ SAFETY GATE
→ OPERATOR APPROVAL
→ EXECUTION
```

C2 state changes do not create an alternative authority path; they trigger the approved safety/flight logic within that chain.

## 7. Open items

1. Final certification-object applicability: `OPEN`.
2. Exact clause-level source references: working allocation established above; final applicability remains `OPEN` until certification object and scope are finalized.
3. Numerical C2 performance/availability/continuity/integrity criteria: `TBD`.
4. Exact allocation between BlueSky, aircraft flight-control software and external C2 provider: `OPEN`.
5. Final ICD parameter/message set: `OPEN`.
6. Verification acceptance thresholds: `TBD` until requirements are baselined.

## 8. Next step

```text
C2-REQ-001..010
→ controlled review
→ Master Requirements Register
→ C2 ICD baseline
→ Safety allocation
→ Verification acceptance criteria
→ traceability/evidence
```

No new authoritative system requirement is created by this document. Existing authoritative IDs retain priority; a new ID is justified only by a proven gap.
