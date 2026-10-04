---
id: REQ-ALLOCATION-AUDIT-002
type: direct_requirement_allocation_followup
status: WORKING
authority: MASTER-REQUIREMENTS-REGISTER-001
date: 2026-09-20
---

# Direct Requirement Allocation Follow-up 002

## Scope

Follow-up to REQ-ALLOCATION-AUDIT-001 for the previously flagged records:

- SAF-REQ-002
- SAF-REQ-004
- SAF-REQ-010
- SYS-REQ-110
- SYS-REQ-111
- SYS-REQ-112

No requirement is created, renamed, merged, baselined, or marked VERIFIED.

## 1. SAF-REQ-002 — Invalid data handling

The repository contains the authoritative safety wording in the Safety Requirements Baseline.

Existing allocation/verification records identify SAF-REQ-002 in:

- C2-V02 verification case;
- C2-V04 verification case;
- C2 verification reconciliation;
- C2 verification execution result record.

The current execution result records for C2 are explicitly `EXECUTION_STUB / NOT EXECUTED`.

**Disposition:**
- Requirement identity: PRESENT
- Safety wording: PRESENT
- Verification allocation: PRESENT
- Verification execution/result: OPEN
- Evidence: OPEN
- Configuration-bound evidence: OPEN

This is therefore **not a missing verification allocation**. It is an unexecuted verification/evidence closure gap.

## 2. SAF-REQ-004 — Loss of critical data

The repository contains the authoritative safety wording.

Existing C2-V04 and C2-V06 records explicitly allocate SAF-REQ-004 to invalid/lost-input handling and controlled degraded/recovery behaviour.

The C2 execution record remains `EXECUTION_STUB / NOT EXECUTED`.

**Disposition:**
- Requirement identity: PRESENT
- Safety wording: PRESENT
- Verification allocation: PRESENT
- Verification execution/result: OPEN
- Evidence: OPEN
- Configuration-bound evidence: OPEN

The remaining gap is execution/evidence, not creation of a new requirement or test concept.

## 3. SAF-REQ-010 — Dynamic Return degradation

The repository contains direct safety-case allocation:

`Safety Objective → SAF-REQ-009 / SAF-REQ-010 → Dynamic Return`

The Energy Model reconciliation explicitly preserves SAF-REQ-010 as the authoritative safety requirement and treats the Energy Model as supporting engineering/calculation design.

The verification evidence index contains:

`EVD-018 → V-RET-002 → SAF-REQ-010 → Scenario Test`

with status `PLANNED`.

Energy design records also allocate SAF-REQ-010 to return-feasibility reassessment and identify supporting data/model gaps.

**Disposition:**
- Requirement identity: PRESENT
- Safety allocation: PRESENT
- Design allocation: PRESENT
- Verification case/evidence identity: PRESENT
- Verification execution: OPEN
- Evidence: PLANNED
- Configuration-bound evidence: OPEN

The earlier allocation gap is therefore reclassified as **allocation covered; execution/evidence open**.

## 4. SYS-REQ-110 — Multi-Agent AI Orchestration

Direct repository records exist:

- `02_SYSTEM/Requirements/SYS-REQ-110.md`
- `02_SYSTEM/Architecture/SYS-REQ-110-CAPABILITY-TRACEABILITY.md`
- `ARCH-DEC-046-CAPABILITY-TRACEABILITY.md`
- `ARCH-DEC-046-REQUIREMENT-GAP-ANALYSIS.md`
- `02_SYSTEM/Design/AI-ORCHESTRATION-LAYER-DESIGN.md`

The records explicitly bind SYS-REQ-110 to ARCH-DEC-046 and the AI orchestration design.

Master Requirements Register retains SYS-REQ-110 as KEEP.

**Disposition:**
- Requirement identity: PRESENT
- Architecture allocation: PRESENT
- Design allocation: PRESENT
- Direct verification/result/evidence allocation: NOT YET CLOSED BY THIS PASS

No new architecture requirement is justified.

## 5. SYS-REQ-111 — AI Agent Authority and Proposal Control

Direct repository records exist:

- `02_SYSTEM/Requirements/SYS-REQ-111.md`
- `02_SYSTEM/Architecture/SYS-REQ-111-CAPABILITY-TRACEABILITY.md`
- `ARCH-DEC-046-CAPABILITY-TRACEABILITY.md`
- `ARCH-DEC-046-REQUIREMENT-GAP-ANALYSIS.md`
- `02_SYSTEM/Design/AI-ORCHESTRATION-LAYER-DESIGN.md`

The requirement is explicitly connected to ARCH-DEC-046 and the AI orchestration design.

**Disposition:**
- Requirement identity: PRESENT
- Architecture allocation: PRESENT
- Design allocation: PRESENT
- Verification/result/evidence allocation: OPEN

No new requirement is justified.

## 6. SYS-REQ-112 — Offline AI Operational Continuity

Direct repository records exist:

- `02_SYSTEM/Requirements/SYS-REQ-112.md`
- `02_SYSTEM/Architecture/SYS-REQ-112-CAPABILITY-TRACEABILITY.md`
- `ARCH-DEC-046-CAPABILITY-TRACEABILITY.md`
- `ARCH-DEC-046-REQUIREMENT-GAP-ANALYSIS.md`
- `02_SYSTEM/Design/AI-ORCHESTRATION-LAYER-DESIGN.md`

The requirement is explicitly connected to ARCH-DEC-046 and the AI orchestration design.

**Disposition:**
- Requirement identity: PRESENT
- Architecture allocation: PRESENT
- Design allocation: PRESENT
- Verification/result/evidence allocation: OPEN

No new requirement is justified.

## 7. Corrected gap picture

The previous audit treated SAF-REQ-002, SAF-REQ-004 and SAF-REQ-010 as direct allocation gaps.

Repository evidence now shows that these three already have allocation/verification records.

Their actual remaining gaps are:

`VERIFICATION EXECUTION → RESULT → EVIDENCE → CONFIGURATION`

For SYS-REQ-110…112, architecture and design allocation are demonstrated. Their remaining closure is verification/result/evidence/configuration.

## 8. Next controlled operation

The next pass shall extract the exact verification records for:

- SYS-REQ-110
- SYS-REQ-111
- SYS-REQ-112
- SAF-REQ-002
- SAF-REQ-004
- SAF-REQ-010

and reconcile:

`Requirement → Verification Case → Execution Result → Evidence ID → Configuration`

Existing cases/evidence records shall be reused. New cases shall be created only where a demonstrable coverage gap remains.

## Status

**WORKING — ALLOCATION RECONCILED; EXECUTION/EVIDENCE CLOSURE REMAINS OPEN.**
