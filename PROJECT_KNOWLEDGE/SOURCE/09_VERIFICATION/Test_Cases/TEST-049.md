---
id: TEST-049
type: test_case
title: AI Resource Isolation Test
status: draft
verifies:
  - "[[SYS-REQ-087]]"
verification_method: test
result: not_run
---

# TEST-049 — AI Resource Isolation Test

## Objective

Verify [[SYS-REQ-087]] by controlled resource contention between AI/analytics processes and safety-critical functions.

## Preconditions

- HUB resource-management mechanism is available.
- AI, machine-learning or analytics workload is available.
- Representative flight control, navigation, C2 and safety functions are available.
- Resource reservation or isolation mechanism is available or observable.
- CPU, memory and relevant system-resource utilization can be monitored.

## Test Procedure

1. Start representative flight control, navigation, C2 and safety functions.
2. Record their initial resource utilization and operational state.
3. Start the AI/ML/analytics workload.
4. Verify that AI/analytics processes execute within their assigned resource contour.
5. Generate controlled resource contention between AI/analytics and safety-critical functions.
6. Verify that AI/analytics processes cannot consume resources reserved for flight control, navigation, C2 or safety.
7. Increase the resource contention until the AI workload competes for resources.
8. Verify that AI and analytics processes are automatically limited or suspended.
9. Verify that flight control, navigation, C2 and safety functions remain operational.
10. Verify that recovery of available resources does not require manual restoration of safety-critical functions.
11. Record resource allocation, AI degradation/suspension and safety-critical function behavior.

## Expected Result

AI, machine-learning, self-learning and analytics processes remain within an isolated resource contour.

AI processes cannot consume resources reserved for flight control, navigation, C2 or safety.

Under resource conflict, AI and analytics workloads are automatically restricted or suspended.

Flight control, navigation, C2 and safety functions remain operational.

## Actual Result

Not executed.

## Result

Not Run

## Evidence Required

- Resource isolation/reservation configuration.
- Initial resource utilization.
- AI/analytics workload measurements.
- Resource contention profile.
- AI throttling or suspension evidence.
- Flight-control availability record.
- Navigation availability record.
- C2 availability record.
- Safety-function availability record.

## Traceability
- [[SYS-REQ-087]]
- [[ARCH-028]]



