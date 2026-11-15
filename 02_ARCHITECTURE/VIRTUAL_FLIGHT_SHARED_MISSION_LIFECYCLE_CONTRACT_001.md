# BlueSky PRO — Virtual Flight Uses the Shared Mission Lifecycle 001

**Status:** CONTROLLED ARCHITECTURE CONTRACT  
**Scope:** One mission lifecycle for physical and virtual execution  
**Authority:** `02_ARCHITECTURE/BLUESKY_END_TO_END_OPERATIONAL_LIFECYCLE_BASELINE.md`

## 1. Binding rule

A mission has one canonical lifecycle regardless of whether its execution is performed by a physical UAV or by Virtual Flight. Virtual Flight is not a separate abbreviated workflow, preflight-only assessment, demo mode, or alternative mission state machine.

The same mission model, task interpretation, vehicle capability/configuration rules, algorithm orchestration, route generation, NOTAM and terrain handling, weather application, performance and energy calculations, validation gates, readiness/action graph, authorization/release contracts, mission package semantics, execution-state transitions, result verification, evidence consolidation and post-mission analysis apply in both modes.

The only operational substitution is the aircraft execution endpoint:

- **Physical mode:** a verified physical aircraft and its live adapter/session provide vehicle identity, telemetry, command acknowledgement and physical execution.
- **Virtual mode:** a selected catalogue aircraft model plus a versioned, validated configuration/performance profile are bound to the mission; the simulator provides virtual telemetry, command acknowledgement and simulated execution.

No parallel optimizer, validation engine, readiness gate, lifecycle controller, mission package contract or result model is permitted.

## 2. Shared lifecycle

Both modes use the canonical stages already defined by the end-to-end lifecycle baseline:

`OBJECTIVE → MISSION MODEL → CAPABILITY / VEHICLE BINDING → TASK DECOMPOSITION → ALGORITHM ORCHESTRATION → ROUTE CANDIDATES → SAFETY / REGULATORY GATES → WEATHER APPLICATION → ROUTE / TIME / ENERGY RECALCULATION → SELECTED SOLUTION → FORECAST VIRTUAL FLIGHT → PACKAGE → VALIDATION → READINESS → AUTHORIZATION → RELEASE → UPLOAD / SIMULATOR LOAD → READ-BACK → EXECUTION → MONITORING / REPLANNING → COMPLETION → RESULT / QUALITY VERIFICATION → EVIDENCE → REPLAY / ANALYSIS`

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

The same authorization applicability, scope and validity gates apply. Virtual mode must use an explicitly configured sandbox/test authority adapter when external authorization workflow is exercised. It must not submit a real operational authorization, release a real flight or issue commands to a physical aircraft as a side effect of simulation.

This sandbox boundary changes the destination of external effects, not the mission lifecycle stages or validation semantics. A sandbox response must be identified as simulated/test evidence and must never be represented as a real regulatory approval.

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

`MissionLifecycleController` already admits `PHYSICAL_LIVE` and `SIL_FIXTURE` at initial vehicle binding and pins vehicle/model/configuration/performance identity across subsequent transitions. Its execution and completion transitions are provenance-neutral and consume the same evidence contract. This is the intended shared foundation.

The current `VirtualFlightAssessmentBridge` is **not** the complete Virtual Flight implementation. It currently evaluates manual geometry against synthetic airspace/NOTAM fixtures, explicitly sets `flightReleaseEligible=false`, and states that weather thresholds, terrain, obstacles, energy, authorizations and flight release are outside its assessment scope. It must remain a non-operational diagnostic fixture until connected to the shared mission lifecycle and full validation/orchestration chain. Its `ASSESSMENT_CLEAR_NON_OPERATIONAL` result must never be mapped to mission `READY`, `RELEASED` or `COMPLETED`.

The next implementation work is to route Virtual Flight through the existing mission and lifecycle services and supply simulator adapters for the same package, execution, telemetry and result contracts. Do not expand the assessment bridge into a second lifecycle controller.

## 9. Acceptance criteria

1. Physical and virtual missions are created from the same canonical mission model and use the same lifecycle controller.
2. Both modes invoke the same algorithm orchestration, NOTAM/terrain/weather, vehicle-envelope, camera/task-quality and energy-reserve validation contracts.
3. Both modes use the same readiness, authorization-applicability, release and completion transition rules.
4. The only execution substitution is physical adapter versus simulator adapter; no physical UAV is required for Virtual Flight.
5. The selected model/configuration/performance revisions remain pinned and traceable throughout the mission.
6. A complete SIL lifecycle can reach simulated execution and simulated completion using the same state transitions as a physical lifecycle.
7. SIL evidence is never promoted to physical-flight evidence or used to satisfy physical fleet admission.
8. Virtual READY is visibly scoped to simulated execution and cannot authorize physical flight.
9. Simulator command/read-back/telemetry/result evidence is versioned and carries SIL provenance.
10. The current route/NOTAM-only assessment bridge cannot produce READY or release eligibility.
11. Tests exercise the same lifecycle transition sequence for both `PHYSICAL_LIVE` and `SIL_FIXTURE`, with mode-specific identity/evidence and no mode-specific lifecycle bypass.

## 10. Definition of done

A representative mission completes the same end-to-end lifecycle in Virtual Flight and physical mode using the same mission model, planner, validators, readiness logic and lifecycle controller. Only the aircraft/execution adapter and provenance of execution evidence differ. Both paths produce auditable evidence; the virtual path is explicitly marked as simulated and cannot be mistaken for a physical flight.
