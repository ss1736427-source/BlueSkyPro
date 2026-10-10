---
id: CAPABILITY-ARCH-REVIEW-001
type: capability-architecture-review
status: draft
authority: ARCH-DEC-045
---

# Capability Architecture Review

## Classification

| CAP | Classification | Architectural Basis | Rationale |
|---|---|---|---|
| CAP-002 | TRANSITIVE | ARCH-DEC-003, ARCH-DEC-014 | Core interfaces and authoritative operational data architecture provide the architectural basis for perception inputs and outputs. |
| CAP-003 | TRANSITIVE | ARCH-DEC-003, ARCH-DEC-014, ARCH-DEC-017 | Core interfaces/data architecture and unified abnormal-condition handling provide supporting mechanisms for detection. |
| CAP-004 | TRANSITIVE | ARCH-DEC-003, ARCH-DEC-004, ARCH-DEC-014 | Mission state, Core data and defined interfaces provide supporting mechanisms for tracking state. |
| CAP-006 | TRANSITIVE | ARCH-DEC-004, ARCH-DEC-015, ARCH-DEC-038 | Mission, spatial architecture and execution orchestration provide the execution basis for inspection. |
| CAP-007 | TRANSITIVE | ARCH-DEC-004, ARCH-DEC-015, ARCH-DEC-038 | Mission, spatial architecture and execution orchestration provide the execution basis for search. |
| CAP-008 | TRANSITIVE | ARCH-DEC-014, ARCH-DEC-015, ARCH-DEC-041 | Centralized operational data, spatial architecture and execution evidence provide the architectural basis for fire monitoring. The capability itself depends on Navigation, Perception, Detection and Mapping. |
| CAP-009 | TRANSITIVE | ARCH-DEC-012, ARCH-DEC-036, ARCH-DEC-037, ARCH-DEC-038 | Fleet/resource coordination, scheduling and execution orchestration provide the architectural basis for delivery. |
| CAP-013 | TRANSITIVE | ARCH-DEC-003, ARCH-DEC-014, ARCH-DEC-015 | Core interfaces, operational data and spatial context support object detection. |
| CAP-016 | TRANSITIVE | ARCH-DEC-014, ARCH-DEC-020 | Centralized data architecture, telemetry and external-system integration provide the architectural basis for environmental observations and source-controlled environmental data. |
| CAP-017 | TRANSITIVE | ARCH-DEC-003, ARCH-DEC-008, ARCH-DEC-014, ARCH-DEC-015, ARCH-DEC-017, ARCH-DEC-039 | Existing interfaces, spatial context, conflict handling, abnormal-condition handling and runtime adaptation support the C-UAS situational-awareness capability. The capability explicitly does not authorize physical countermeasures. |

## Rule

TRANSITIVE relationships are accepted only as architectural support relationships and shall not be interpreted as explicit capability implementation.

CANDIDATE relationships require explicit architectural justification before promotion.

A capability GAP shall be declared only when the existing architecture cannot reasonably provide the required mechanism.

## Traceability

CAPABILITY → ARCH-DEC → SYS-REQ → COMPONENT / PLATFORM / PAYLOAD → INTERFACE → VERIFICATION → EVIDENCE

## Authority

ARCH-DEC-045 governs requirements and architecture traceability.
## SYS-REQ-036 Semantic Review

SYS-REQ-036 requires selection of UAV type and payload based on required capabilities, payload,
range, endurance, energy, communication, autonomy, operational constraints and safety.

The following relationships are accepted or supporting:

| SYS-REQ | ARCH-DEC | CAP | Assessment | Basis |
|---|---|---|---|---|
| SYS-REQ-036 | ARCH-DEC-012 | CAP-012 | ACCEPT | Fleet/resource allocation is explicitly capability-driven |
| SYS-REQ-036 | ARCH-DEC-012 | CAP-009 | SUPPORTING | Delivery is a resource/capability-dependent mission capability |
| SYS-REQ-036 | ARCH-DEC-015 | CAP-005 | ACCEPT | Mapping is explicitly part of the common spatial architecture |
| SYS-REQ-036 | ARCH-DEC-015 | CAP-014 | SUPPORTING | 3D reconstruction is a spatial capability supported by the architecture |

The remaining SYS-REQ-036 candidate relationships shall remain unaccepted unless explicit
architectural justification is established.

Keyword similarity, shared dependencies and transitive capability relationships shall not
constitute accepted coverage.




