# HMI Live Altitude Telemetry Bridge

**ID:** OPS-HMI-ALT-BRIDGE-001  
**Status:** CONTRACT DRAFT — implementation and runtime verification pending  
**Related:** Issue #48, OPS-ALT-ADAPTER-001

## Purpose

Define the narrow, display-only contract between accepted source-specific MAVLink telemetry and the Qt/QML HMI. This contract must not reuse the planning-result channel or imply flight readiness.

## Existing integration boundary

- `qt/BlueSkyPRO-HMI/src/main.cpp` creates `PlanningBridge` and exposes it to QML as `planningBridge`.
- `PlanningBridge` accepts only `planning.result` schema 1.0 and validates mission verification fields. Do not overload that schema with live telemetry.
- `MavlinkTransportChannelRuntime::receive()` currently consumes a frame and returns the original bytes; the accepted `DecodedMavlinkMessage` is not exposed to a live HMI consumer.
- `MavlinkAltitudeDisplayAdapter` consumes a `MavlinkTelemetrySample`, but the current sample structure has a single optional distance-sensor sample. A caller must not overwrite a configured source with a different sensor ID or an older sample.

## Required component

Add a separate `QObject` boundary named `AltitudeTelemetryBridge` (or an equivalent narrowly scoped component approved by the codebase) and expose it independently to QML. Keep `PlanningBridge` responsibilities unchanged.

The display state must be explicitly unavailable before valid live telemetry is received. Preview fixture values must not populate this object.

## Proposed QML-facing state

- `available: bool`
- `valueM: double` (meaningful only when available)
- `sourceLabel: string`: exactly `RA · AGL`, `BARO · STD 1013.25 hPa`, or an explicit unavailable label
- `degraded: bool`
- `timestampQualityDegraded: bool`
- `freshness: string` (current/stale/missing/out-of-order or equivalent controlled enum)
- `receivedTimestampMs: qint64`
- `sourceTimestampBootMs: quint32` (preserve original source boot time; never compare directly with host monotonic time)

The display bridge is not a mission-readiness, authorization, terrain-clearance, or flight-safety authority.

## Ingestion contract

1. Accept only a message already accepted by the MAVLink session/transport boundary; do not decode the same frame through an unrelated second path.
2. Keep per-vehicle/source/component identity and per-sensor samples. A configured radio-altimeter sensor is selected only by explicit capability configuration plus exact sensor ID, sensor type and downward orientation.
3. Retain radio-altimeter readings by sensor ID so non-configured DISTANCE_SENSOR instances cannot overwrite the configured sensor's reading.
4. Reject duplicate/out-of-order updates for each source identity using its boot-time and receive-time progression; preserve both timestamps. A boot-time counter must not be numerically compared to host time. Until clock normalization is proven, mark timestamp quality degraded and qualify any use of receive-time freshness.
5. Freshness and radio/barometric pair-coherence limits must be explicit configuration values. Missing or invalid values fail closed.
6. Apply the existing altitude source selector without changing its policy: radio-altimeter AGL at and below 100.0 m; standard-pressure barometric altitude above 100.0 m; degraded barometric fallback only when allowed by the existing selector; otherwise unavailable.
7. Publish a single coherent snapshot and emit `stateChanged` only when the visible state changes. QML must not calculate altitude or choose sources itself.

## QML presentation

Bind the live altitude in a dedicated header sector or approved existing metric slot. Keep the current header geometry and neighboring sectors unchanged unless a separate approved design change is made. Display the source label and a visible degraded/stale annunciation. Never display a barometric value as AGL.

Do not bind this value to `ready`, mission validation, authorization or start-flight controls.

## Verification required

- Unit tests for per-sensor cache identity and multiple DISTANCE_SENSOR IDs.
- Duplicate and out-of-order samples must not replace the most recent accepted source reading.
- Missing/stale data becomes unavailable; barometric fallback is labelled degraded.
- Thresholds at 99.9 m, 100.0 m, 100.1 m.
- Timestamp fallback is visibly degraded.
- Qt-facing tests for initial unavailable state, state-change notifications and QML property shape.
- HMI build and CI for the exact integration commit.
- Real-autopilot configuration and hardware-in-loop evidence remain **UNVERIFIED** until tested.

## Non-goals

This contract does not add flight control, calculate terrain clearance, validate routes, or prove mission readiness. It does not set operational freshness/coherence thresholds or default sensor IDs.
