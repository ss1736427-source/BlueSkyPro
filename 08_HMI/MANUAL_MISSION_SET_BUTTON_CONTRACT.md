# Manual Mission SET Button Contract

**Status:** HMI behavior requirement  
**Scope:** Manual mission setup only

1. The manual mission action is labelled **SET** (УСТАНОВИТЬ), not Validate / Validation.
2. It is available only in manual mission setup and only after manual composition is complete.
3. Pressing SET submits the manually defined mission, route geometry, and constraints to the authoritative automatic planning/validation pipeline. The HMI must not validate locally or infer success from the click.
4. While the authoritative check is pending, show a checking state; do not show success.
5. SET becomes fully green only after a fresh result for the active mission reports `finalGateStatus=PASS`, `releaseStatus=RELEASE_ELIGIBLE`, and `verified=true`.
6. A failed or blocked result must not turn the control green; return to editable manual setup for correction and retry.
7. A previous result, a result for another mission, or a UI state transition is not evidence of success.
8. SET does not replace the automatic mission workflow or authorize flight by itself.

## Integration requirement

The request must carry actual manual task geometry and constraints. Do not fabricate missing geometry, environment snapshots, vehicle data, or route candidates. The HMI's SET click and status display are not a substitute for wiring the manual task model to PlanningBridge and the authoritative planning algorithm.
