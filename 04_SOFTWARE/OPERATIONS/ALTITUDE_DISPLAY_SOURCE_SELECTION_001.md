# Altitude Display Source Selection — 100 m Crossover

**ID:** OPS-ALT-DISPLAY-001  
**Status:** IMPLEMENTED — source-selection policy and regression tests; telemetry/UI integration pending

## Display rule

The primary displayed altitude source is selected as follows:

1. When a valid, fresh radio-altimeter measurement reports **AGL <= 100 m**, display that measured AGL value.
2. When a valid, fresh radio-altimeter measurement reports **AGL > 100 m**, display the barometric altitude explicitly referenced to standard pressure (1013.25 hPa), provided that measurement is valid and fresh.
3. The boundary is inclusive on the radio-altimeter side: exactly 100.0 m uses radio altitude; 100.1 m selects the barometric source.
4. If radio-altimeter data is unavailable or invalid, do not silently present barometric altitude as equivalent to AGL. A barometric value above 100 m may be shown only as a degraded fallback; the display must identify the source and degraded state. If no such fallback is available, the selected value is unavailable.

## Data contract

Each input reading must retain:
- numeric value in metres;
- source identity;
- source timestamp and receive timestamp;
- adapter-provided validity and freshness state.

The selector rejects invalid, stale, non-finite, provenance-free, or timestamp-inconsistent readings. It does not guess a source from an ambiguous generic `altitude_m` field. The adapter must distinguish radio-altimeter AGL from barometric pressure altitude and must guarantee that the barometric source uses the standard-pressure reference.

## Important limitation

Radio altitude is height above the surface directly beneath the vehicle; barometric standard-pressure altitude is a pressure-derived altitude and is not AGL. Switching sources therefore changes the vertical reference and may create a numerical discontinuity. The UI must label the selected source (for example, `RA · AGL` versus `BARO · STD 1013.25 hPa`) rather than presenting the values as one continuous measurement.

This policy selects a display source only. It does not control the vehicle, calculate terrain clearance, prove that the radio altimeter is installed, or establish mission readiness.

## Integration gap

The current normalized telemetry/MAVLink contracts do not yet expose separate radio-altimeter AGL and standard-pressure barometric readings with per-field validity/freshness. The current QML profile also contains preview values and is not connected to this selector. The policy is implemented and unit-tested in the operations contract layer, but it is **not yet wired to the live HMI**. Completing that requires explicit source mapping in the adapter, capability discovery, coherent per-field timestamps, and a visible source/degraded annunciation.

## Verification

`altitude_display_source_policy_test` covers values below, at and above 100 m; missing/stale radio data; missing barometric data; non-finite/negative radio readings; and malformed provenance/timestamps.
