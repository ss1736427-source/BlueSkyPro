---
id: ARCH-DEC-031
type: architecture_decision
title: Deployment Environment and Operational Readiness Architecture
status: accepted
scope: BlueSky System
---

# ARCH-DEC-031 — Deployment / Environment / Operational Readiness Architecture

## Decision

BlueSky shall use controlled deployment environments and explicit operational readiness checks to ensure that software, configuration, services and external dependencies are suitable for their intended mode of operation.

Deployment state shall remain separate from mission readiness and mission approval.

## Deployment Authority

The C++ Core and controlled deployment services shall own authoritative deployment state.

QML shall display deployment and environment status but shall not independently authorize operational deployment.

## Deployment Domains

The architecture shall distinguish:

- DEVELOPMENT;
- TEST;
- SIMULATION;
- SANDBOX;
- OPERATIONAL.

Each environment shall have an explicit identity.

## Environment Identity

Every running instance shall expose:

- environment;
- release;
- configuration baseline;
- software version;
- interface versions;
- deployment profile;
- operational mode.

Environment identity shall be available to monitoring and audit.

## Environment Separation

Development, Test, Simulation, Sandbox and Operational environments shall be isolated according to their risk and purpose.

Operational systems shall not silently consume non-operational endpoints, credentials or configuration.

## Deployment Profiles

Deployment profiles shall define applicable:

- services;
- configuration;
- endpoints;
- capabilities;
- feature availability;
- resource limits;
- security policy;
- monitoring policy.

Deployment profiles shall be versioned.

## Development Environment

Development shall support engineering work without operational execution authority.

Development data shall not silently become authoritative operational data.

## Test Environment

Test shall support automated and human verification.

Test environments shall remain distinguishable from operational environments.

## Simulation Environment

Simulation shall support Digital Twin and virtual execution.

Simulation shall not directly control real UAV execution.

## Sandbox Environment

Sandbox shall support isolated experimentation and training.

Sandbox shall not have direct operational execution authority.

## Operational Environment

Operational deployment shall use an accepted release and configuration baseline.

Operational deployment shall provide the services required by the accepted system architecture.

## Startup Validation

Startup shall verify applicable:

- software integrity;
- configuration;
- service availability;
- interface compatibility;
- security state;
- data stores;
- monitoring;
- external dependencies.

Startup shall fail safely when mandatory conditions are not satisfied.

## Runtime Health

Operational deployment shall continuously monitor required service health.

A service shall not be considered healthy merely because its process is running.

## Service Dependencies

Deployment readiness shall account for dependencies between:

- Core;
- Safety Engine;
- Validation;
- Readiness;
- Mission AI;
- Mission Memory;
- data stores;
- UAV adapters;
- C2;
- communication;
- external services.

## Dependency Readiness

A dependent service shall not be reported operational when mandatory dependencies remain unavailable.

## Configuration Readiness

The active configuration shall be checked against the accepted deployment baseline.

Unexpected configuration drift shall be detected.

## Release Readiness

A release shall be eligible for operational deployment only when applicable:

- verification;
- validation;
- security review;
- configuration checks;
- deployment checks;

are satisfied.

## Operational Readiness

System operational readiness shall remain distinct from mission readiness.

A healthy deployed system does not imply that a specific mission is Ready.

## Mission Readiness

Mission Readiness shall remain governed by the mission-specific validation and safety architecture.

Deployment readiness is a prerequisite for mission operation, not a replacement for mission readiness.

## Approval

Operational deployment shall use the appropriate release approval.

Mission approval remains separately controlled.

## Deployment Gate

The deployment gate shall prevent operational activation when mandatory release or configuration conditions are unresolved.

## Mode Binding

Deployment environment and operational mode shall be mutually consistent.

Invalid combinations shall be blocked.

Examples include:

- OPERATIONAL mode on a TEST-only deployment;
- operational endpoint in SANDBOX;
- SIMULATION mode with direct UAV execution;
- TEST deployment using an uncontrolled operational configuration.

## Feature Availability

Capabilities shall be explicitly available or unavailable according to deployment profile.

Unavailable functionality shall not be presented as operationally usable.

## Feature Flags

Material feature flags shall be versioned and controlled.

Safety-critical functionality shall not depend on uncontrolled feature flag changes.

## Resource Requirements

Deployment readiness shall account for required:

- CPU;
- GPU;
- memory;
- storage;
- network;
- fleet communication resources.

Insufficient resources shall produce an explicit degraded or blocked state.

## Performance Readiness

Performance-critical deployments shall be evaluated against ARCH-DEC-022.

Required runtime responsiveness shall be verified before operational acceptance.

## Security Readiness

Deployment shall verify applicable:

- authentication;
- authorization;
- certificates;
- protected interfaces;
- credential availability;
- security configuration;
- audit capability.

Security failures affecting protected functions shall block operational readiness.

## Safety Readiness

Safety-critical services shall be available before operational readiness is granted.

The deployment shall not bypass:

- Safety Engine;
- Mission Validation;
- Mission Readiness;
- execution authority.

## Monitoring Readiness

Monitoring shall be operational before the system is considered fully ready.

Required health, performance, security and safety metrics shall be available.

## Logging Readiness

Operational logging and audit recording shall be available before operational acceptance.

Critical events shall have an appropriate persistence path.

## Data Readiness

Required data stores shall be available and consistent.

Data readiness shall include:

- schema;
- configuration;
- mission store;
- telemetry store;
- audit store;
- simulation store where applicable.

## Data Integrity

Deployment shall detect material data integrity problems.

A system with uncertain authoritative data shall enter an appropriate blocked or degraded state.

## External Connection Readiness

Required external systems shall be checked before operational activation.

Applicable dependencies include:

- UAV;
- C2;
- communication;
- map;
- terrain;
- airspace;
- geofence;
- weather;
- simulation services where applicable.

## External Dependency Failure

Loss of a non-critical dependency may result in degraded operation where safe.

Loss of a safety-critical dependency shall trigger the applicable safety response.

## Deployment Verification

Deployment shall be verified before operational use.

Verification shall cover applicable:

- startup;
- services;
- configuration;
- interfaces;
- security;
- monitoring;
- logging;
- data;
- recovery.

## Smoke Testing

A controlled smoke test shall verify that the deployed system behaves according to the accepted baseline.

Smoke testing shall not accidentally initiate real mission execution.

## Health Check

Health checks shall distinguish:

- healthy;
- degraded;
- unavailable;
- failed;
- unknown.

Unknown health shall not be treated as healthy.

## Readiness State Machine

Deployment readiness shall support a controlled state model such as:

DEPLOYING
→ INITIALIZING
→ CHECKING
→ DEGRADED / BLOCKED / READY
→ OPERATIONAL

with failure and recovery paths.

## Recovery

After deployment failure, the system shall support controlled:

- rollback;
- retry;
- recovery;
- diagnostic investigation.

A failed deployment shall not silently become operational.

## Rollback

Rollback shall restore a known accepted release/configuration state where applicable.

Rollback shall preserve deployment and audit history.

## Upgrade

Upgrades shall be controlled through ARCH-DEC-026.

Material upgrades shall receive applicable verification and validation.

## Compatibility

Upgrades shall verify compatibility between:

- application;
- Core;
- adapters;
- interfaces;
- data schemas;
- external systems.

## Migration

Data or configuration migration shall be explicit and reversible where technically applicable.

Migration shall preserve historical data interpretation.

## Zero-Downtime Considerations

Where zero-downtime deployment is used, the architecture shall preserve:

- authoritative state;
- safety boundaries;
- interface compatibility;
- audit continuity.

Zero-downtime shall not be used to bypass validation or approval.

## Multi-Instance Operation

If multiple instances are deployed, authoritative state ownership shall remain unambiguous.

Split-brain operational authority shall be prevented.

## Operator Visibility

The operator shall be able to determine:

- environment;
- release;
- deployment status;
- configuration baseline;
- service health;
- dependency health;
- monitoring state;
- readiness state.

## Operator Notification

Material deployment or readiness failures shall generate appropriate notifications.

The operator shall be informed when operation is:

- blocked;
- degraded;
- recovering;
- ready.

## Security and Access

Deployment operations shall follow ARCH-DEC-018.

Operational deployment authority shall be restricted according to role and least privilege.

## Audit

Deployment and environment changes shall be auditable.

Records shall include where applicable:

- actor;
- timestamp;
- environment;
- previous release;
- new release;
- baseline;
- result;
- rollback;
- reason.

## Configuration Control

Deployment configuration shall follow ARCH-DEC-019 and ARCH-DEC-026.

Material configuration drift shall be detected.

## Backup and Recovery

Deployment recovery shall follow ARCH-DEC-027.

Required recovery assets shall be available before operational acceptance.

## Logging and Observability

Deployment state shall integrate with ARCH-DEC-024 and ARCH-DEC-029.

Startup failures, dependency failures and readiness transitions shall be recorded.

## External Integration

Deployment shall follow ARCH-DEC-028 for external interface configuration and compatibility.

## Simulation and Sandbox

Deployment verification shall be executable in Simulation and Sandbox where appropriate.

These environments shall remain isolated from operational execution.

## Change Impact

Deployment changes shall undergo impact analysis when they affect:

- safety;
- mission execution;
- validation;
- readiness;
- security;
- interfaces;
- data;
- performance.

## No Silent Deployment

The system shall not silently transition an unaccepted build or configuration into operational use.

## No Direct Execution

Deployment readiness shall not itself grant mission execution authority.

The execution path remains:

DEPLOYMENT READY
→ MISSION VALIDATION
→ MISSION READINESS
→ APPROVAL
→ EXECUTION AUTHORITY
→ EXECUTION.

## Operational Acceptance

Operational acceptance shall require the applicable deployment, safety, security, verification and configuration gates to pass.

## Revalidation

Material deployment or configuration changes shall trigger applicable revalidation.

## Traceability

Deployment traceability shall support:

BUILD
→ RELEASE
→ CONFIGURATION BASELINE
→ DEPLOYMENT
→ VERIFICATION
→ READINESS
→ OPERATIONAL ACCEPTANCE
→ MONITORING
→ CHANGE / ROLLBACK.

## Rationale

Explicit deployment and environment architecture prevents configuration drift, accidental operational activation and environment crossover while ensuring that operational readiness is established through controlled verification, health, security, safety and configuration checks.


