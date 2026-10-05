---
id: ARCH-DEC-035
type: architecture_decision
title: Configuration Feature Flags and Runtime Capability Management Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-035 — Configuration / Feature Flags / Runtime Capability Management Architecture

## Decision

BlueSky shall manage system configuration, capability availability and feature activation through controlled, versioned and traceable configuration mechanisms.

Configuration shall be separated from executable authority.

Feature flags shall control feature availability but shall not bypass Safety Engine, Validation, Readiness, Approval or Execution Authority.

## Configuration Authority

The C++ Core shall consume authoritative configuration.

QML shall display configuration and request authorized configuration changes through controlled interfaces.

QML shall not directly modify protected configuration storage.

## Configuration Domains

Configuration shall support:

- system;
- mission;
- UAV;
- fleet;
- C2;
- communication;
- map;
- terrain;
- airspace;
- geofence;
- weather;
- simulation;
- sandbox;
- Mission AI;
- optimization;
- safety;
- validation;
- readiness;
- deployment;
- security;
- observability.

## Configuration Identity

Every authoritative configuration shall have:

- configuration ID;
- version;
- revision;
- source;
- creation time;
- effective time where applicable;
- author;
- approval state where applicable.

## Configuration Versioning

Configuration changes shall create identifiable revisions.

Material changes shall not silently modify an existing approved configuration.

## Immutable Baselines

Approved and released configurations shall be immutable.

Changes shall create a new revision or version.

## Configuration Binding

Mission execution shall reference the applicable configuration baseline.

Simulation shall record the configuration used.

Actual execution shall record the configuration used.

## Mission Configuration

Mission configuration shall include applicable:

- mission parameters;
- capabilities;
- UAV assignments;
- constraints;
- optimization profile;
- environment;
- safety configuration;
- external data references.

## UAV Configuration

UAV configuration shall identify applicable:

- vehicle type;
- capabilities;
- limits;
- software;
- interface version;
- payload configuration;
- safety parameters.

## Fleet Configuration

Fleet configuration shall identify:

- UAV membership;
- roles;
- resource constraints;
- communication configuration;
- coordination parameters.

## Capability Model

The system shall maintain an explicit capability model.

Capabilities may include:

- navigation;
- perception;
- detection;
- tracking;
- mapping;
- payload;
- communication relay;
- fleet coordination;
- simulation;
- Mission AI;
- FPV;
- external integrations.

## Capability Identity

Each capability shall have an identifiable identity and applicable version.

## Capability Availability

Capability availability shall distinguish:

- configured;
- available;
- enabled;
- operational;
- degraded;
- unavailable;
- blocked.

Configured capability shall not automatically mean operational capability.

## Feature Flags

Feature flags may control optional functionality.

Feature flags shall have:

- flag identity;
- owner;
- scope;
- default state;
- effective configuration;
- audit history.

## Feature Flag Authority

Feature flags shall not:

- bypass safety rules;
- bypass validation;
- bypass readiness;
- bypass approval;
- grant execution authority;
- directly control UAVs.

## Safety-Critical Features

Safety-critical features shall not be disabled through ordinary runtime feature flags unless explicitly authorized by the Safety Architecture.

## Experimental Features

Experimental capabilities shall be isolated from operational baseline according to deployment and environment rules.

Experimental functionality shall not silently become operational.

## Environment Binding

Feature availability shall be evaluated against environment:

- DEVELOPMENT;
- TEST;
- SIMULATION;
- SANDBOX;
- OPERATIONAL.

A feature enabled in one environment shall not automatically be enabled in another.

## Deployment Binding

Deployment profiles shall define applicable feature availability.

Feature availability shall be verified during startup and deployment readiness.

## Startup Validation

Startup shall validate:

- configuration integrity;
- configuration compatibility;
- feature flag validity;
- capability dependencies;
- required services;
- safety configuration;
- environment;
- deployment profile.

## Configuration Dependencies

Configuration dependencies shall be explicit.

A feature shall not be enabled when required dependencies are unavailable or incompatible.

## Dependency Graph

The configuration system shall support a dependency graph for:

- capabilities;
- services;
- interfaces;
- data schemas;
- external systems;
- safety functions.

## Compatibility

Configuration shall be checked against:

- software version;
- interface version;
- capability version;
- data schema version;
- deployment environment.

## Invalid Configuration

Invalid configuration shall be rejected.

The system shall not silently substitute an incompatible configuration.

## Configuration Validation

Configuration shall pass applicable validation before becoming authoritative.

Safety-relevant configuration shall receive additional validation according to Safety Architecture.

## Configuration Approval

Material configuration changes may require approval according to authority rules.

Approval shall be bound to the specific configuration revision.

## Runtime Changes

Runtime configuration changes shall be explicitly classified.

Changes shall be:

- prohibited;
- restart-required;
- dynamically applicable;
- mission-revalidation-required;
- approval-required.

## No Silent Runtime Change

Material runtime configuration changes shall not occur silently.

The system shall record:

- previous configuration;
- new configuration;
- actor or source;
- time;
- reason;
- affected capabilities;
- validation result;
- resulting state.

## Mission Impact

A configuration change affecting an active mission shall trigger applicable:

- impact analysis;
- validation;
- revalidation;
- readiness reassessment;
- replanning;
- approval.

## Safety Impact

Changes affecting safety-critical behaviour shall be identified explicitly.

Safety-impacting configuration changes shall not bypass Safety Engine authority.

## Mission AI Configuration

Mission AI configuration shall be versioned and traceable.

Changes shall not silently alter an already approved mission decision basis.

## Optimization Configuration

Optimization configuration shall identify applicable:

- profile;
- objectives;
- constraints;
- weights where defined;
- resource assumptions;
- version.

Optimization configuration shall be recorded with the resulting mission variant.

## Simulation Configuration

Simulation shall record:

- simulation engine version;
- environment;
- configuration;
- feature flags;
- capability set;
- data sources.

Simulation configuration shall remain distinguishable from operational configuration.

## Reproducibility

A simulation or mission result shall be reproducible to the extent that its referenced configuration, software, data and environment remain available.

## Planned / Simulated / Actual

Configuration records shall preserve the relationship between:

- PLANNED;
- SIMULATED;
- ACTUAL.

The system shall not silently use a simulated configuration as the actual execution configuration.

## Configuration Comparison

The system shall support comparison between configuration revisions.

Differences shall identify affected domains and capabilities.

## Impact Analysis

Material configuration changes shall support impact analysis across:

- mission;
- safety;
- validation;
- readiness;
- simulation;
- performance;
- security;
- external integrations.

## Change Classification

Changes shall be classified according to impact.

Example classes may include:

- informational;
- operational;
- mission;
- safety;
- security;
- performance;
- interface;
- deployment.

## Configuration Rollback

Where supported, configuration shall be restorable to a known compatible revision.

Rollback shall not bypass validation or safety requirements.

## Failed Configuration

A failed configuration change shall not become authoritative.

The system shall preserve the previous valid configuration where safe to do so.

## Recovery

Configuration recovery shall verify:

- integrity;
- compatibility;
- dependencies;
- safety;
- validation;
- readiness where applicable.

## Security

Configuration storage and modification shall follow:

- authentication;
- authorization;
- least privilege;
- integrity protection;
- audit requirements.

## Configuration Secrets

Secrets shall not be stored in ordinary configuration fields where protected secret storage is required.

## Audit

Material configuration changes shall be auditable.

Audit records shall identify:

- previous state;
- new state;
- actor;
- authorization;
- timestamp;
- reason;
- result.

## Operator Visibility

The operator shall be able to identify the active:

- configuration;
- revision;
- environment;
- deployment profile;
- feature set;
- capability set.

## Configuration Explainability

Where a capability is unavailable or disabled, the system should identify the applicable reason where security and safety policy permit.

Reasons may include:

- not configured;
- dependency unavailable;
- incompatible version;
- environment restriction;
- safety block;
- validation failure;
- authorization restriction.

## No Silent Capability Substitution

The system shall not silently replace an unavailable capability with another capability unless an explicitly defined fallback policy permits it.

Fallback selection shall remain traceable.

## Feature Rollout

Optional features may support controlled rollout.

Rollout shall be environment- and configuration-controlled.

## Feature Rollback

Feature flags may be reverted through controlled change procedures.

Rollback shall not bypass safety or validation requirements.

## Runtime Capability State

Capability state shall be observable at runtime.

The system shall distinguish:

AVAILABLE
→ ENABLED
→ READY
→ ACTIVE

from states such as:

DEGRADED
→ BLOCKED
→ UNAVAILABLE.

## Capability Activation

Activation of a capability shall pass applicable:

- dependency checks;
- configuration checks;
- safety checks;
- validation;
- readiness.

## Mission Capability Binding

A mission shall identify the capabilities it requires.

Mission readiness shall fail or degrade according to defined policy when required capabilities are unavailable.

## Fleet Capability Binding

Fleet allocation shall consider actual capability availability rather than configured capability alone.

## UAV Capability Binding

UAV assignment shall use the authoritative capability model.

## C2 Capability Binding

Communication capabilities shall be evaluated using current C2 and communication state.

## Map and Environmental Capability Binding

Map, terrain, airspace, geofence and weather capabilities shall identify current source, version and availability.

## Simulation Capability Binding

Simulation shall use an explicit capability set.

A simulated capability shall not imply actual operational capability.

## Sandbox Capability Binding

Sandbox capabilities shall remain isolated from operational authority.

## FPV Capability

FPV shall remain an optional capability.

The baseline system shall not require FPV.

When implemented, FPV shall use the Media / Video Adapter architecture defined for future extension.

FPV activation shall not create direct execution authority.

## Configuration and QML

QML shall consume configuration state through Core-provided models.

The UI shall not become an independent configuration authority.

## Configuration and Mission AI

Mission AI may inspect permitted configuration and capability state.

Mission AI shall not silently modify authoritative configuration.

## Configuration and Safety

Safety Engine shall have authority to reject configuration or capability states that violate safety constraints.

## Configuration and Validation

Mission Validation shall evaluate configuration changes that affect mission validity.

## Configuration and Readiness

Mission Readiness shall evaluate whether the active configuration supports required mission execution.

## Configuration and Execution

Execution shall use only an authoritative, validated and applicable configuration.

## Configuration State Machine

Configuration changes shall support explicit states such as:

DRAFT
→ VALIDATING
→ APPROVED
→ ACTIVE

and:

DRAFT
→ VALIDATION FAILED

or:

ACTIVE
→ CHANGE REQUESTED
→ REVALIDATION REQUIRED.

## Traceability

Configuration traceability shall support:

CONFIGURATION
→ VERSION
→ CHANGE
→ IMPACT
→ VALIDATION
→ APPROVAL
→ MISSION
→ EXECUTION
→ RESULT.

## Verification

Configuration management shall be verified through:

- valid configuration;
- invalid configuration;
- incompatible configuration;
- missing dependency;
- feature flag changes;
- runtime changes;
- rollback;
- failed change;
- safety-impacting change;
- mission-impacting change;
- simulation configuration;
- sandbox configuration;
- environment separation;
- capability loss.

## Negative Testing

Verification shall confirm that:

- invalid configuration cannot become authoritative;
- feature flags cannot bypass safety;
- feature flags cannot grant execution authority;
- environment configuration cannot silently cross environments;
- capability availability cannot be falsely reported;
- failed configuration cannot replace a valid configuration;
- runtime changes cannot silently alter an active mission;
- simulated configuration cannot silently become actual configuration.

## Performance

Configuration evaluation shall not block safety-critical execution unnecessarily.

Large configuration sets shall be handled without unacceptable runtime degradation.

## Observability

Configuration and feature state changes shall integrate with the observability and logging architecture.

## Deployment Integration

Configuration shall integrate with deployment profiles, startup validation, release readiness, rollback and recovery.

## Data Lifecycle

Configuration records shall follow defined retention, archival and integrity requirements.

## Rationale

A controlled configuration and capability architecture allows BlueSky to evolve through optional features and environment-specific functionality without weakening safety, validation, readiness or execution authority.

It also provides a safe path for future capabilities such as FPV while preserving reproducibility, traceability and configuration integrity.


