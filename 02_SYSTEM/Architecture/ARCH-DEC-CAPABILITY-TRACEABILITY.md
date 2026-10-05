---
id: ARCH-CAP-TRACE-001
type: architecture-capability-traceability
status: draft
version: 1.0.0
authority: ARCH-DEC-045
---

# ARCH-DEC → CAPABILITY Traceability

## Decision Basis

This document records candidate architecture-to-capability relationships.
A CANDIDATE relationship is supported by capability dependencies/interfaces and an existing architecture mechanism.
CANDIDATE does not constitute accepted requirements traceability.

## Traceability

| CAP-ID | Capability | ARCH-DEC References | Status | Evidence Basis |
|---|---|---|---|---|
| CAP-001 | Navigation | ARCH-DEC-003, ARCH-DEC-008, ARCH-DEC-015, ARCH-DEC-017, ARCH-DEC-038, ARCH-DEC-039 | CANDIDATE | Existing capability coverage matrix |
| CAP-002 | Perception | ARCH-DEC-003, ARCH-DEC-014 | CANDIDATE | Dependencies / Interfaces |
| CAP-003 | Detection | ARCH-DEC-003, ARCH-DEC-010, ARCH-DEC-014 | CANDIDATE | Dependencies / Interfaces |
| CAP-004 | Tracking | ARCH-DEC-003, ARCH-DEC-004, ARCH-DEC-010, ARCH-DEC-014 | CANDIDATE | Dependencies / Interfaces |
| CAP-005 | Mapping | ARCH-DEC-015 | CANDIDATE | Existing capability coverage matrix |
| CAP-006 | Inspection | ARCH-DEC-004, ARCH-DEC-015, ARCH-DEC-038 | CANDIDATE | Dependencies / Interfaces |
| CAP-007 | Search | ARCH-DEC-004, ARCH-DEC-015, ARCH-DEC-038 | CANDIDATE | Dependencies / Interfaces |
| CAP-008 | Fire Monitoring | ARCH-DEC-014, ARCH-DEC-015, ARCH-DEC-041 | CANDIDATE | Dependencies / Interfaces |
| CAP-009 | Delivery | ARCH-DEC-012, ARCH-DEC-036, ARCH-DEC-037, ARCH-DEC-038 | CANDIDATE | Dependencies / Interfaces |
| CAP-010 | Drone-in-a-Box | ARCH-DEC-023, ARCH-DEC-030, ARCH-DEC-031 | CANDIDATE | Existing capability coverage matrix |
| CAP-011 | BVLOS | ARCH-DEC-023, ARCH-DEC-030, ARCH-DEC-031 | CANDIDATE | Existing capability coverage matrix |
| CAP-012 | Fleet Coordination | ARCH-DEC-012, ARCH-DEC-036, ARCH-DEC-037, ARCH-DEC-038, ARCH-DEC-039 | CANDIDATE | Existing capability coverage matrix |
| CAP-013 | Object Detection | — | CANDIDATE | No explicit architecture mechanism established yet |
| CAP-014 | 3D Reconstruction | ARCH-DEC-015 | CANDIDATE | Existing capability coverage matrix |
| CAP-015 | Communication Relay | ARCH-DEC-003, ARCH-DEC-013, ARCH-DEC-017, ARCH-DEC-020 | CANDIDATE | Existing capability coverage matrix |
| CAP-016 | Environmental Monitoring | — | CANDIDATE | No explicit architecture mechanism established yet |
| CAP-017 | C-UAS | — | CANDIDATE | No explicit architecture mechanism established yet |
| CAP-018 | Sensor Fusion | ARCH-DEC-003, ARCH-DEC-014 | CANDIDATE | Existing capability coverage matrix |
| CAP-019 | Obstacle Avoidance | ARCH-DEC-008, ARCH-DEC-017, ARCH-DEC-039 | CANDIDATE | Existing capability coverage matrix |
| CAP-020 | Mission Reporting | ARCH-DEC-014, ARCH-DEC-029, ARCH-DEC-033, ARCH-DEC-041 | CANDIDATE | Existing capability coverage matrix |

## Status Rules

- CANDIDATE = architecture mechanism appears applicable, but formal capability traceability is not yet accepted.
- ACCEPTED = relationship explicitly confirmed by architecture and capability evidence.
- GAP = capability requires an architecture mechanism not currently established.

## Traceability Chain

SYS-REQ → ARCH-DEC → CAPABILITY → COMPONENT / PLATFORM / PAYLOAD → INTERFACE → VERIFICATION → EVIDENCE

## Authority

ARCH-DEC-045 governs requirements traceability and architecture coverage.


