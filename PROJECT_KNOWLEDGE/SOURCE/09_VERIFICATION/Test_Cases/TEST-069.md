---
id: TEST-069
type: test_case
title: Rollback Applicability and Acceptance Test
status: draft
verifies:
  - "[[SYS-REQ-107]]"
verification_method: test
result: not_run
---

# TEST-069 — Rollback Applicability and Acceptance Test

## Objective

Verify [[SYS-REQ-107]] by controlled rollback execution and confirmation of restoration criteria.

## Preconditions

- A significant change with a defined rollback policy is available.
- A known original version is available.
- Rollback execution mechanism is available.

## Test Procedure

1. Verify that rollback applicability is explicitly defined for the change.
2. Record the original version.
3. Verify that rollback trigger conditions are defined.
4. Verify that successful-restoration criteria are defined.
5. Verify that the rollback recovery sequence is defined.
6. Verify that rollback completion criteria are defined.
7. Trigger rollback under the defined conditions.
8. Execute the defined recovery sequence.
9. Verify restoration against the required-state criteria.
10. Verify that rollback completion is explicitly determined.
11. Verify that rollback is recorded as a separate lifecycle event.

## Expected Result

Each significant change has a defined rollback policy.

Where rollback is applicable, the original version, trigger conditions, restoration criteria, recovery sequence and completion criteria are defined.

Rollback restores the required system state and the restoration is explicitly confirmed.

Rollback is recorded as a separate lifecycle event.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- Rollback policy.
- Original version.
- Rollback trigger conditions.
- Restoration criteria.
- Recovery sequence.
- Rollback completion criteria.
- Restoration confirmation.
- Rollback lifecycle event.

## Traceability
- [[SYS-REQ-107]]
- [[ARCH-029]]



