---
id: ARCH-DEC-006
type: architecture_decision
title: Mission Optimization Profiles
status: accepted
scope: BlueSky System
---

# ARCH-DEC-006 — Mission Optimization Profiles

## Decision

BlueSky shall support optimization profiles that define the primary optimization objective for a mission.

The applicable profile shall be selected according to the user's task, required result, operational constraints and mission context.

The system shall not select an optimization profile independently of the user task.

## Supported Profiles

The system shall support the following base profiles:

- FAST
- SAFE
- ECONOMIC
- HIGH QUALITY

## FAST

FAST prioritizes reduction of mission execution time while remaining within mandatory safety and operational constraints.

The profile may prefer:

- shorter execution time;
- reduced mission duration;
- faster route alternatives;
- efficient resource allocation.

FAST shall not override mandatory safety constraints.

## SAFE

SAFE prioritizes risk reduction and operational safety.

The profile may prefer:

- lower-risk routes;
- larger safety margins;
- increased redundancy;
- more conservative operational parameters;
- additional validation or simulation.

Safety requirements and Safety Engine decisions have priority over optimization preferences.

## ECONOMIC

ECONOMIC prioritizes efficient use of mission resources.

The profile may consider:

- energy consumption;
- UAV count;
- payload utilization;
- communication resources;
- mission duration;
- operational cost.

ECONOMIC shall not violate mandatory safety or mission requirements.

## HIGH QUALITY

HIGH QUALITY prioritizes the quality and completeness of the required mission result.

The profile may prefer:

- improved coverage;
- higher data quality;
- increased observation time;
- additional sensing capabilities;
- higher-quality payload configuration;
- additional resources when required.

HIGH QUALITY shall remain within safety, operational and user-defined constraints.

## Profile Selection

The system shall evaluate the user's task and required result to determine the most appropriate optimization profile.

Selection may consider:

- explicit user preference;
- required result;
- time constraints;
- quality requirements;
- resource constraints;
- operational risk;
- environmental conditions;
- mission criticality;
- available capabilities.

When the user explicitly specifies an optimization preference, the system shall preserve that preference unless it conflicts with mandatory safety or system constraints.

## Automatic Selection

The system may automatically select the most appropriate profile when the user has not explicitly specified one.

The selected profile shall be explainable.

The system shall be able to provide the main factors that influenced the selection.

## Operator Control

The operator shall be able to review the selected profile.

Where permitted by the applicable safety and operational constraints, the operator shall be able to select another supported profile.

Changing the profile shall trigger the required recalculation of mission variants and affected planning results.

## Mission Variants

The system shall support generation of multiple mission variants using different optimization profiles.

Variants may be compared according to:

- execution time;
- resources;
- risk;
- mission quality;
- UAV count;
- energy;
- other applicable mission criteria.

## Adaptation

A change in the user's task, required result, operational environment or mission constraints may require recalculation of the selected optimization profile.

The system shall preserve traceability between:

USER TASK
→ OPTIMIZATION PROFILE
→ MISSION VARIANT
→ VALIDATION
→ SIMULATION
→ FINAL PLAN

## Safety Priority

Optimization profiles shall never override mandatory safety constraints.

The priority relationship is:

SAFETY CONSTRAINTS
>
MISSION REQUIREMENTS
>
USER PREFERENCES
>
OPTIMIZATION PROFILE PREFERENCES

The Safety Engine remains authoritative for safety decisions.

## Revalidation

Changing the optimization profile or materially changing its parameters shall invalidate affected planning, validation, readiness or simulation results when applicable.

The mission shall be revalidated before execution when the change affects mission safety, feasibility or execution behaviour.

## Rationale

Optimization profiles provide a controlled way to adapt mission planning to different user objectives without changing the underlying Mission Architecture.

The four base profiles represent different optimization priorities while preserving safety, traceability, operator control and mission requirements.


