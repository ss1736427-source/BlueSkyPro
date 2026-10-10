# EC-11 — Safety / Failsafe

**Status:** IMPLEMENTATION BASELINE — PARTIAL

**Source:** `SRC-19 FAULT_RECOVERY`

## Baseline scope

The prototype defines a universal safety/failsafe event contract and a baseline adapter with deterministic validation of source identity, timestamp, validity, freshness, trigger, requested action, applied action, state, and quality.

The baseline test covers a representative C2-loss failsafe event and rejection of invalid, stale, and incomplete events.

## Safety boundary

This implementation does not authorize an operational safety action by itself. It records and validates a safety/failsafe fact. Actual action selection, command execution, timing verification, and final safety acceptance remain subject to the approved safety logic and verification procedure.

## Evidence boundary

Prototype, replay, bench, or injected events are not physical verification evidence automatically. EC-11 remains PARTIAL until approved real sources, controlled execution evidence, domain processing/result linkage, integrity-controlled evidence package, certification export verification, and physical/operational verification are available.
