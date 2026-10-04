---
id: TEST-073
type: test_case
title: AI Agent Authority and Proposal Control Test
status: draft
verifies:
  - "[[SYS-REQ-111]]"
verification_method: test
result: not_run
---

# TEST-073 — AI Agent Authority and Proposal Control Test

## Objective

Verify [[SYS-REQ-111]] by demonstrating that AI-generated proposals cannot bypass validation, Safety Gate, authorization or authoritative C++ Core state ownership.

## Preconditions

- AI proposal path is available.
- Mission Validation is available.
- Safety Gate is available.
- Authorization mechanism is available.
- Authoritative C++ Core mission state is observable.
- Operator approval control is available where required.

## Test Procedure

1. Generate a material AI proposal that requests a controlled mission-state change.
2. Verify that the proposal receives a unique identity and records its source agent, task/context and proposed changes.
3. Attempt to apply the proposal without Mission Validation and verify that execution is blocked.
4. Attempt to apply the proposal with failed safety validation and verify that the Safety Gate blocks execution.
5. Where operator approval is required, attempt execution without approval and verify that execution is blocked.
6. Verify that rejected proposals do not become executable or authoritative state.
7. Submit two conflicting proposals and verify that the system does not select one solely from AI confidence or optimization score.
8. Execute an authorized proposal through the complete path: PROPOSAL → VALIDATION → SAFETY GATE → AUTHORIZATION → C++ CORE → EXECUTION.
9. Verify that the resulting authoritative state change occurs only after the required controls.
10. Verify that the material decision history is reconstructable from request through result.

## Expected Result

AI analysis and proposals remain non-authoritative until all applicable controls are satisfied.

Mission Validation, Safety Gate and required operator approval cannot be bypassed.

Rejected or conflicting proposals do not become executable state.

An authorized proposal is applied only through the defined controlled execution path.

Material AI decision history remains traceable and auditable.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- Proposal identity and source record.
- Validation result.
- Safety Gate result.
- Operator approval or rejection record where applicable.
- Rejection/blocking record.
- Conflicting-proposal resolution record.
- Authorized execution record.
- Before/after authoritative-state record.
- End-to-end decision trace.

## Traceability

- [[SYS-REQ-111]]
- [[ARCH-DEC-046]]
- [[SYS-REQ-085]]
- [[SYS-REQ-086]]
- [[SYS-REQ-110]]
