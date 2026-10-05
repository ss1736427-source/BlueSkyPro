# PH4-INT-021 — Navigation lifecycle correlation

Status: CLOSED — CI VERIFIED

## Objective
Verify deterministic navigation-source lifecycle handling and preserve source/vehicle/flight correlation through the controlled SIL fixture path.

## Scope
- reject navigation sample from an unexpected source;
- accept a valid navigation sample;
- preserve source and adapter timestamps;
- read back the accepted sample without correlation loss;
- preserve navigation quality and correction state;
- reject stale data;
- reject invalid data;
- preserve flight/mission/vehicle/source/protocol correlation in EvidenceSession.

## Verification
GitHub Actions workflow `BlueSky Autopilot Adapter`, run #523, completed successfully.

Verified chain:
- repository consistency validation — PASS;
- Configure — PASS;
- Build — PASS;
- verification evidence recorder — PASS;
- Test — PASS.

The PH4-INT-021 navigation fixture and the existing PH4-INT-015…020 verification chain passed in CI.

## Evidence boundary
`SIL_FIXTURE_ONLY`.

No real GNSS receiver, RTK/NTRIP service, HIL, real-aircraft execution, flight-test evidence, or certification evidence is claimed.

## Exit criterion
Satisfied: CI passed the PH4-INT-021 navigation fixture and EvidenceSession verification without error masking, while preserving the existing PH4-INT-015…020 test chain.

## Merge
PR #50 (`PH4-INT-021: navigation lifecycle correlation`) merged into `main`.
Merge commit: `209141702abf0cd137fd245ec0aa4187cd705a8c`.
