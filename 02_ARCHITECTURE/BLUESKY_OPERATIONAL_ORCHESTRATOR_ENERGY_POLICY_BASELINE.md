# BlueSky PRO — Operational Orchestrator & Energy Policy Baseline

**Status:** ARCHITECTURE BASELINE
**Purpose:** establish the cross-cutting operational decision policy for the unified BlueSky PRO UAV mission system.

## 1. Architectural intent

BlueSky PRO is the unified operational system for the UAV lifecycle from task definition through verified mission completion and controlled post-flight learning. ArduPilot, PX4 and OEM FCS are possible onboard execution contours; BlueSky is not an FCS replacement and must not duplicate low-level flight-control responsibility.

The Operational Orchestrator coordinates the canonical mission, vehicle, payload, environment, regulatory, C2, energy, readiness, execution and evidence contracts. Existing Algorithm Orchestration remains a planning capability inside this larger operational control model; it is not the whole system orchestrator.

**Additive rule:** this baseline extends existing Mission Model, Objective Profiles, Capability, Algorithm Orchestration, Runtime, C2, Mission Package, Validation and Intelligence contracts. It does not replace, rename or invalidate them.

## 2. Core decision principle

BlueSky shall select the solution and next action that provides the best achievable combination of:

1. successful mission completion;
2. mandatory safety and regulatory compliance;
3. required task/result quality;
4. preservation of a protected energy/fuel reserve for foreseeable and unforeseen changes;
5. minimum necessary time;
6. efficient use of energy/fuel and other constrained resources.

Energy/fuel reserve is a protected constraint, not merely a residual quantity after optimization.

The system shall not optimize speed, distance, coverage or resource consumption in isolation when doing so reduces the required reserve or threatens successful completion.

## 3. Operational outcome model

A mission may define more than one acceptable operational outcome. BlueSky shall represent the task objective and its acceptance conditions separately from the selected route, UAV allocation or algorithm.

The orchestrator shall evaluate candidate solutions against the mission's mandatory acceptance conditions and then optimize the applicable soft objectives. Examples include completion quality, completion time, energy/resource efficiency, number of UAVs/sorties and operational robustness.

The system shall not require the pilot to manually choose among optimization algorithms or internal objective weights.

## 4. Energy as a cross-cutting constraint

The Energy Engine supplies authoritative energy state, prediction, uncertainty, reserve requirements, degradation effects and resource cost for candidate actions. The Operational Orchestrator combines these outputs with safety, mission, environment, regulatory and operational constraints.

Energy does not act alone as the decision-maker. The orchestrator shall reject candidate solutions that violate mandatory constraints even if they appear energetically efficient.

The energy policy applies to, at minimum:

- vehicle selection;
- fleet composition;
- payload selection/configuration;
- route and alternative-route selection;
- altitude and speed profiles;
- departure timing;
- task partitioning;
- multi-UAV redistribution;
- replanning;
- contingency response;
- return/recovery decisions;
- selection of the next preparation or operational action.

## 5. Protected reserve and uncertainty boundary

Every executable plan shall contain an explicit protected reserve appropriate to the mission, vehicle, environment, uncertainty and approved operational policy.

The reserve shall account for relevant uncertainty and foreseeable changes, including where applicable:

- forecast-versus-actual wind deviation;
- energy-model uncertainty;
- battery/fuel condition and degradation;
- payload-dependent consumption;
- route changes;
- C2 degradation and recovery;
- contingency manoeuvres;
- return/recovery requirements;
- other approved operational uncertainties.

A candidate plan that consumes the protected reserve merely to improve nominal efficiency is not an acceptable optimization result.

**No new numerical reserve value is introduced by this architecture baseline.** Numerical coefficients, uncertainty bounds and acceptance thresholds remain subject to the existing Energy Model engineering/qualification path.

## 6. Dynamic mission adaptation

BlueSky may automatically adapt an active mission within a pre-authorized adaptation envelope.

Automatic adaptation may include, where contractually permitted:

- route/profile adjustment;
- redistribution of work between available UAVs;
- speed/altitude adjustment;
- task splitting or consolidation;
- replacement of an unavailable vehicle;
- replanning in response to environmental or operational changes.

Changes outside the authorized envelope, or changes requiring human authority by regulation, mission authorization or safety policy, shall be presented for human decision.

No automatic adaptation may bypass immutable safety, regulatory or authorization constraints.

## 7. Next-best-action policy

Preparation and operation are represented as a dependency-aware action graph, not a rigid linear checklist.

For each state, BlueSky shall:

1. determine the current canonical operational state;
2. identify prerequisites and blocked actions;
3. identify actions that are executable now;
4. execute automatable actions where authorized;
5. assign human actions to the responsible role when required;
6. evaluate feasible actions by their contribution to successful, safe and quality-compliant mission completion while preserving protected reserve;
7. select the next action or coordinated set of parallel actions;
8. re-evaluate after material state changes.

When several actions are possible, the system shall prefer the combination that advances readiness or mission completion with efficient time/resource use, subject to all mandatory constraints and reserve preservation.

The ranking is a system-internal operational decision; the pilot is not required to select the algorithm used to obtain it.

## 8. Parallel work and role-aware execution

Independent actions may proceed in parallel. Dependent actions remain blocked until their prerequisites are satisfied.

The same preparation algorithm shall support:

- a single person performing all required roles;
- multiple people with distributed responsibilities.

Each role receives only the actions requiring that role, while the orchestrator maintains the common dependency graph and readiness state.

The system shall prefer presenting the human with the currently actionable step rather than exposing a static list of blocked steps.

## 9. Human authority and automation boundary

The system should automate actions that are objectively executable and should expose to the human only actions, confirmations and decisions that cannot be safely or legitimately automated.

The system shall not silently change mandatory configuration or exceed an assigned authority boundary.

Where an action requires human confirmation, BlueSky shall present the relevant reason, consequence, affected mission state and required next step rather than exposing unnecessary low-level technical detail.

## 10. FCS boundary

BlueSky owns operational intent, mission orchestration, supervisory decisions, planning, resource coordination, readiness, C2 orchestration, replanning and lifecycle evidence.

The onboard FCS remains authoritative for real-time stabilization, actuator control and its onboard safety mechanisms within the verified integration contract.

BlueSky shall integrate with ArduPilot/PX4/OEM FCS through adapters and shall not become a second onboard flight controller or a clone of an engineering GCS.

## 11. Contingency policy

Contingency decisions shall preserve the aircraft and collected mission data as primary operational objectives, subject to safety and authorization constraints.

For loss or degradation of C2, the operational decision chain is conceptually:

```text
C2 degradation/loss
        ↓
Can control/link recover safely?
   ├── YES → recover/continue under policy
   └── NO
        ↓
Can the mission continue safely under autonomous capability?
   ├── YES → continue
   └── NO
        ↓
Can approved automatic return be executed?
   ├── YES → return
   └── NO
        ↓
Approved automatic recovery / landing mechanism
```

The exact action is constrained by the verified vehicle/FCS capability, mission authorization and configured contingency policy.

## 12. Multi-UAV failure handling

If an active UAV becomes unavailable, BlueSky shall preserve available data, remove the vehicle from the active resource set, assess the remaining task, redistribute work where feasible, recalculate energy/time/resource constraints and validate the resulting plan before continuation.

If no safe feasible continuation exists, the system shall transition to the applicable human-decision or contingency path.

## 13. Mission completion

Landing alone does not define successful mission completion.

BlueSky shall verify, according to the mission contract, that required outputs were obtained and are acceptable. This may include:

```text
UAV landed
   ↓
data collected?
   ↓
data integrity acceptable?
   ↓
required coverage/task completed?
   ↓
quality requirements satisfied?
   ↓
required outputs available?
   ↓
MISSION COMPLETE
```

The exact checks are mission-type and payload dependent.

## 14. Controlled learning

Operational data shall be accumulated and analysed to identify deviations, causes and candidate corrections. Production behaviour shall not be silently changed from a single observation or by uncontrolled self-modification.

The controlled learning chain is:

```text
flight data
  ↓
actual behaviour
  ↓
deviation analysis
  ↓
cause analysis
  ↓
proposed correction
  ↓
human review
  ↓
approve/reject
  ↓
versioned update
  ↓
future planning/execution
```

Evidence, applicability conditions, version, approval and rollback/provenance shall be retained according to the applicable governance contract.

## 15. Relationship to existing architecture

This baseline extends, rather than replaces, the existing contracts for Mission Model, Mission Objective Profiles, Vehicle/Payload Capability Model, Algorithm Orchestration, Mission Package, C2, Runtime State Machine, Operational Validation and Intelligence/Evolution.

The existing End-to-End Operational Lifecycle already binds objective → planning → validation → authorization → execution → evidence → replay → corrections. The present baseline adds the missing operational orchestration policy across those existing stages rather than creating a parallel lifecycle.

The existing Architecture GAP Matrix defines implementation/verification progression and the current P0 dependency order. This baseline does not change those gates; it adds the operational orchestration layer above them.

## 16. Architectural invariants

The following are mandatory invariants:

- BlueSky is the unified UAV operational system, not an onboard FCS replacement.
- ArduPilot/PX4/OEM FCS are supported execution contours, not mandatory architectural centers.
- Energy/fuel reserve is a protected cross-cutting constraint.
- Safety, regulatory and authorization constraints cannot be traded away for nominal efficiency.
- Mission adaptation is automatic only within an authorized envelope.
- Human authority is explicit and traceable.
- Preparation is dependency-aware and may execute independent actions in parallel.
- The next action is selected for successful, safe, quality-compliant completion with efficient time/resource use and reserve preservation.
- Mission completion requires verified result quality, not merely landing.
- Learning requires evidence and human-controlled promotion.
- UI shall consume these contracts and shall not redefine operational authority or domain rules.
- Existing canonical contracts remain authoritative; new orchestration behaviour composes them and does not duplicate them.

## 17. Implementation consequence

Before further PH5 execution work expands, the repository shall align the Algorithm Orchestrator, Runtime State Machine, Energy Gate, Replanning and FCS integration contracts with this operational orchestration boundary. This alignment is additive: existing contracts remain intact, while missing orchestration relationships are added through explicit interfaces and traceability updates.

Implementation and verification remain separate lifecycle states under the repository GAP rules.
