---
id: SYS-REQ-007-CAPABILITY-TRACEABILITY
type: system-requirement-capability-traceability
status: draft
authority: ARCH-DEC-045
---

# SYS-REQ-007 — Capability Traceability

## Traceability

| SYS-REQ | ARCH-DEC | CAP | Status | Basis |
|---|---|---|---|---|
| SYS-REQ-007 | ARCH-DEC-017 | CAP-001 | SUPPORTING | Navigation capability provides the navigation basis required to validate and execute applicable emergency return and landing behaviour. |
| SYS-REQ-007 | ARCH-DEC-017 | CAP-015 | SUPPORTING | Communication capability supports C2 and position reporting relevant to emergency recovery. |
| SYS-REQ-007 | ARCH-DEC-017 | CAP-019 | SUPPORTING | Obstacle avoidance may contribute to safe return and landing, subject to the applicable UAV capability and safety constraints. |

## Architecture Mechanism

ARCH-DEC-017 defines the emergency recovery architecture used by validation:

EMERGENCY CONDITION
→ EVALUATE SAFE RETURN
→ RETURN TO OPERATOR / APPLICABLE RETURN LOCATION
→ IF RETURN IMPOSSIBLE OR UNSAFE
→ PRE-PLANNED LANDING POINT
→ VALIDATE
→ SAFETY GATE
→ LAND

Where applicable, the architecture also supports emergency descent with parachute deployment, propulsion transition to a safe state, position preservation/transmission and emergency beacon activation.

SYS-REQ-007 requires Individual Validation to verify the availability, configuration and applicability of these emergency recovery mechanisms for the selected UAV.

## Coverage Assessment

SYS-REQ-007 is architecturally covered by ARCH-DEC-017 for the emergency recovery validation mechanism. Detailed UAV-specific implementation and verification remain subject to the applicable UAV configuration.

## Authority

ARCH-DEC-045 governs requirements and architecture traceability.


