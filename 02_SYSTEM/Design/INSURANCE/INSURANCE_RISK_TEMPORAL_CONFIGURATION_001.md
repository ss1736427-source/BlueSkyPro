# INSURANCE-RISK-004 — Temporal and Configuration-Specific Risk Analytics

## Purpose
Provide deterministic time-window and configuration-specific analytics for the persistent `INSURED_UAV` risk history.

## Data path
`RISK OBSERVATION / CONFIGURATION EXPOSURE → TEMPORAL CALCULATOR → TIME/CONFIGURATION METRIC + LINEAGE`

## Rules
- The persistent insurance object remains the specific UAV; configuration is an effective operational context, not a separate insured object.
- Time windows are explicit half-open intervals `[start, end)`; overlapping baseline/current windows are rejected.
- Numeric observations are compared between baseline and current windows using valid, timestamped evidence.
- Configuration exposure metrics use only exposures with a non-empty configuration identifier and valid timestamps/evidence.
- Metrics retain UAV identity, configuration identity where applicable, both window boundaries, source record IDs, method/version, and calculation timestamp.
- No opaque overall risk score is produced.
- Missing/invalid evidence is excluded from authoritative calculations.
- This layer does not decide insurance applicability, policy coverage, readiness, safety, authorization, execution, or claims liability.
- AI may consume temporal/configuration metrics for analysis or proposals but cannot promote them to authoritative flight decisions.

## Metrics
- Observation mean delta: current-window mean minus baseline-window mean.
- Observation rate of change: mean delta divided by the elapsed time between the two window midpoints.
- Configuration flight-hour exposure delta.
- Configuration BVLOS exposure ratio.
- Configuration adverse-weather exposure ratio.
- Configuration altitude-hour exposure ratio.

## Verification
`insurance_risk_temporal_test` verifies UAV binding, non-overlapping windows, timestamp filtering, configuration separation, deterministic deltas/ratios, and lineage.
