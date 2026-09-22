# Technical Review Guide

## Purpose

This document defines how a software team should inspect the BlueSky PRO project without turning the review into an uncontrolled redistribution of the complete development history.

## Review sequence

### Frontend

Review:

- application shell;
- Flight Chart;
- navigation and workspace model;
- panel architecture;
- telemetry presentation;
- route editing;
- state management;
- component boundaries;
- desktop/multi-window strategy;
- design-system consistency.

Questions:

- Which UI state belongs locally and which belongs to the domain/backend?
- How are high-frequency telemetry updates isolated from ordinary UI state?
- How is map-provider replacement isolated?
- How are route/profile edits represented?
- How is offline operation handled?

### Backend

Review:

- domain boundaries;
- mission model;
- aircraft model;
- flight record;
- authorization/compliance state;
- integration adapters;
- persistence;
- event/telemetry processing;
- audit/traceability;
- AI experience storage.

Questions:

- What is the authoritative source of each operational state?
- Which entities require immutable records?
- Which data is event-like versus current-state?
- Which boundaries should be synchronous APIs versus asynchronous events?
- How are aircraft-specific records isolated from mission-level records?

### Integrations

Review:

- autopilot interfaces;
- C2;
- telemetry;
- weather/environment;
- maps;
- regulatory/ATM interfaces;
- insurance/provider interfaces;
- payload/data interfaces.

The core model should remain independent of any one external provider where technically and legally possible.

### AI

Review the interfaces around AI rather than assuming that AI should own the core domain.

Expected separation:

```
Operational data
      ↓
Feature / evidence preparation
      ↓
AI / analytical function
      ↓
Prediction / recommendation / correction
      ↓
Human or deterministic operational decision
      ↓
Operational result
      ↓
Learning evidence
```

The review should determine where deterministic rules must remain authoritative and where statistical/AI methods add value.

### Assurance

Review whether each safety- or compliance-relevant capability can eventually be traced from requirement to verification evidence.

## What should not be requested for the first technical review

Unless a specific engineering task requires it, the review does not need:

- credentials;
- private keys;
- production secrets;
- production infrastructure access;
- model weights;
- private training datasets;
- proprietary calibration coefficients;
- internal security bypasses.

## Expected output from the technical team

The useful result is not a general opinion. It should identify:

1. confirmed architectural strengths;
2. architectural inconsistencies;
3. missing interfaces or contracts;
4. backend/frontend coupling risks;
5. data-model risks;
6. real-time processing risks;
7. offline-operation risks;
8. AI integration risks;
9. security gaps;
10. recommended implementation sequence;
11. items requiring clarification from the product owner.

## Working rule

The team may propose changes freely. Changes to the protected product baseline should be introduced through reviewed Pull Requests rather than direct modification of the protected baseline.
