# EC-08 — Propulsion

**Status:** IMPLEMENTATION BASELINE — PARTIAL

**Source:** `SRC-09 PROPULSION`

## Baseline scope

The prototype defines a universal propulsion sample contract and a deterministic baseline adapter covering propulsion identity, timestamp, RPM, thrust, torque, fuel flow, throttle, temperature, vibration, state, quality, validity and freshness.

The baseline test covers a representative nominal propulsion sample and rejection of out-of-range, stale and source-mismatched data.

## Safety boundary

This implementation records and validates propulsion facts. It does not independently authorize engine/propeller control, shutdown, emergency action or operational acceptance. Those functions remain subject to approved propulsion and safety logic and verification procedures.

## Evidence boundary

Prototype, replay, bench or injected data is not physical verification evidence automatically. EC-08 remains PARTIAL until approved real propulsion sources, controlled execution evidence, domain processing/result linkage, integrity-controlled evidence package, certification export verification and physical/operational verification are available.
