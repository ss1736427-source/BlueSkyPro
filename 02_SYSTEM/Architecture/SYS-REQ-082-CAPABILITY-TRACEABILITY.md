---
id: SYS-REQ-082-CAPABILITY-TRACEABILITY
type: system-requirement-capability-traceability
status: draft
authority: ARCH-DEC-045
---

# SYS-REQ-082 — Capability Traceability

## Traceability

| SYS-REQ | ARCH-DEC | CAP | Status | Basis |
|---|---|---|---|---|
| SYS-REQ-082 | ARCH-DEC-017 | CAP-001 | SUPPORTING | Emergency return and controlled landing behaviour depends on applicable navigation capability. |
| SYS-REQ-082 | ARCH-DEC-017 | CAP-009 | SUPPORTING | Delivery may be an affected mission task requiring controlled interruption or completion during safe mission termination. |
| SYS-REQ-082 | ARCH-DEC-017 | CAP-012 | SUPPORTING | Recovery and contingency architecture integrates fleet coordination and resource reassignment. |
| SYS-REQ-082 | ARCH-DEC-017 | CAP-015 | SUPPORTING | Communication state is an explicit recovery concern and may be required for position reporting and recovery coordination. |
| SYS-REQ-082 | ARCH-DEC-017 | CAP-019 | SUPPORTING | Obstacle avoidance may support safe return or emergency landing but does not define the completion policy. |

## Architecture-Only Mechanism

ARCH-DEC-017 explicitly defines the emergency completion mechanism:

EMERGENCY CONDITION
→ EVALUATE SAFE RETURN
→ RETURN TO OPERATOR / APPLICABLE RETURN LOCATION
→ IF RETURN IMPOSSIBLE OR UNSAFE
→ PRE-PLANNED LANDING POINT
→ VALIDATE
→ SAFETY GATE
→ LAND

Where safe return and controlled landing are unavailable, the architecture supports applicable emergency descent behaviour including parachute deployment where available, propulsion transition to a safe state, position preservation/transmission and emergency beacon activation.

| SYS-REQ-082 | ARCH-DEC-016 | - | SUPPORTING | Safety Architecture and Safety Gate provide authoritative safety control for emergency completion and prevent unsafe continuation. |
| SYS-REQ-082 | ARCH-DEC-038 | - | SUPPORTING | Mission Execution Orchestrator coordinates execution state, UAV assignments, resources and applicable abort/recovery actions during safe mission completion. |
## Coverage Assessment

The requirement is architecturally covered by ARCH-DEC-017. Capability relationships are supporting relationships; the emergency completion policy itself is defined by the architecture decision and system requirement.

## Authority

ARCH-DEC-045 governs requirements and architecture traceability.


