---
id: ARCH-DEC-CAPABILITY-COVERAGE-001
type: architecture-capability-coverage
status: draft
version: 1.0.0
authority: ARCH-DEC-045
---

# ARCH-DEC → CAPABILITY Coverage

## Decision Basis

This matrix maps architecture decisions to the existing BlueSky capability registry.
A capability relationship is not accepted solely because of terminology similarity.

Coverage states:

- PARTIAL
- NOT_APPLICABLE

PARTIAL indicates that the capability contributes to the architectural mechanism but does not by itself constitute complete architectural implementation.

NOT_APPLICABLE indicates that the architecture decision is a system-level mechanism for which no UAV operational capability is required from CAP-001…CAP-020.

## Coverage

| ARCH-DEC | Capability References | Coverage |
|---|---|---|
| ARCH-DEC-001 | - | NOT_APPLICABLE |
| ARCH-DEC-002 | - | NOT_APPLICABLE |
| ARCH-DEC-003 | CAP-001, CAP-002, CAP-003, CAP-004, CAP-013, CAP-015, CAP-017, CAP-018 | PARTIAL |
| ARCH-DEC-004 | CAP-004, CAP-006, CAP-007 | NOT_APPLICABLE |
| ARCH-DEC-005 | - | NOT_APPLICABLE |
| ARCH-DEC-006 | - | NOT_APPLICABLE |
| ARCH-DEC-007 | - | NOT_APPLICABLE |
| ARCH-DEC-008 | CAP-001, CAP-003, CAP-017, CAP-019 | PARTIAL |
| ARCH-DEC-009 | - | NOT_APPLICABLE |
| ARCH-DEC-010 | - | NOT_APPLICABLE |
| ARCH-DEC-011 | - | NOT_APPLICABLE |
| ARCH-DEC-012 | CAP-009, CAP-012 | PARTIAL |
| ARCH-DEC-013 | CAP-015 | PARTIAL |
| ARCH-DEC-014 | CAP-002, CAP-003, CAP-004, CAP-008, CAP-013, CAP-016, CAP-017, CAP-018, CAP-020 | PARTIAL |
| ARCH-DEC-015 | CAP-001, CAP-006, CAP-007, CAP-008, CAP-013, CAP-014, CAP-017, CAP-005 | PARTIAL |
| ARCH-DEC-016 | - | NOT_APPLICABLE |
| ARCH-DEC-017 | CAP-001, CAP-003, CAP-012, CAP-015, CAP-017, CAP-019 | PARTIAL |
| ARCH-DEC-018 | - | NOT_APPLICABLE |
| ARCH-DEC-019 | - | NOT_APPLICABLE |
| ARCH-DEC-020 | CAP-015, CAP-016 | PARTIAL |
| ARCH-DEC-021 | - | NOT_APPLICABLE |
| ARCH-DEC-022 | - | NOT_APPLICABLE |
| ARCH-DEC-023 | CAP-010, CAP-011 | PARTIAL |
| ARCH-DEC-024 | - | NOT_APPLICABLE |
| ARCH-DEC-025 | - | NOT_APPLICABLE |
| ARCH-DEC-026 | - | NOT_APPLICABLE |
| ARCH-DEC-027 | - | NOT_APPLICABLE |
| ARCH-DEC-028 | - | NOT_APPLICABLE |
| ARCH-DEC-029 | CAP-020 | PARTIAL |
| ARCH-DEC-030 | CAP-010, CAP-011 | PARTIAL |
| ARCH-DEC-031 | CAP-010, CAP-011 | PARTIAL |
| ARCH-DEC-032 | - | NOT_APPLICABLE |
| ARCH-DEC-033 | CAP-020 | PARTIAL |
| ARCH-DEC-034 | - | NOT_APPLICABLE |
| ARCH-DEC-035 | - | NOT_APPLICABLE |
| ARCH-DEC-036 | CAP-009, CAP-012 | PARTIAL |
| ARCH-DEC-037 | CAP-009, CAP-012 | PARTIAL |
| ARCH-DEC-038 | CAP-001, CAP-006, CAP-007, CAP-009, CAP-012 | PARTIAL |
| ARCH-DEC-039 | CAP-001, CAP-012, CAP-017, CAP-019 | PARTIAL |
| ARCH-DEC-040 | - | NOT_APPLICABLE |
| ARCH-DEC-041 | CAP-008, CAP-020 | PARTIAL |
| ARCH-DEC-042 | - | NOT_APPLICABLE |
| ARCH-DEC-043 | - | NOT_APPLICABLE |
| ARCH-DEC-044 | - | NOT_APPLICABLE |
| ARCH-DEC-045 | - | NOT_APPLICABLE |

## Capability Registry Basis

| CAP-ID | Capability |
|---|---|
| CAP-001 | Navigation |
| CAP-002 | Perception |
| CAP-003 | Detection |
| CAP-004 | Tracking |
| CAP-005 | Mapping |
| CAP-006 | Inspection |
| CAP-007 | Search |
| CAP-008 | Fire Monitoring |
| CAP-009 | Delivery |
| CAP-010 | Drone-in-a-Box |
| CAP-011 | BVLOS |
| CAP-012 | Fleet Coordination |
| CAP-013 | Object Detection |
| CAP-014 | 3D Reconstruction |
| CAP-015 | Communication Relay |
| CAP-016 | Environmental Monitoring |
| CAP-017 | C-UAS |
| CAP-018 | Sensor Fusion |
| CAP-019 | Obstacle Avoidance |
| CAP-020 | Mission Reporting |

## Traceability Chain

SYS-REQ → ARCH-DEC → CAPABILITY → COMPONENT / PLATFORM / PAYLOAD → INTERFACE → VERIFICATION → EVIDENCE

## Authority

ARCH-DEC-045 governs requirements traceability and architecture coverage.


