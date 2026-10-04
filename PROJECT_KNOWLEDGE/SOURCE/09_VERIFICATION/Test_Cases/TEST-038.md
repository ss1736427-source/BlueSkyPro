---
id: TEST-038
type: test_case
title: UAV Capability Profile Inspection
status: draft
verifies:
  - "[[SYS-REQ-076]]"
verification_method: inspection
result: not_run
---

# TEST-038 — UAV Capability Profile Inspection

## Objective

Verify [[SYS-REQ-076]] by inspection of the machine-readable UAV capability and limitation profile.

## Preconditions

- [[ARCH-027]] available.
- UAV capability/profile definitions available, where implemented.

## Inspection Procedure

1. Identify the machine-readable UAV capability profile.
2. Verify the presence of platform_type.
3. Verify the presence of payload_capabilities.
4. Verify the presence of endurance.
5. Verify the presence of range.
6. Verify the presence of speed.
7. Verify the presence of altitude_limits.
8. Verify the presence of navigation_capabilities.
9. Verify the presence of communication_capabilities.
10. Verify the presence of sensing_capabilities.
11. Verify the presence of landing_capabilities.
12. Verify the presence of current_energy.
13. Verify the presence of current_health.
14. Verify the presence of current_mission_load.
15. Verify that the profile is machine-readable and represents both capabilities and current limitations/state where required.

## Expected Result

Each connected UAV has a machine-readable capability profile containing all required capability, limitation and current-state fields.

## Actual Result

Not inspected.

## Result

Not Run

## Evidence Required

- UAV profile schema.
- Representative machine-readable UAV profile.
- Field definitions.
- Capability and limitation mapping.

## Traceability
- [[SYS-REQ-076]]
- [[ARCH-027]]



