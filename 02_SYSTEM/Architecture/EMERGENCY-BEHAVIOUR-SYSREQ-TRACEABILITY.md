---
id: SYS-REQ-017-EMERGENCY-TRACEABILITY
type: system-requirement-architecture-traceability
status: draft
authority: ARCH-DEC-045
---

# Emergency Behaviour — System Requirement Traceability

## Traceability

| SYS-REQ | ARCH-DEC | Status | Basis |
|---|---|---|---|
| SYS-REQ-002 | ARCH-DEC-017 | SUPPORTING | Mission-template adaptation shall account for applicable emergency scenarios and use the recovery behaviour defined by the architecture. |
| SYS-REQ-004 | ARCH-DEC-017 | SUPPORTING | Mission Graph shall represent applicable emergency recovery transitions and actions defined by the emergency behaviour architecture. |
| SYS-REQ-007 | ARCH-DEC-017 | SUPPORTING | Mission Validation shall verify the applicable emergency recovery configuration and conditions for the selected UAV. |
| SYS-REQ-011 | ARCH-DEC-017 | SUPPORTING | Simulation / Digital Twin shall exercise applicable emergency recovery behaviour using the same mission and safety mechanisms as real execution. |
| SYS-REQ-082 | ARCH-DEC-017 | PRIMARY | Safe Mission Completion defines the required priority of emergency completion behaviour. |

## Emergency Behaviour Chain

SAFE RETURN
→ RETURN TO OPERATOR / CONFIGURED RETURN LOCATION
→ IF RETURN IMPOSSIBLE OR UNSAFE
→ PRE-PLANNED / VALIDATED LANDING POINT
→ IF SAFE LANDING IMPOSSIBLE
→ EMERGENCY DESCENT / RECOVERY

Where applicable:

- parachute deployment;
- propulsion transition to a safe state;
- position preservation/transmission;
- emergency beacon activation.

## Responsibility

SYS-REQ-082 defines the required safe-completion priority.

SYS-REQ-007 defines validation of the applicable emergency recovery configuration.

SYS-REQ-004 provides Mission Graph representation of applicable emergency behaviour.

SYS-REQ-011 provides Simulation / Digital Twin verification context.

SYS-REQ-002 ensures applicable emergency scenarios are considered during mission-template adaptation.

ARCH-DEC-017 defines the common architectural recovery mechanism and safety boundaries.

## Coverage Assessment

The emergency behaviour is architecturally represented by ARCH-DEC-017. The referenced system requirements provide complementary planning, representation, validation, simulation and safe-completion obligations.


