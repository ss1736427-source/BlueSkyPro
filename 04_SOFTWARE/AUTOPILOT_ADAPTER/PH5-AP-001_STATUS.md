# PH5-AP-001 — ArduPilot MAVLink2 Reference Adapter Boundary

**Status:** IMPLEMENTATION PREPARATION — VERIFICATION PENDING  
**Evidence boundary:** `SIL_BOUNDARY_ONLY`  
**Protocol:** `MAVLINK2`  
**Reference FCS:** `ArduPilot`

## Purpose

Establish the first concrete reference-autopilot implementation boundary required by Phase 5 without coupling BlueSky core to ArduPilot or MAVLink types.

## Scope

1. ArduPilot reference adapter class behind the Universal Autopilot Adapter contract.
2. Normalized vehicle identity reporting.
3. Explicit MAVLink2 protocol boundary.
4. Representative capability declaration.
5. Automated CMake/CTest verification.

## Acceptance

`UNIVERSAL CONTRACT → ARDUPILOT ADAPTER → MAVLINK2 BOUNDARY`

The test shall prove that the reference adapter exposes ArduPilot/MAVLink2 identity and capabilities through the normalized interface.

## Explicit boundary

This stage does **not** claim real MAVLink transport, ArduPilot SITL, HIL, physical-UAV operation, flight-test evidence or certification evidence. Those require subsequent stages.

## Exit condition

Close only after Configure, Build and Test pass in CI and the PR is merged to `main`.

## Next gate

Bind the ArduPilot reference adapter to a controlled MAVLink2 transport/SITL lifecycle while preserving the universal API and evidence boundary.
