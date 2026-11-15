# BlueSky PRO — Virtual Flight Uses the Shared Mission Lifecycle 001

**Status:** CONTROLLED ARCHITECTURE CONTRACT  
**Scope:** Shared mission core with isolated physical and virtual execution lifecycles  
**Authority:** `02_ARCHITECTURE/BLUESKY_END_TO_END_OPERATIONAL_LIFECYCLE_BASELINE.md`

## 1. Binding rule

A mission uses one canonical mission model, planning core, validators and lifecycle controller, but its execution mode is pinned at mission admission. Physical and virtual missions are separate mission contexts and are never prepared or executed concurrently in the same active context. Selecting `VIRTUAL FLT` in the bottom toolbar enters the virtual workspace; it must bind a `VIRTUAL` lifecycle context. Returning to the physical workspace must not reuse virtual readiness, release, authorization or execution state.

The same mission model, task interpretation, vehicle capability/configuration rules, algorithm orchestration, route generation, NOTAM and terrain handling, weather application, performance and energy calculations, internal safety/regulatory validation, readiness/action graph, mission package semantics, execution-state transitions, result verification, evidence consolidation and post-mission analysis apply in both modes. External authorization and regulatory submission are mode-specific and must not be invoked by the virtual path.

The execution endpoint and external authorization path are mode-specific:

- **Physical mode:** a verified physical aircraft and its live adapter/session provide vehicle identity, telemetry, command acknowledgement and physical execution.
- **Virtual mode:** a selected catalogue aircraft model plus a versioned, validated configuration/performance profile are bound to the mission; the simulator provides virtual telemetry, command acknowledgement and simulated execution.

No parallel optimizer, validation engine, readiness gate, lifecycle controller, mission package contract or result model is permitted.

## 2. Shared lifecycle

Both modes use the canonical stages already defined by the end-to-end lifecycle baseline:

`OBJECTIVE → MISSION MODEL → CAPABILITY / VEHICLE BINDING → TASK DECOMPOSITION → ALGORITHM ORCHESTRATION → ROUTE CANDIDATES → INTERNAL SAFETY / REGULATORY GATES → WEATHER APPLICATION → ROUTE / TIME / ENERGY RECALCULATION → SELECTED SOLUTION → PACKAGE → VALIDATION → READINESS → MODE-SPECIFIC RELEASE → PHYSICAL UPLOAD OR SIMULATOR LOAD → READ-BACK → EXECUTION → MONITORING / REPLANNING → COMPLETION → RESULT / QUALITY VERIFICATION → EVIDENCE → REPLAY / ANALYSIS`

The lifecycle controller is mode-neutral. Mode-specific adapters satisfy the same domain contracts and return evidence with explicit provenance.

## 3. Aircraft binding

A virtual mission does not require a physical UAV, serial number, live connection or live telemetry.

It does require:
- a selected catalogue model and exact variant;
- a versioned vehicle configuration and equipment/payload configuration;
- an applicable performance/safety profile with approved evidence;
- stable virtual vehicle identity for the simulation session;
- pinned model, configuration, performance profile and simulator revisions.

A catalogue record alone is not proof of a physical aircraft and cannot be admitted as `PHYSICAL_LIVE`. Virtual resources must retain `SIL_FIXTURE` provenance (or a separately defined simulator provenance class if the controlled provenance taxonomy is later extended). The same pinned identity and revisions must remain attached to all mission transitions and evidence.

## 4. Same gates, different evidence providers

The validation criteria and acceptance thresholds remain the same. The source of evidence is mode-specific:

| Evidence domain | Physical mode | Virtual mode |
|---|---|---|
| Vehicle identity | Verified airframe identity | Stable virtual session/model identity |
| Configuration | Read-back/verified installed configuration | Selected versioned model configuration |
| Performance/safety | Approved profile for actual configuration | Approved profile applicable to the selected model/configuration |
| Telemetry | Live vehicle telemetry | Simulator telemetry |
| Command acknowledgement | Physical adapter/autopilot ACK | Simulator adapter ACK |
| Package read-back | Read-back from physical autopilot | Read-back from simulator execution endpoint |
| Execution record | Physical flight/session record | Simulation execution record |
| Post-flight/result evidence | Collected real mission data | Simulated result/data with SIL provenance |

A missing or unverified safety-critical performance value blocks both modes wherever the same calculation depends on it. Virtual mode must not replace unknown values with typical values or fabricated telemetry.

## 5. Authorization and external effects

Internal airspace, NOTAM, terrain, weather, vehicle and task-safety validations remain active in both modes. Physical mode alone may prepare and submit a flight plan to ЕСОРВД, consume its response, and perform the applicable external authorization workflow.

Virtual mode MUST NOT call an ЕСОРВД submission or flight-plan-check adapter, MUST NOT create an ЕСОРВД submission identifier, and MUST NOT wait for external authorization. Its lifecycle transitions directly from `READY` to `RELEASED` after package-integrity checks and an explicit `VIRTUAL_SANDBOX_NO_ESORVD_SUBMISSION_OR_AUTHORIZATION` applicability record. The virtual release is a simulator package release only, never a physical flight release.

## 6. READY semantics

The same readiness action graph and mandatory checks apply in both modes. System-completed actions are not shown as pilot checklist tasks. The checklist contains only actions requiring human participation.

A virtual mission may reach lifecycle `READY` when the shared gates pass for its virtual execution context. This means **ready for simulated execution**, not physically cleared for real flight. The UI must display the execution mode and evidence provenance adjacent to the status; it must not imply that a virtual READY authorizes a physical flight.

The physical mission can reach `READY` only when the physical aircraft admission and all physical-context gates are satisfied. A SIL result cannot satisfy a physical admission, live connection, live telemetry or physical flight evidence requirement.

## 7. Virtual execution

Virtual execution must consume the same released mission package semantics and follow the same execution-state transitions as physical execution. The simulator must provide the same normalized interface classes for:
- loading and read-back verification;
- start, pause, continue, replan and abort;
- telemetry and event delivery;
- execution closure;
- result and post-flight evidence.

The simulator may model vehicle dynamics, sensor/payload behaviour and environment response. It must not replace the shared planning, validation or lifecycle logic with a simplified route/NOTAM-only assessment.

## 8. Existing implementation boundary and known gap

`MissionLifecycleController` now pins `MissionExecutionMode` at initial vehicle binding and requires `PHYSICAL_LIVE` for `PHYSICAL` or `SIL_FIXTURE` for `VIRTUAL`. Mode and vehicle/model/configuration/performance identity are immutable across subsequent transitions. The physical path retains external submission/authorization states; the virtual path cannot enter them.

The `VIRTUAL FLT` toolbar entry is the existing user entry point and selects the virtual workspace. However, `BottomToolbar.activateTool()` currently changes the active tool/workspace context; the central `MainContent.qml` view still renders a watermark overlay. `VirtualFlightTestConsole.qml` is compiled as a resource but is not instantiated by `MainContent.qml`; it is a synthetic diagnostic console, not the simulator. `VirtualFlightAssessmentBridge` evaluates manual geometry against synthetic airspace/NOTAM fixtures, explicitly sets `flightReleaseEligible=false`, and does not cover the full production planning chain. Its `ASSESSMENT_CLEAR_NON_OPERATIONAL` result must never be mapped to mission `READY`, `RELEASED` or `COMPLETED`.

The repository already contains a useful SIL execution foundation: `PH4-INT-014` provides a provenance-bound SIL execution source, and `PH4-INT-027` demonstrates a SIL adapter sequence for connect, identify, capabilities, compile, upload, read-back, semantic verification, execute, log acquisition and close. The new `PH5-VF-001` integration fixture now binds that adapter sequence and SIL execution evidence to the canonical `MissionLifecycleController` and exercises one correlated synthetic mission through `COMPLETED`. This proves the shared lifecycle transition and adapter evidence contract at fixture level only; it does not yet connect production optimizer outputs or real simulator telemetry. ``PlanningBridge` now publishes the pinned execution mode and vehicle provenance with readiness, rejects mode/provenance drift across lifecycle history, and the HMI gates readiness presentation on the active workspace mode. The lifecycle controller now pins `MissionExecutionMode` and provenance, allows virtual `READY → RELEASED` without `SUBMITTED` / `AUTHORIZED`, rejects virtual `SUBMITTED`, and requires the explicit no-ЕСОРВД applicability reason. The remaining integration gap is to connect the existing button-selected virtual workspace to a real virtual mission context, replace synthetic evidence with actual production planning/validation results, and connect a simulator implementation to the same package, telemetry, command, replanning and post-flight contracts. Do not expand the assessment bridge into a second lifecycle controller.

## 9. Acceptance criteria

1. Physical and virtual missions are created from the same canonical mission model and use the same lifecycle controller, but pin distinct execution modes and independent mission contexts.
2. Both modes invoke the same algorithm orchestration, NOTAM/terrain/weather, vehicle-envelope, camera/task-quality and energy-reserve validation contracts.
3. Both modes use the same internal readiness and completion rules; physical mode alone uses ЕСОРВД submission/response and external authorization. Virtual mode transitions `READY → RELEASED` directly with an explicit no-ЕСОРВД record.
4. The only execution substitution is physical adapter versus simulator adapter; no physical UAV is required for Virtual Flight.
5. The selected model/configuration/performance revisions remain pinned and traceable throughout the mission.
6. A complete SIL lifecycle can reach simulated execution and simulated completion using the same state transitions as a physical lifecycle.
7. SIL evidence is never promoted to physical-flight evidence or used to satisfy physical fleet admission. Virtual lifecycle state cannot leak into the physical workspace.
8. Virtual READY is visibly scoped to simulated execution and cannot authorize physical flight.
9. Simulator command/read-back/telemetry/result evidence is versioned and carries SIL provenance.
10. The current route/NOTAM-only assessment bridge cannot produce READY or release eligibility.
11. Tests exercise the common planning/validation stages and mode-specific transition paths: physical uses `READY → SUBMITTED → AUTHORIZED → RELEASED`; virtual uses `READY → RELEASED` and rejects all external submission/authorization states.

## 10. Definition of done

A representative mission completes the shared planning and validation chain in each mode using the same mission model, planner, validators and lifecycle controller. The execution mode is pinned and contexts are isolated. Physical mode may use ЕСОРВД; virtual mode must not submit or check a flight plan there and releases only to a simulator. Both paths produce auditable evidence; virtual evidence is explicitly marked as simulated and cannot be mistaken for physical flight evidence.
