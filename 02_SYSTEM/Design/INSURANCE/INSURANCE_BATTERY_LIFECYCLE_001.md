# INSURANCE-RISK-005 — Battery Lifecycle and Degradation Analytics

## Purpose
Provide deterministic, traceable battery lifecycle indicators for the persistent insured UAV history.

## Model boundary
The battery is a component of the insured UAV, not a separate insured object. These indicators describe observed lifecycle behaviour and do not constitute a universal degradation coefficient.

## Inputs
- UAV identity
- battery identity
- controlled configuration identity
- state of health
- cycle count
- temperature
- timestamp
- source/evidence lineage
- calculation method/version

## Derived indicators
- state-of-health delta across valid observations;
- state-of-health loss per cycle;
- cycle-count delta;
- mean observed battery temperature;
- elapsed observation age.

## Rules
- Only valid, evidenced observations for the target UAV are used.
- Battery and configuration identities are retained.
- Observations are ordered by timestamp before lifecycle calculation.
- A universal threshold or degradation coefficient is not introduced.
- Negative cycle regressions are not converted into an authoritative lifecycle rate.
- All derived metrics retain source record IDs, method/version and calculation time.
- This layer does not decide legal insurance applicability, policy coverage, readiness, safety, authorization, execution or claims liability.
- The metrics may feed the Energy Model and AI analysis only through controlled interfaces.

## Verification
`insurance_battery_lifecycle_test` verifies UAV/battery/configuration binding, deterministic SOH/cycle/temperature/age metrics and evidence lineage.
