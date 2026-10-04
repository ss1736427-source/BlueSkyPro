---
id: ARCH-DEC-026
type: architecture_decision
title: Release Configuration Baseline and Change Control Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-026 — Release / Configuration Baseline / Change Control Architecture

## Decision

BlueSky shall use controlled release, configuration baseline and change-control processes to preserve system integrity, reproducibility, safety and traceability.

No material change shall silently invalidate previously established validation, readiness, approval or verification results.

## Configuration Authority

The authoritative configuration state shall be maintained through controlled Core services and versioned configuration records.

QML shall not directly modify protected configuration without the applicable Core authorization path.

## Configuration Domains

The architecture shall manage versions for applicable:

- software;
- system configuration;
- mission configuration;
- mission revision;
- UAV configuration;
- capability definitions;
- spatial data;
- environmental data;
- Safety configuration;
- Validation configuration;
- Optimization configuration;
- Simulation configuration;
- external interface configuration.

## Configuration Identity

Each material configuration shall have an identifiable version or revision.

The system shall be able to determine which configuration was active for:

- planning;
- validation;
- optimization;
- simulation;
- readiness;
- approval;
- execution.

## Baseline

A baseline shall represent a known configuration state against which verification, validation or release decisions are made.

A baseline shall identify applicable versions and dependencies.

## Baseline Integrity

Once established for a controlled activity, a baseline shall not be silently modified.

A material change shall create a new revision or baseline.

## Mission Baseline

A mission baseline shall preserve the exact mission context used for:

- planning;
- validation;
- optimization;
- simulation;
- readiness;
- approval;
- execution.

## Release Baseline

A release baseline shall identify the software and configuration required for an accepted system release.

The release baseline shall include applicable dependencies and interface versions.

## Dependency Graph

Configuration relationships shall be represented sufficiently to determine affected dependencies.

Changes shall be evaluated against the dependency graph before acceptance.

## Change Classification

Changes shall be classified according to impact.

Applicable categories may include:

- informational;
- minor;
- functional;
- safety-relevant;
- mission-critical;
- security-relevant;
- interface-breaking.

## Change Impact Analysis

Material changes shall undergo impact analysis.

Impact analysis shall determine whether the change affects:

- requirements;
- architecture;
- interfaces;
- validation;
- safety;
- readiness;
- optimization;
- simulation;
- security;
- existing missions;
- existing verification evidence.

## Safety-Relevant Changes

Safety-relevant changes shall require dedicated review and applicable re-verification or re-validation.

A safety-relevant change shall not be treated as an ordinary cosmetic update.

## Mission Changes

Material mission changes shall invalidate or re-evaluate applicable:

- validation;
- readiness;
- optimization;
- simulation;
- approval.

Previously generated results shall remain associated with the previous mission revision.

## Configuration Changes During Planning

Planning-time configuration changes shall create or update the applicable mission revision.

The system shall preserve the relationship between the mission and the configuration used to generate its results.

## Configuration Changes During Execution

Runtime configuration changes shall be controlled.

Changes affecting execution safety shall trigger the applicable:

- detection;
- assessment;
- revalidation;
- adaptation;
- replanning;
- block;
- abort.

## Approval Binding

Approval shall be bound to the exact mission revision and applicable configuration baseline.

Changing a material approved element shall require applicable reapproval.

## Verification Binding

Verification results shall identify the baseline against which verification was performed.

A material baseline change shall trigger applicable reverification.

## Validation Binding

Validation results shall identify the mission revision and configuration used.

A material change shall trigger revalidation.

## Readiness Binding

Readiness shall be associated with the exact mission revision and applicable configuration.

A material change shall require readiness to be evaluated again.

## Simulation Binding

Simulation results shall identify:

- mission revision;
- configuration baseline;
- simulation environment;
- simulation configuration;
- relevant data versions.

A material change shall prevent old simulation results from being silently treated as current.

## Release States

A release may progress through controlled states such as:

DRAFT
→ REVIEW
→ VERIFIED
→ VALIDATED
→ ACCEPTED
→ RELEASED
→ SUPERSEDED
→ RETIRED.

Rejected or failed releases shall remain traceable.

## Release Gate

A release shall not become operationally accepted while mandatory:

- verification;
- validation;
- safety review;
- security review;
- configuration checks;

remain unresolved.

## Release Candidate

A release candidate shall have an immutable identity for the applicable verification activity.

Changes to a release candidate shall create a new candidate identity or revision.

## Software Versioning

Software versions shall be explicit and traceable.

The system shall identify:

- application version;
- Core version;
- UI version;
- adapter versions;
- interface versions;
- relevant service versions.

## Interface Compatibility

Interface changes shall be evaluated for compatibility.

Breaking interface changes shall require explicit migration or coordinated release handling.

## Database and Data Schema Changes

Material data schema changes shall be versioned.

Migration procedures shall be controlled and tested where applicable.

Historical mission data shall remain interpretable according to its recorded schema version.

## Configuration Migration

When configuration formats change, migration shall be explicit and traceable.

Migration shall not silently change safety-critical meaning.

## Rollback

The deployment architecture shall support rollback where technically applicable.

Rollback shall restore a known valid software/configuration state.

Rollback shall not delete or rewrite historical mission records.

## Recovery From Failed Release

A failed release shall be identifiable.

The system shall support return to a previously accepted baseline where applicable.

Recovery shall preserve evidence of the failed release.

## Change Approval

Material changes shall require approval according to their classification and authority.

Safety and execution authority shall remain separate from ordinary software change authority.

## Operator Impact

Changes that materially affect operator behaviour shall be identified.

Applicable documentation, UI behaviour and training requirements shall be reviewed.

## Security Impact

Security-relevant changes shall undergo applicable security assessment.

Authentication, authorization, audit and protected interfaces shall remain consistent with ARCH-DEC-018.

## Performance Impact

Performance-sensitive changes shall be evaluated against ARCH-DEC-022.

A change shall not silently introduce resource exhaustion or unacceptable runtime degradation.

## Monitoring Impact

Changes affecting observability shall be evaluated against ARCH-DEC-024.

Critical health and safety monitoring shall remain available after release.

## Verification Impact

Changes shall identify affected verification activities.

The system shall support targeted regression verification.

## Regression

Regression testing shall be required for material changes affecting:

- Safety;
- Mission Engine;
- Validation;
- Readiness;
- Conflict Resolution;
- Optimization;
- Simulation;
- UAV interfaces;
- C2;
- data architecture;
- security;
- execution authority.

## Reverification

Previously passing verification results shall be reconsidered when their baseline is materially changed.

Reverification shall preserve the original result and create a new result for the new baseline.

## Revalidation

Previously validated mission behaviour shall be revalidated when a material dependency changes.

## Audit

All material release and configuration changes shall be auditable.

Audit information shall include where applicable:

- identity;
- timestamp;
- previous version;
- new version;
- change reason;
- authority;
- approval;
- verification result;
- validation result.

## Change History

The system shall maintain change history sufficient to determine:

- what changed;
- when;
- by whom;
- why;
- which baseline was affected;
- which verification was performed;
- which missions were affected.

## Reproducibility

The architecture shall preserve the versions required to reproduce historical:

- planning;
- validation;
- optimization;
- simulation;
- readiness;
- execution analysis.

## Mission Memory

Mission Memory shall not silently rewrite historical confirmed experience when a new configuration or software release changes future results.

Historical results shall remain associated with their original baseline.

## Data Integrity

Configuration records shall preserve integrity and detect where applicable:

- missing dependencies;
- invalid references;
- inconsistent versions;
- corrupted records.

## Deployment Separation

Development, Test, Simulation, Sandbox and Operational deployments shall remain distinguishable.

Operational deployment shall use an explicitly accepted release baseline.

## Environment Compatibility

A release shall identify supported runtime environments.

An unsupported environment shall not be treated as operationally equivalent.

## Feature Flags

Where feature flags are used, material safety or execution functionality shall not depend on uncontrolled operator-side feature changes.

Feature flag state shall be versioned where material.

## Emergency Changes

Emergency changes may use an expedited process when required to protect safety or restore operation.

Emergency changes shall remain:

- authorized;
- recorded;
- traceable;
- reviewed after deployment;
- verified or revalidated as soon as practicable.

## No Silent Change

The architecture shall prohibit silent material changes to:

- mission configuration;
- safety configuration;
- validation rules;
- execution authority;
- capability definitions;
- spatial constraints;
- environmental assumptions;
- approved mission state.

## Operator Visibility

The operator shall be able to determine relevant:

- software version;
- configuration version;
- mission revision;
- release identity;
- baseline;
- validation state;
- readiness state;
- approval state.

## Traceability

The complete change chain shall support:

CHANGE
→ IMPACT ANALYSIS
→ NEW REVISION
→ VERIFICATION
→ VALIDATION
→ APPROVAL
→ RELEASE
→ DEPLOYMENT
→ MONITORING
→ REASSESSMENT.

## Rationale

Controlled configuration and release management prevents version drift, preserves reproducibility and ensures that safety, validation, readiness, approval and verification decisions remain bound to the exact system and mission state for which they were established.


