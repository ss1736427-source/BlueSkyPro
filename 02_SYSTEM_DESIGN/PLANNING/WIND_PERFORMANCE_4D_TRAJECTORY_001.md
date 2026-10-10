# Wind + Vehicle Performance → 4D Trajectory

**ID:** PLAN-CALC-001
**Status:** BASELINED
**Scope:** deterministic flight-planning calculation boundary

## Purpose

This boundary produces the single reusable wind/performance result consumed by downstream 4D trajectory and multi-UAV conflict calculations.

Pipeline:

`ROUTE + UAV CONFIGURATION + WIND SNAPSHOT + PERFORMANCE MODEL -> GROUND VECTOR -> GROUND SPEED/TRACK -> SEGMENT TIME -> ENERGY -> 4D TRAJECTORY`

## Contract

Inputs are versioned:
- route identity/version and calculation input version;
- UAV identity and configuration version;
- performance model version;
- wind snapshot identity/version;
- airspeed, climb/descent rates, wind tolerance and energy parameters.

Every route segment requires one normalized wind sample. The current implementation supports a segment-specific horizontal wind vector plus vertical wind component. Source-specific forecast/observation validity is resolved upstream into the versioned wind snapshot.

Outputs contain:
- ground speed and track per segment;
- traversal time;
- segment energy;
- ordered 4D points with altitude, elapsed time and cumulative energy;
- total time/energy and remaining energy after reserve;
- calculation version and dependency identity.

## Determinism and reuse

The dependency identity includes route identity/version, calculation input version, UAV/configuration, performance version, wind snapshot identity/version, calculation version and wind sample values. The result is reusable until one of those dependencies changes.

No second wind/performance calculation is permitted downstream for the same dependency identity.

## Hard feasibility

The calculation returns Infeasible when:
- performance inputs are invalid;
- route structure is invalid;
- a segment has no wind sample;
- wind exceeds the configured UAV tolerance;
- ground speed cannot be established;
- usable energy after reserve is insufficient.

These are feasibility conditions, not optimization weights.

## Vertical segments

A zero-horizontal-distance segment is treated as a vertical profile segment. Its traversal time is determined by climb/descent rate. It remains part of the same trajectory calculation and is therefore visible to downstream temporal conflict analysis.

The current RouteSegment remains a compact spatial contract; explicit climb/descent rate/profile fields can be added later if the route model requires richer vertical semantics. This implementation does not treat zero horizontal distance as zero elapsed time.

## Authority boundary

This component:
- does not authorize a flight;
- does not change readiness;
- does not modify safety state;
- does not execute commands;
- does not override restrictions;
- does not make AI authoritative.

It is a deterministic planning calculation only.
