---
id: ARCH-DEC-045
type: architecture_decision
title: System Requirements Traceability and Architecture Coverage
status: accepted
scope: BlueSky System
---

# ARCH-DEC-045 — System Requirements Traceability and Architecture Coverage

## Decision

BlueSky shall maintain explicit bidirectional traceability between system requirements and architectural decisions.

Every accepted system requirement shall be traceable to one or more architectural decisions, and every architectural decision shall identify the requirements, constraints or architectural drivers that justify it.

## Traceability Chain

The architecture shall support the chain:

SYS-REQ
→ ARCH-DEC
→ ARCHITECTURE DOMAIN
→ AUTHORITY / COMPONENT
→ INTERFACE
→ VERIFICATION
→ EVIDENCE.

## Requirement Identity

Every requirement shall retain its stable SYS-REQ identity.

Requirement identity shall not be replaced by title, description or implementation identifier.

## Architecture Decision Identity

Every architectural decision shall retain a stable ARCH-DEC identity.

Architecture decision identities shall not be silently reused.

## Bidirectional Traceability

Traceability shall work in both directions:

SYS-REQ → ARCH-DEC

and

ARCH-DEC → SYS-REQ.

## Coverage States

Architecture coverage shall explicitly classify each requirement as:

- COVERED;
- PARTIAL;
- NOT_COVERED;
- DEFERRED;
- NOT_APPLICABLE.

## Covered

COVERED means the architectural decision defines the architectural mechanism, boundary or constraint required to satisfy the requirement.

## Partial

PARTIAL means the architecture addresses the requirement but additional architecture, detailed design or implementation decisions remain necessary.

## Not Covered

NOT_COVERED means no accepted architectural decision currently establishes how the requirement is satisfied.

## Deferred

DEFERRED means the requirement is intentionally postponed by an explicit architectural decision.

Deferred status shall include rationale and the condition for reopening the decision.

## Not Applicable

NOT_APPLICABLE shall only be used when an explicit architectural rationale establishes why the requirement does not apply.

## No Implicit Coverage

Conceptual similarity shall not be sufficient to claim requirement coverage.

A requirement shall not be considered covered merely because an architecture document contains similar terminology.

## Explicit References

Architecture decisions shall contain explicit SYS-REQ references where the decision materially addresses a requirement.

## Requirement-to-Decision Mapping

A requirement may map to multiple architecture decisions.

An architecture decision may satisfy multiple requirements.

The relationship shall therefore be many-to-many.

## Primary Decision

Where several decisions contribute to one requirement, one decision may be identified as the primary architectural decision.

Supporting decisions shall remain traceable.

## Architectural Domain

Each traceability relationship shall identify the applicable architectural domain where practical.

Examples include:

- Mission;
- Mission AI;
- Safety;
- Autonomy;
- Planning;
- Scheduling;
- Execution;
- Recovery;
- Data;
- Configuration;
- Resources;
- Communication;
- C2;
- Fleet;
- UI;
- Simulation;
- Audit;
- Observability;
- Security;
- Learning;
- External Integration.

## Authority Traceability

Safety-critical and operationally authoritative requirements shall trace to the authority boundary responsible for satisfying them.

## Component Traceability

Where architecture identifies a responsible component, service or subsystem, that relationship shall be traceable.

## Interface Traceability

Requirements involving interaction between components shall trace to the applicable interface or contract.

## Verification Traceability

Each requirement shall identify its applicable verification path.

Verification may include:

- analysis;
- inspection;
- review;
- test;
- simulation;
- operational validation.

## Evidence Traceability

Verification results shall produce identifiable evidence.

Evidence shall retain the requirement and architecture identities to which it applies.

## Requirement Change

A requirement change shall trigger impact analysis against linked architecture decisions.

## Architecture Change

An architecture decision change shall trigger impact analysis against linked requirements.

## No Silent Change

Changing the coverage relationship without explicit change control is prohibited.

## Baseline Binding

Accepted requirements and accepted architecture decisions shall be associated with identifiable baselines or revisions.

## Version Traceability

Traceability shall preserve the applicable requirement and architecture versions.

## Configuration Binding

Where architecture behavior depends on configuration, traceability shall identify the applicable configuration binding.

## Mission Binding

Where a requirement affects mission behavior, traceability shall preserve the applicable mission model or mission revision.

## Safety Binding

Safety-critical requirements shall retain explicit traceability to safety constraints and safety verification.

## Approval Binding

Requirements affecting approval boundaries shall trace to the applicable human approval authority.

## Execution Binding

Requirements affecting execution shall trace to the execution authority and execution gates.

## Simulation Binding

Requirements verified through simulation shall identify the applicable simulation model or simulation configuration.

## Planned / Simulated / Actual

Traceability shall distinguish:

PLANNED
SIMULATED
ACTUAL.

Evidence from one state shall not silently be presented as evidence for another state.

## External Integration

Requirements involving external systems shall trace through the corresponding adapter or integration boundary.

External systems shall not become architectural authority merely because they participate in a requirement.

## UI Boundary

UI requirements shall trace to the UI boundary and the underlying authoritative Core mechanism.

The UI shall not become the sole authorization mechanism for safety-critical behavior.

## Mission AI Boundary

Mission AI requirements shall trace to Mission AI boundaries and to the authority that accepts or rejects AI outputs.

AI recommendations shall not automatically become authoritative execution decisions.

## FPV Extension

FPV functionality shall remain traceable as an extension/capability rather than silently becoming a mandatory architectural dependency.

FPV-related requirements introduced later shall be mapped to the existing FPV extension and authority boundaries.

## Coverage Validation

The architecture process shall periodically validate that every accepted SYS-REQ has an explicit coverage state.

## Coverage Metrics

The architecture repository shall be able to report:

- total requirements;
- covered requirements;
- partial requirements;
- uncovered requirements;
- deferred requirements;
- not-applicable requirements.

## Coverage Integrity

Coverage metrics shall be reproducible from repository content.

Manual claims of coverage shall not be treated as authoritative evidence.

## Gap Detection

Uncovered or partial requirements shall be identifiable without manually inspecting every architecture decision.

## Architecture Gap

A material uncovered requirement shall result in one of:

- a new architecture decision;
- an update to an existing architecture decision;
- an explicit deferral;
- an explicit not-applicable decision.

## Duplicate Architecture

Multiple architecture decisions addressing the same architectural concern shall identify their relationship.

The system shall avoid silently creating contradictory architectural authorities.

## Conflict Detection

Conflicting architecture decisions shall be explicitly identified and resolved through change control.

## Supersession

When an architecture decision supersedes another decision, the relationship shall be explicit.

The superseded decision shall remain traceable for historical reconstruction.

## Historical Traceability

Traceability shall preserve historical relationships required to reconstruct why an architectural decision was made.

## Reproducibility

A reviewer shall be able to reconstruct:

REQUIREMENT
→ DECISION
→ RATIONALE
→ ARCHITECTURAL MECHANISM
→ VERIFICATION
→ EVIDENCE.

## Audit

Changes to requirement-to-architecture relationships shall be auditable.

## Observability

Traceability processing shall expose failures such as:

- missing requirement;
- missing decision;
- invalid reference;
- broken relationship;
- stale relationship;
- conflicting relationship.

## Security

Traceability data shall respect access-control requirements.

Security-sensitive architecture information shall not be exposed beyond authorized boundaries.

## Performance

Traceability validation shall support the current repository scale without requiring manual document-by-document inspection.

## Scalability

The traceability model shall support growth in:

- requirements;
- architecture decisions;
- components;
- interfaces;
- verification artifacts;
- evidence.

## Verification

Verification shall confirm that:

- every accepted requirement has a coverage state;
- every COVERED requirement has an explicit architecture reference;
- PARTIAL requirements identify remaining architectural work;
- DEFERRED requirements have rationale;
- NOT_APPLICABLE requirements have rationale;
- architecture decisions identify their drivers;
- broken references are detected;
- superseded decisions remain historically traceable.

## Negative Testing

Verification shall confirm that:

- a missing SYS-REQ reference cannot be reported as COVERED;
- a deleted ARCH-DEC reference is detected;
- duplicate decision identity is detected;
- contradictory decisions are detectable;
- stale relationships are detectable;
- an unverified requirement cannot be represented as fully verified;
- simulation evidence cannot silently satisfy ACTUAL evidence;
- FPV extension cannot silently become an execution authority.

## Rationale

Explicit requirements-to-architecture traceability prevents architectural gaps from being hidden by terminology or document structure.

The model establishes a reproducible chain from system intent to architecture, implementation boundaries, verification and evidence while preserving authority, versioning, safety and historical decision context.

It also allows the architecture to continue growing without losing control of the relationship between the original SYS-REQ baseline and subsequent ARCH-DEC decisions.


