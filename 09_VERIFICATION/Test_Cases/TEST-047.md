---
id: TEST-047
type: test_case
title: Safety-Critical Priority Under Resource Contention Test
status: draft
verifies:
  - "[[SYS-REQ-085]]"
verification_method: test
result: not_run
---

# TEST-047 — Safety-Critical Priority Under Resource Contention Test

## Objective

Verify [[SYS-REQ-085]] by controlled resource contention between P0/P1 and lower-priority functions.

## Preconditions

- HUB resource-management mechanism is available.
- Representative P0/P1 functions are available.
- Representative P2–P5 functions are available.
- Controlled computational or communication resource contention can be generated.
- Function execution and resource allocation can be monitored.

## Test Procedure

1. Configure representative P0/P1 functions for safety, flight control, navigation and C2.
2. Configure representative P2–P5 functions.
3. Start the P0/P1 functions under nominal conditions.
4. Start the P2–P5 functions.
5. Generate controlled resource contention.
6. Verify that P0/P1 functions retain priority over P2–P5.
7. Increase the resource contention until lower-priority functions must be limited.
8. Verify that P2–P5 functions are degraded or restricted before P0/P1 functions.
9. Verify that safety, flight control, navigation and C2 functions remain executable.
10. Verify that lower-priority functions cannot prevent P0/P1 execution.
11. Record resource allocation and function behavior throughout the contention scenario.

## Expected Result

Under resource contention, P0 and P1 functions have priority over P2–P5.

P2–P5 functions are limited or degraded before P0/P1 functions.

Safety, flight control, navigation and C2 functions remain available and are not prevented by lower-priority functions.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- Priority configuration.
- Resource contention setup.
- P0/P1 execution records.
- P2–P5 execution records.
- Resource allocation measurements.
- Degradation/restriction records for lower-priority functions.
- Evidence of uninterrupted P0/P1 execution.

## Traceability
- [[SYS-REQ-085]]
- [[ARCH-028]]



