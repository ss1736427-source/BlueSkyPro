# BlueSky PRO — Linked Interface and Data Flow Architecture

**Status:** ARCHITECTURE BASELINE  
**Date:** 2026-10-04

## 1. Principle

All functional data flows in BlueSky PRO MUST pass through explicit, linked interfaces.

A visual control, algorithm module, sensor, external service or storage component MUST NOT create an undocumented point-to-point dependency on another component.

The interface is the contract for data ownership, commands, queries, state changes, events, validation results, errors, versioning and traceability.

This is consistent with contract-based architecture: interfaces define what one component provides and what another component may rely on. citeturn0academia13

## 2. Canonical data ownership

The Planning Kernel owns the authoritative planning state.

The principal shared domain objects are:

- Mission Profile;
- Task Model;
- Environment Model;
- UAV State;
- Constraint Set;
- Route Model;
- 4D Trajectory;
- Optimization Result;
- Safety Verification Result;
- Mission Release State.

UI views are projections/editors of these domain objects. They are not independent sources of truth.

## 3. Linked interface chain

The core flow is:

OPERATOR UI → Mission Interface → Task Model Interface → Planning Kernel Interface → Constraint Interface → Route Interface → Energy/Performance Interface → Trajectory Interface → Conflict Interface → Optimization Interface → Safety Verification Interface → Mission Release Interface → UAV Integration Interface

Feedback/state flow is bidirectional:

UI ←→ interfaces ←→ authoritative domain state

The same principle applies to Map, Profile, 3D View, Telemetry and UAV Integration.

## 4. Map / Profile / 3D linkage

Map, flight profile and 3D trajectory are linked views of the same trajectory/task state.

The operator may edit a route element in any authorized view.

Example:

MAP: move mandatory point → Route Edit Interface → Task Model update → validation → route recalculation → 3D trajectory update → profile update → safety status update

The profile must never contain a second independent copy of the route.

Canonical relationship:

MAP VIEW ↔ ROUTE INTERFACE ↔ TASK MODEL ↔ TRAJECTORY MODEL ↔ PROFILE VIEW

## 5. Mandatory operator points

Mandatory passage points are represented by a common domain object and interface.

Required information includes:

- point ID;
- geographic position;
- altitude / altitude band;
- sequence/order;
- tolerance/corridor;
- point type;
- dwell/observation requirements;
- source = operator;
- constraint priority.

Every view references the same point ID.

Moving the point produces a route/task change event and invalidates affected derived calculations.

## 6. Interface classes

### Command interfaces

Used when an actor requests a state-changing operation.

Examples: CreateMission, SetMissionProfile, AddMandatoryPoint, MoveMandatoryPoint, DeleteMandatoryPoint, SetAltitudeConstraint, RecalculateRoute, StartOptimization, ApproveMission, ReleaseMission.

### Query interfaces

Used to obtain current authoritative state.

Examples: GetTaskModel, GetRoute, GetTrajectory, GetSafetyStatus, GetEnergyEstimate, GetOptimizationResult.

### Event interfaces

Used to propagate completed state changes.

Examples: MissionProfileChanged, TaskModelChanged, MandatoryPointChanged, EnvironmentChanged, RouteChanged, TrajectoryChanged, ConstraintViolationDetected, OptimizationCompleted, SafetyVerificationCompleted, MissionReleaseStateChanged.

Event-driven architecture is appropriate for propagation where multiple consumers must react to the same state change, while synchronous interfaces remain appropriate for operations requiring an immediate authoritative response. citeturn0search2turn0search5

## 7. Interface contract

Every interface shall define:

- interface ID;
- producer/owner;
- consumer;
- command/query/event type;
- request schema;
- response schema;
- event schema;
- units and coordinate reference;
- validation rules;
- error model;
- version;
- timing/latency expectation;
- consistency requirement;
- authorization requirement;
- trace/correlation ID.

Interface schemas must be versioned.

## 8. Safety-critical consistency

Asynchronous events MUST NOT be used as a substitute for authoritative safety validation.

For example:

UI → AddMandatoryPoint → Task Model

may emit MandatoryPointChanged, but mission release must synchronously or transactionally obtain the authoritative result of Constraint Validation → Route Validation → Energy Validation → Conflict Validation → Safety Verification.

A stale UI projection can never be treated as READY.

## 9. Data ownership rule

Each important data object has one authoritative owner.

Other components receive a reference/ID, a validated projection, or a contract-defined copy.

They must not silently mutate another component's authoritative state.

This avoids multiple competing sources of truth.

## 10. Interface adapters

External systems must connect through adapters.

Examples: map provider, weather, NOTAM/airspace, terrain/obstacle, UAV telemetry, UAV mission-upload and storage/export adapters.

External formats MUST be converted at the adapter boundary into BlueSky domain contracts.

The Planning Kernel must not contain vendor-specific API formats.

## 11. Data lineage

Every planning result must be traceable to:

- operator input;
- environment snapshot;
- UAV state;
- algorithm/template version;
- optimization policy/version;
- safety verification result.

A route change must therefore be traceable from SOURCE INPUT → INTERFACE → DOMAIN STATE → CALCULATION → RESULT.

## 12. No hidden flows

The following are prohibited:

- direct UI manipulation of planner internals;
- direct Map → Route-engine calls bypassing Task Model;
- direct Profile → UAV commands;
- direct Optimization → UAV commands;
- direct external-provider data injection into safety logic;
- duplicated route state in Map/Profile/3D;
- undocumented shared mutable state.

All such interactions must use the defined interfaces.

## 13. Engineering rule

**No data flow without an interface contract.**

**No authoritative state without an owner.**

**No derived view becomes the source of truth.**

**No safety release without authoritative re-verification.**

## 14. Architectural target

OPERATOR ↕ HMI INTERFACES ↕ MISSION / TASK MODEL ↕ PLANNING KERNEL INTERFACES ↕ CONSTRAINT / ROUTE / ENERGY / TRAJECTORY / CONFLICT ↕ OPTIMIZATION ↕ SAFETY VERIFICATION ↕ MISSION RELEASE ↕ UAV / EXTERNAL SYSTEM ADAPTERS

This architecture allows the HMI, planning algorithms and integrations to evolve independently while preserving one authoritative data model and traceable contracts.
