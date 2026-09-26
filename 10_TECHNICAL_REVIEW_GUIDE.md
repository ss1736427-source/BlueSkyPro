# Technical Review Guide

## Purpose

Enable a software team to assess the BlueSky PRO engineering direction without receiving the complete proprietary implementation.

## Review by discipline

### Frontend / HMI

Assess:

- Flight Chart / map-first shell;
- workspace and panel architecture;
- route interaction;
- telemetry presentation;
- state management;
- desktop and multi-window strategy;
- role-oriented workspaces.

Do not request internal implementation details unless required for a defined engineering task.

### Backend / Domain

Assess:

- mission and aircraft domain boundaries;
- readiness and authorization state;
- persistence;
- event/telemetry handling;
- audit and traceability;
- integration boundaries;
- AI experience boundary.

### Planning

Assess:

- how mission objectives enter the planning process;
- how aircraft capabilities are represented;
- how constraints and environment are supplied;
- how planning outputs are verified;
- how failures are surfaced.

The protected planning recipe is not part of the initial review.

### Multi-UAV

Assess:

- task decomposition concept;
- aircraft/task association;
- route and fleet consistency responsibilities;
- verification boundary;
- release blocking behavior.

Do not request the protected algorithms, numerical parameters, internal validators or reproduction-oriented contracts for the initial review.

### Integrations

Assess boundaries for:

- autopilot;
- C2;
- telemetry;
- weather/environment;
- maps;
- regulatory/authorization;
- payload/data.

### AI

Assess interfaces and authority boundaries rather than model internals.

### Assurance

Assess whether the architecture can support requirement allocation, verification, evidence, configuration control and change impact.

## Expected review output

The technical team's useful deliverable is:

1. architectural inconsistencies;
2. missing boundaries or interfaces;
3. implementation risks;
4. data-model risks;
5. real-time processing risks;
6. offline-operation risks;
7. AI integration risks;
8. security/access gaps;
9. recommended engineering sequence;
10. questions requiring the project owner's decision.

## Protected material rule

Additional protected material should be disclosed only when necessary for a specific engineering task and only to the people who need it.

The controlled overview is not a source-code handoff.
