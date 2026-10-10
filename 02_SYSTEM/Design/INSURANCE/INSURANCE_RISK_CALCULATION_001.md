# INSURANCE-RISK-003 — Deterministic Risk Calculation and Lineage

## Purpose
Provide reproducible derived indicators from the persistent `INSURED_UAV` risk observations and exposure records.

## Data path
`RISK OBSERVATION / EXPOSURE → DETERMINISTIC CALCULATOR → DERIVED INDICATOR + SOURCE LINEAGE`

## Rules
- Calculations are deterministic and versioned.
- Observations are aggregated only when `RiskDimension` and unit match.
- Each derived metric retains source record IDs, calculation method, method version, calculation timestamp, UAV identity and evidence validity.
- Exposure indicators use valid flight-hour denominators from the same UAV.
- The layer produces a vector of indicators; it does not collapse dimensions into an opaque overall score.
- Invalid observations are excluded from aggregation. Invalid exposure records are excluded from exposure denominators.
- No metric is authoritative without valid evidence, source lineage, a calculation method, and a method version.
- This layer does not decide legal insurance applicability, policy coverage, readiness, safety, authorization, execution, or claims liability.
- AI may consume these indicators for analysis/proposals, but cannot promote them to authoritative flight decisions.

## Initial deterministic indicators
- Mean and maximum for numeric observations grouped by dimension and unit.
- BVLOS exposure ratio.
- Adverse-weather exposure ratio.
- Altitude-hours ratio.
- Cycles per flight hour.

## Verification
`insurance_risk_calculator_test` verifies deterministic calculation, UAV binding, dimension/unit grouping, exposure normalization, evidence lineage and versioning.
