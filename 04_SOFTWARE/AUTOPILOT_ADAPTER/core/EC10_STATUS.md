# EC-10 — Weather

**Status:** IMPLEMENTATION BASELINE — PARTIAL

**Source:** `SRC-07 WEATHER`

## Baseline scope

The prototype defines a universal weather sample contract and deterministic validation for wind speed/direction, temperature, pressure, humidity, timestamp, source identity, state, quality, validity and freshness.

The baseline test covers a representative weather sample and rejection of invalid, stale and source-mismatched data.

## Engineering boundary

The adapter records environmental facts. It does not independently declare flight suitability, authorize a mission, or replace the approved meteorological assessment and operating limitations.

## Evidence boundary

Prototype, replay, bench or injected data is not physical verification evidence automatically. EC-10 remains PARTIAL until approved weather sources, controlled execution evidence, domain analysis/result linkage, integrity-controlled evidence package, certification export verification and physical/operational verification are available.
