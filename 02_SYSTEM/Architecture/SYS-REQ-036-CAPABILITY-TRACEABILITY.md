---
id: SYS-REQ-036-CAPABILITY-TRACEABILITY
type: requirement-capability-traceability
status: draft
authority: ARCH-DEC-045
---

# SYS-REQ-036 — Capability Traceability

## Requirement

SYS-REQ-036 requires selection of an appropriate UAV type and payload based on task requirements, capabilities, payload, range, endurance, energy, communication, autonomy, operational constraints and safety.

## Accepted Coverage

| SYS-REQ | ARCH-DEC | CAP | Status | Basis |
|---|---|---|---|---|
| SYS-REQ-036 | ARCH-DEC-012 | CAP-012 | ACCEPTED | Capability-driven fleet/resource allocation explicitly selects resources providing required capabilities. |

## Supporting Coverage

| SYS-REQ | ARCH-DEC | CAP | Status | Basis |
|---|---|---|---|---|
| SYS-REQ-036 | ARCH-DEC-012 | CAP-009 | SUPPORTING | Payload assignment is explicitly part of resource allocation, but Delivery is not itself the selection mechanism. |
| SYS-REQ-036 | ARCH-DEC-015 | CAP-005 | SUPPORTING | Spatial information may support UAV/resource selection constraints, but Mapping is not itself the resource-selection mechanism. |
| SYS-REQ-036 | ARCH-DEC-015 | CAP-014 | SUPPORTING | Spatial/3D information can support resource and mission planning, but is not itself the selection mechanism. |

## Unaccepted Candidates

The following candidates remain unaccepted because the available architectural evidence establishes only transitive or indirect relationships:

- CAP-001
- CAP-006
- CAP-007
- CAP-008
- CAP-013
- CAP-015
- CAP-016
- CAP-017

## Traceability Rule

Candidate relationships generated through SYS-REQ → ARCH-DEC → CAPABILITY propagation shall not be promoted to accepted coverage without explicit semantic architectural justification.

## Traceability Chain

SYS-REQ-036 → ARCH-DEC-012 / ARCH-DEC-015 → CAPABILITY → COMPONENT / PLATFORM / PAYLOAD → INTERFACE → VERIFICATION → EVIDENCE

## Authority

ARCH-DEC-045 governs requirements and architecture traceability.


