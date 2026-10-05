# ARCH-OPS-053 — Manual / Override Authority Boundary

**Status:** IMPLEMENTED — CI PENDING FOR IMPLEMENTATION COMMITS  
**Evidence boundary:** `SIL_BOUNDARY_ONLY`

## Approved policy basis

The user approved the authority-state × command-class matrix as the implementation basis.

## Deterministic command-admission matrix

| Authority state | Admission rule |
|---|---|
| `AUTONOMOUS_MISSION` | Admit `MissionManagement` only |
| `GUIDED_MANUAL_SUPERVISED` | Admit `Supervisory` only when explicitly approved |
| `FAILSAFE` | Reject external commands; onboard failsafe retains authority |
| `RETURN_RECOVERY` | Admit `RecoveryNonInterfering` only when explicitly approved |
| `EMERGENCY_ABORT` | Admit `EmergencyAbort` only when explicitly approved |
| `COMMUNICATION_LOST` | Reject all commands |
| Unknown state | Reject all commands |

All command classes not explicitly admitted by the current state are rejected. Low-level actuation and manual-control commands are not admitted by this gate.

## Implementation

- `core/manual_override_authority_gate.hpp`
- `core/manual_override_authority_gate.cpp`
- `core/manual_override_authority_gate_test.cpp`
- Registered as `manual_override_authority_gate_test` in CMake/CTest.

The gate returns a deterministic admit/reject result with a reason. This is an admission boundary only; it does not dispatch commands or implement vendor-specific RC/autopilot behavior.

## Safety invariants

- Loss of BlueSky communication does not transfer flight-control authority.
- Onboard stabilization and failsafe behavior remain authoritative.
- Unknown authority state never authorizes a command.
- Failsafe state rejects external commands.
- Explicit approval is required for supervisory, recovery, and emergency-abort command classes.
- No low-level actuator commands or vendor-specific RC semantics are introduced.

## Verification status

SIL-oriented deterministic unit tests are registered. CI has not yet been observed for the implementation commits; do not mark this slice CI-verified until a workflow run succeeds on the resulting code commit.

This implementation does not establish HIL, real-UAV, operational, or certification verification.
