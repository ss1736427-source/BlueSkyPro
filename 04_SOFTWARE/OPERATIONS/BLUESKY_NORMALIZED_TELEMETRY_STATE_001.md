# ARCH-OPS-017 — Normalized Telemetry / Runtime State Boundary

**Status:** IMPLEMENTED — contract/test baseline; integration pending

## Purpose

Provide a transport- and autopilot-independent runtime-facing snapshot of actual vehicle telemetry/state. The contract preserves source provenance and freshness so the Operational Orchestrator can distinguish usable current state from stale, delayed, missing, or derived information.

## Boundary

```text
AUTOPILOT / C2 / OTHER ADAPTER SOURCES
                ↓
        TELEMETRY NORMALIZATION
                ↓
     NormalizedTelemetryState
                ↓
      RUNTIME / ORCHESTRATOR
```

The normalization layer does not own runtime state transitions, C2 transport, autopilot control semantics, numerical Energy Model parameters, UI behavior, or safety thresholds.

## Contract

Each normalized snapshot carries:

- vehicle identity;
- snapshot version;
- source provenance and adapter version;
- freshness classification;
- source and receipt timestamps;
- validity of position, navigation, vehicle state, energy state, and C2 state.

Freshness classes are explicit:

- `Current` — suitable for current runtime coordination when required validity facts are present;
- `Stale` — previously valid data whose freshness is no longer current;
- `Delayed` — data received later than expected and requiring conservative treatment;
- `Missing` — required source data is unavailable;
- `Derived` — value produced from other data rather than directly observed.

No numerical freshness or safety thresholds are defined here; those belong to the applicable system/runtime contracts and verified configuration.

## Runtime use

`normalized_state_usable()` is deliberately conservative: it requires identity, provenance, current freshness, valid position, valid navigation, and valid vehicle state. Energy and C2 validity are represented for downstream gates but are not silently converted into numerical feasibility decisions here.

`normalized_state_requires_refresh()` identifies stale, delayed, and missing snapshots for runtime coordination.

## Replay aggregation constraint — GAP

The current normalized snapshot exposes one source timestamp and one receipt timestamp for the snapshot, while MAVLink replay assembly combines fields received in separate messages. The current sample contract does not carry field-specific timestamps or an explicit coherence window.

Consequently, a replay assembler must not treat the maximum snapshot timestamps as proof that every retained field was observed at that time. A field may have been retained from an earlier message while the aggregate timestamp advances.

**Status: OPEN — contract and verification required before operational integration.** Define the ordering basis for replay messages, per-field timestamp/validity semantics (or an equivalent bounded-coherence rule), and behavior for out-of-order or delayed messages. Do not introduce numerical age/coherence thresholds in this document; they require an applicable runtime contract and verified configuration.

Required verification cases should cover at least:

- a newer message of one type following older position or battery data;
- out-of-order messages for the same vehicle/source identity;
- delayed data that must not make an older field appear current;
- preservation of the existing heartbeat-degradation behavior.

Until this gap is closed, replay aggregation is deterministic fixture logic only and is not evidence of current, coherent live telemetry.

## Traceability

This contract closes the structural implementation gap for normalized vehicle state while preserving the existing distinction between planned state and actual vehicle/autopilot state. Adapter-specific telemetry mappings and replay/integration verification remain pending.

## Verification boundary

Current evidence is limited to deterministic C++ contract tests and build integration. No claim is made for SIL, HIL, real autopilot hardware, or real-UAV flight verification.
