# EC-12 — Airspace / Geofence

**Status:** IMPLEMENTATION BASELINE — PARTIAL

**Source:** `SRC-11 AIRSPACE_GEOFENCE`

## Baseline scope

The prototype defines a universal airspace/geofence sample contract and deterministic validation for source identity, timestamp, position, altitude, restriction state, clearance state, decision, quality, validity and freshness.

The baseline test covers an allowed position and rejection of invalid coordinates, stale data, source mismatch and an inconsistent restricted-position decision.

## Safety boundary

The adapter records geographic/restriction facts and consistency checks. It does not independently grant clearance, authorize a flight or replace approved airspace data and operational decision logic.

## Evidence boundary

Prototype, replay, bench or injected data is not physical verification evidence automatically. EC-12 remains PARTIAL until approved airspace/restriction sources, controlled execution evidence, domain analysis/result linkage, integrity-controlled evidence package, certification export verification and physical/operational verification are available.
