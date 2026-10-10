---
id: TEST-071
type: test_case
title: Failed Improvement Iteration Traceability Inspection
status: draft
verifies:
  - "[[SYS-REQ-109]]"
verification_method: inspection
result: not_run
---

# TEST-071 — Failed Improvement Iteration Traceability Inspection

## Objective

Verify [[SYS-REQ-109]] by inspection of traceability across successive improvement iterations following a failed improvement.

## Preconditions

- At least one failed improvement iteration is available.
- Root-cause analysis and subsequent proposal records are available.

## Inspection Criteria

Verify the complete causal chain:

Improvement → Failure → Root Cause → Learning Event → New Proposal → New Justification → Validation

Verify that the new proposal references the previous iteration when it resulted from analysis of the previous failure.

Verify that the results of the previous attempt considered by the new solution can be identified.

Verify that iteration history remains available after rollback or creation of a new proposal.

## Expected Result

The complete causal relationship between the failed improvement and the subsequent iteration is traceable.

The new proposal, justification and validation can be linked to the preceding failure and root cause.

The results considered from the previous attempt are identifiable.

Previous iteration history remains preserved after rollback or creation of a new proposal.

## Evidence Required

- Original improvement record.
- Failure result.
- Root-cause analysis.
- Learning event.
- New proposal.
- Link to previous iteration.
- New justification.
- Validation record.
- Evidence of previous results considered.
- Preserved iteration history after rollback or new proposal.

## Result

Not Run

## Traceability
- [[SYS-REQ-109]]
- [[ARCH-029]]



