---
id: TEST-065
type: test_case
title: Controlled AI Model Update and Rollback Test
status: draft
verifies:
  - "[[SYS-REQ-103]]"
verification_method: test
result: not_run
---

# TEST-065 — Controlled AI Model Update and Rollback Test

## Objective

Verify [[SYS-REQ-103]] by controlled update of an AI model, validation before operational use and rollback to the previous model version.

## Preconditions

- An AI model is available with a known identifier and version.
- A replacement model version is available.
- A controlled model-update mechanism is available.
- The applicable operational loop is available.

## Test Procedure

1. Record the current model identifier and version.
2. Initiate a controlled update to the replacement model.
3. Verify that the replacement model is identified and versioned.
4. Verify that the replacement model undergoes the required validation before use in the operational loop.
5. Verify that an unvalidated replacement model cannot enter the applicable operational loop.
6. Activate the validated replacement model.
7. Where rollback is required, initiate rollback to the previous model version.
8. Verify that the previous model version is restored.
9. Verify that the rollback is recorded as a separate event.

## Expected Result

Every AI model has a unique identifier and version.

Model updates are validated before use in the applicable operational loop.

Where rollback is required, the previous model version can be restored and the rollback is recorded as a separate event.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- Model identifier and version records.
- Model update record.
- Validation result before operational use.
- Evidence preventing unvalidated model activation.
- Previous model version.
- Rollback execution record.
- Separate rollback event record.

## Traceability
- [[SYS-REQ-103]]
- [[ARCH-029]]



