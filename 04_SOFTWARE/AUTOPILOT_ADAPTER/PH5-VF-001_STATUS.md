# PH5-VF-001 — Virtual Flight Shared Mission Lifecycle Integration

**Status:** SIL LIFECYCLE MODE-SEPARATION CONTRACT IMPLEMENTED; CI VERIFICATION PENDING  
**Evidence class:** `SIL_FIXTURE_ONLY`  
**Execution model:** DJI Mavic 3E catalogue reference only; no operational performance profile admitted

## Objective

Prove that the existing SIL autopilot adapter execution path can drive the same canonical `MissionLifecycleController` transitions used by physical execution, without introducing a second mission state machine.

## Delivered

- Added `core/ph5_vf_001_shared_mission_lifecycle_fixture.cpp`.
- Registered `ph5_vf_001_shared_mission_lifecycle_fixture` in this module's CMake/CTest configuration.
- The fixture creates one canonical mission lifecycle record and exercises:
  `DRAFT → CALCULATING → VALIDATING → READY → RELEASED → EXECUTING → COMPLETED`.
- The fixture pins `MissionExecutionMode::Virtual` and `SIL_FIXTURE` provenance.
- Virtual release is a direct `READY → RELEASED` transition. It rejects `SUBMITTED`, carries no regulatory submission ID, and requires the explicit `VIRTUAL_SANDBOX_NO_ESORVD_SUBMISSION_OR_AUTHORIZATION` applicability reason.
- Before entering `EXECUTING`, the existing `AutopilotAdapterBaseline` performs connect, mission compile, upload, read-back and semantic comparison.
- An acknowledged adapter start command is recorded through the existing `SilExecutionSource`; its flight-record reference is bound into the lifecycle execution evidence.
- Completion is admitted only after the adapter returns a completed flight-log acquisition and a successful clean close. The post-flight record reference is preserved in the terminal transition.
- Every lifecycle event is checked for stable vehicle identity, `SIL_FIXTURE` provenance, model reference and performance-profile reference.
- The test-only profile identifier is explicitly `SIL-FIXTURE-PERF-NOT-OPERATIONAL`; the vehicle model identifier is catalogue-reference-only. Neither is an approved flight performance profile.
- The physical lifecycle retains `READY → SUBMITTED → AUTHORIZED → RELEASED`; the virtual lifecycle cannot enter those external authorization stages.

## Important evidence boundary

This is a deterministic integration contract test. It uses synthetic validation attestations to exercise the shared lifecycle transition contract and a baseline SIL adapter fixture. It does **not** prove that the full production route optimizer, live NOTAM/terrain/weather providers, camera task-quality engine, regulatory services, or a physics simulator are connected end-to-end.

The fixture must not be used to claim real Mavic 3E performance validation, physical flight readiness, regulatory authorization, HIL, flight-test or certification evidence. The normal production readiness gates remain authoritative and must consume actual versioned evidence.

## DJI Mavic 3E source boundary

The manufacturer's published specifications are available at:
- https://enterprise.dji.com/mavic-3-enterprise/specs
- https://dl.djicdn.com/downloads/DJI_Mavic_3_Enterprise/20240606/DJI_Mavic_3E_3T_User_Manual_EN.pdf

Published maximum flight time, speed, wind resistance and camera specifications are reference limits under stated manufacturer test conditions. They are not, by themselves, an approved configuration-specific dynamic envelope, endurance model, energy reserve model, or camera quality guarantee for BlueSky PRO. The operational performance profile must be separately versioned and validated for the exact aircraft, battery, propeller, payload, firmware and task configuration. No such operational profile is created by this fixture.

## Acceptance criteria

1. One SIL adapter run traverses the canonical `MissionLifecycleController`; no parallel SIL lifecycle is created.
2. Adapter upload and read-back are semantically compared before execution is admitted.
3. The execution reference and vehicle identity match the released mission assignment.
4. SIL provenance remains attached to all lifecycle events.
5. A completed lifecycle requires both archived post-flight evidence and adapter clean-close confirmation.
6. The test remains explicitly non-operational and cannot be interpreted as proof of physical READY.

## Remaining integration work

1. Connect the production mission model and real planning/validation outputs to the same lifecycle record; remove all synthetic evidence from the production path.
2. Connect the simulator implementation to the normalized adapter contract for telemetry, pause/resume, abort, replanning and execution completion.
3. The HMI readiness snapshot now carries `executionMode`; the active `VIRTUAL FLT` workspace accepts only `VIRTUAL` + `SIL_FIXTURE`, while the physical workspace accepts only `PHYSICAL` + `PHYSICAL_LIVE`. The workspace still has no production simulator execution controls or live simulator telemetry; the existing diagnostic assessment console is not a simulator.
4. Build and validate the exact DJI Mavic 3E configuration/performance profile from authoritative source documents and configuration evidence. Keep unknown limits blocking; do not substitute catalogue maxima as operational limits.
5. Verify the full HMI and adapter CI runs on the exact merge candidate SHA.

## Definition of done

A real planning mission, using the same production planning and validation chain as physical mode, is released to a simulator adapter and completed through the canonical mission lifecycle. The simulator supplies versioned telemetry and execution evidence, while the mission, vehicle configuration, route, weather/NOTAM/terrain snapshots, validations, and evidence lineage remain pinned and auditable. The result is explicitly marked `SIL_FIXTURE_ONLY` / simulated and cannot satisfy physical-flight evidence requirements.
