---
id: TEST-023
type: test_case
title: HUB Automatic Failover Test
status: draft
verifies:
  - "[[SYS-REQ-061]]"
verification_method: test
result: not_run
---

# TEST-023 — HUB Automatic Failover Test

## Objective

Verify [[SYS-REQ-061]] by controlled failover testing.

## Preconditions

- HUB-A is active.
- HUB-B is available as the redundant instance.
- State synchronization is operational.
- Functional modules are connected through the logical HUB Service.
- A controlled method for simulating confirmed HUB-A failure is available.

## Test Procedure

1. Start HUB-A as the active HUB instance.
2. Confirm normal operation of critical functions.
3. Confirm synchronization of critical state to HUB-B.
4. Introduce a controlled failure of HUB-A.
5. Observe automatic failure detection.
6. Observe automatic failover to HUB-B.
7. Verify that functional modules do not require manual reconnection.
8. Verify continuation of critical functions.
9. Verify preservation of the required critical state.

## Expected Result

The system automatically detects the confirmed failure of the active HUB and switches to the redundant instance.

Functional modules continue operating without manual reconnection.

Critical functions continue operating and the required critical state is preserved.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- Failure injection record.
- Failover detection timestamp.
- Active/redundant HUB state before and after failure.
- Functional module connection state.
- Critical state before and after failover.
- Test logs.

## Traceability
- [[SYS-REQ-061]]
- [[ARCH-025]]



