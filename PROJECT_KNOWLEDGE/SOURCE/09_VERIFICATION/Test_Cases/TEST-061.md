---
id: TEST-061
type: test_case
title: Controlled Improvement Deployment Test
status: draft
verifies:
  - "[[SYS-REQ-099]]"
verification_method: test
result: not_run
---

# TEST-061 — Controlled Improvement Deployment Test

## Objective

Verify [[SYS-REQ-099]] by controlled deployment of an approved improvement and inspection of deployment records and rollback behavior.

## Preconditions

- An approved improvement is available.
- A known baseline version is available.
- A controlled deployment mechanism is available.
- Rollback policy is defined where applicable.

## Test Procedure

1. Record the baseline version before deployment.
2. Initiate controlled deployment of the approved improvement.
3. Verify that the deployed version is recorded.
4. Verify that a unique deployment identifier is recorded.
5. Verify that the source baseline is recorded.
6. Verify that deployment status is recorded and updated.
7. Where rollback is required by policy, initiate a controlled rollback.
8. Verify that the rollback mechanism returns the system to the defined baseline.
9. Verify that the deployment and rollback records remain available.

## Expected Result

The approved improvement is deployed through a controlled mechanism with version, deployment ID, source baseline and deployment status recorded.

Where rollback is required, the rollback mechanism is available, executable and successfully restores the defined baseline.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- Baseline version.
- Deployed version.
- Deployment ID.
- Deployment status.
- Deployment record.
- Rollback execution and result, where applicable.

## Traceability
- [[SYS-REQ-099]]
- [[ARCH-029]]



