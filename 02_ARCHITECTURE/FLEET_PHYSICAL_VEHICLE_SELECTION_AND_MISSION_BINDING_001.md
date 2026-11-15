# BlueSky PRO — Physical Fleet Discovery, UAV Recommendation and Mission Binding 001

**Status:** CONTROLLED IMPLEMENTATION CONTRACT  
**Scope:** Fleet discovery, physical vehicle identity, mission suitability recommendation and lifecycle identity continuity  
**Related baselines:**  
- `02_ARCHITECTURE/BLUESKY_END_TO_END_OPERATIONAL_LIFECYCLE_BASELINE.md`
- `02_SYSTEM_DESIGN/ADMINISTRATOR/UAV_AUTOMATIC_CONFIGURATION_ENGINE_001.md`
- `02_SYSTEM_DESIGN/ADMINISTRATOR/UAV_CONFIGURATION_SELECTION_ARCHITECTURE_001.md`
- `03_FLEET/BLUESKY_FLEET_VEHICLE_EQUIPMENT_IMPLEMENTATION_SPEC.md`
- `03_FLEET/BLUESKY_UNIVERSAL_ADAPTER_CONTRACT_001.md`
- `04_SOFTWARE/OPERATIONS/core/device_identity_runtime.hpp`
- `04_SOFTWARE/OPERATIONS/core/device_capability_runtime.hpp`
- `04_SOFTWARE/OPERATIONS/core/fleet_addressing_runtime.hpp`

## 1. Purpose

Define one controlled path from real connected fleet resources to an explainable UAV recommendation, operator-confirmed assignment and immutable mission binding. This contract closes the ambiguity between a UAV model in the catalogue, a demonstration fixture and an actual physical vehicle.

The system recommends the most suitable **available physical vehicle** for a particular mission. It does not silently assign a catalogue model, demo fixture or substitute aircraft.

## 2. Identity levels — never interchangeable

| Identity | Meaning | Operational rule |
|---|---|---|
| `vehicleId` | Stable BlueSky identity of one physical airframe | Required for a physical mission assignment |
| `serialNumber` / manufacturer identity | External identity of that airframe | Must be read or verified from an authoritative source |
| `referenceModelId` | Exact catalogue model/variant | Must be explicitly resolved from verified physical identity |
| `referenceTypeId` | Controlled UAV type | Derived from the resolved catalogue model; not independently guessed |
| `configurationId` + revision | Exact installed vehicle/equipment configuration | Must be read/verified and revision-pinned |
| `performanceProfileId` + revision | Approved performance/safety model for this exact applicable configuration | Mandatory for operational safety/performance evaluation |
| Adapter/device/session IDs | Integration and live-session identity | Correlated with the physical vehicle; never substitutes for `vehicleId` |

A model catalogue entry describes a model. It is not evidence that a physical aircraft exists, is connected, is airworthy, or has a particular installed configuration.

## 3. Discovery and admission sequence

The production path shall be:

```
REAL TRANSPORT / DEVICE DISCOVERY
        ↓
ADAPTER DISCOVER + IDENTIFY
        ↓
PHYSICAL VEHICLE IDENTITY RESOLUTION
        ↓
LIVE CONNECTION / TELEMETRY FRESHNESS
        ↓
PROFILE + CONFIGURATION + EQUIPMENT READ-BACK
        ↓
CAPABILITY / HEALTH / MAINTENANCE / C2 STATE
        ↓
MODEL + TYPE CATALOGUE RESOLUTION
        ↓
CONFIGURATION COMPATIBILITY + APPROVAL CHECK
        ↓
MISSION-SPECIFIC HARD CONSTRAINT FILTER
        ↓
FEASIBLE PHYSICAL VEHICLE CANDIDATES
        ↓
MISSION SUITABILITY RANKING
        ↓
EXPLAINABLE RECOMMENDATION(S)
        ↓
OPERATOR CONFIRMATION
        ↓
MISSION VEHICLE / CONFIGURATION SNAPSHOT PINNED
```

Each adapter response must retain source, timestamp, validity, quality, adapter/version and explicit error state. A transport connection alone does not establish identity, capability verification or operational availability.

## 4. Candidate admission — fail closed

A physical vehicle may enter the operational candidate set only when all applicable mandatory conditions are satisfied:

1. A real adapter/transport has discovered the device; the source is not SIL, replay, demo or a UI fixture.
2. Physical identity is unambiguous and resolves to one stable `vehicleId`.
3. Manufacturer, exact model/variant and serial identity are verified sufficiently for the controlled identity policy.
4. `referenceModelId` resolves to the exact catalogue record and `referenceTypeId` is derived from that record.
5. Live connection and required telemetry/state are current within their configured validity limits.
6. Configuration and equipment have been read back or verified against controlled records; exact configuration revision is known.
7. Required capabilities are both supported and verified; unsupported, degraded or unknown safety-critical capability is not promoted to verified.
8. Configuration compatibility and approval state satisfy the mission's operational policy.
9. Maintenance, health, C2, navigation and other required operational states satisfy their gates.
10. An applicable approved performance profile and safety evidence exist for the exact vehicle/configuration relationship.
11. Mission-specific hard constraints pass, including task capability, equipment/sensor, mass, geometry, environmental, route, energy/reserve and regulatory constraints as applicable.

If a mandatory input is missing, stale, ambiguous, unapproved or unverifiable, the candidate is excluded from operational ranking and the blocking reason is retained. The system must not fill gaps from a model catalogue, a demo record, a typical class value or an unverified manufacturer headline.

## 5. Recommendation is not assignment

The recommendation service shall reuse the existing controlled compatibility and automatic configuration selection logic. It shall not create a parallel compatibility validator.

Processing order:

1. Build the candidate set from admitted, real, currently available physical vehicles.
2. Apply mission hard constraints. Any failure excludes the candidate; a high score cannot compensate.
3. Evaluate mission-specific performance, equipment/sensor suitability, data quality/GSD, route feasibility, time, energy and protected reserve using approved models and current environment evidence.
4. Rank only the feasible candidates using deterministic scoring for identical input and revision snapshots.
5. Return one or more recommendations with score/factors, source revisions, passed constraints and exclusion reasons for alternatives.
6. If no physical vehicle passes, return `NO_ELIGIBLE_CONNECTED_VEHICLE` (with structured blockers). Do not fall back to a catalogue model or fixture.
7. Present the recommendation to the operator. Assignment occurs only after an explicit operator confirmation (or a separately authorized workflow that records the responsible authority and policy).

A recommendation is advisory decision support. It is not flight authorization, readiness approval or release.

## 6. Mission assignment snapshot and identity lock

On operator confirmation, persist and bind at minimum:

- mission ID and mission version;
- physical `vehicleId` and verified serial/manufacturer identity;
- `referenceModelId`, derived `referenceTypeId`, and catalogue revision;
- `configurationId` and immutable configuration revision/snapshot;
- equipment identities and revisions;
- approved `performanceProfileId` and revision, including evidence reference;
- adapter identity/version, device identity and connection/session reference;
- recommendation ID, candidate-set snapshot, ranking inputs and decision rationale;
- operator/actor identity, confirmation timestamp and event reference.

The pinned vehicle/configuration is used consistently by route safety, energy, weather, mission package compilation, readiness, authorization, release, upload, read-back, execution and post-flight evidence.

The same physical vehicle and configuration revision must remain bound throughout one end-to-end lifecycle test. No fixture or model-level record may replace it at any stage.

## 7. Change, loss and reassignment rules

- A transient disconnect or stale telemetry marks the vehicle/session degraded or unavailable; it does not silently replace the mission vehicle.
- A change to physical identity, model binding, configuration revision, equipment, firmware, approved performance profile or material adapter baseline invalidates dependent evidence according to its dependency lineage.
- Before release, reassignment requires a new explicit operator decision, a new assignment snapshot and re-execution of all affected validations and calculations.
- After release or during execution, a vehicle change is a controlled operational contingency/replanning event. It must follow the applicable authority and safety policy; it is never a transparent substitution.
- Historical assignment snapshots and lifecycle events remain immutable and auditable.

## 8. SIL, replay, demo and physical evidence separation

Every resource/source shall carry an explicit provenance class, at minimum:

- `PHYSICAL_LIVE`
- `SIL_FIXTURE`
- `REPLAY`
- `DEMO_FIXTURE`
- `CATALOGUE_REFERENCE`

Only `PHYSICAL_LIVE` with the complete admission evidence may be represented as a connected physical aircraft for a real-hardware lifecycle test. SIL and replay remain valid for software verification but cannot satisfy physical discovery, physical identity or real-flight evidence criteria.

The HMI must visibly distinguish these provenance classes. A demo row must never be presented as a connected physical vehicle.

## 9. Mavic 3 Enterprise reference scenario

The initial reference model for the next lifecycle integration scenario is DJI Mavic 3 Enterprise:

- catalogue model: `DJI-MAVIC-3E`;
- catalogue type: `UAV-TYPE-001`;
- a real physical M3E instance must be discovered and identified before it can be selected for a physical lifecycle run;
- its actual `vehicleId`, serial identity, installed configuration and revision must be retained end-to-end;
- published/reference catalogue values do not create an approved performance profile;
- unknown safety-critical dynamic/performance parameters (including unverified lateral acceleration/turn performance) remain `UNKNOWN` and block any calculation or READY gate that depends on them.

If no physical M3E is connected, the system reports that the physical M3E scenario cannot currently be run. It may recommend another eligible connected physical aircraft for a different mission, but it must not claim that this validates the M3E scenario.

## 10. Required HMI behavior

The fleet/mission interface shall distinguish:

- discovered devices;
- verified physical vehicles;
- connected and fresh vehicles;
- vehicles requiring identification/configuration/approval action;
- admitted mission candidates;
- excluded candidates and their controlled blocking reasons;
- ranked recommendations and their rationale;
- operator-confirmed mission assignment;
- pinned vehicle/configuration identity for the active mission.

The interface shall not expose demo fixtures as physical fleet availability. The operator sees only valid recommendation choices for assignment; diagnostic views may show excluded resources with explicit status/reason.

## 11. Implementation boundary and reuse

Reuse the existing:
- `DeviceIdentityRuntime`;
- `DeviceCapabilityRuntime`;
- `FleetAddressingRuntime`;
- `CapabilityCompatibilityGate`;
- universal adapter contract and registry;
- UAV model/type catalogue;
- automatic configuration/compatibility engine;
- mission lifecycle evidence and validation coordinator.

These in-memory runtimes and SIL fixtures are not, by themselves, a production discovery provider, persistent fleet store or real transport integration. Implementation must connect them to a real provider and persistence boundary. Do not create duplicate identity or capability registries.

Where production transport or persistence is not implemented, expose the missing interface as a blocker and keep the physical candidate set empty. Do not simulate success.

## 12. Acceptance criteria

1. A SIL/demo/catalogue record cannot enter the physical candidate set.
2. One discovered physical device resolves to exactly one stable `vehicleId`; identity collisions are rejected.
3. Exact model/variant and type mapping is traceable to catalogue revision and verified physical identity.
4. Stale, missing or ambiguous connection/configuration/capability/health data excludes the candidate with a reason.
5. Unknown or unapproved safety-critical configuration/performance inputs block operational admission.
6. Compatibility and hard constraints execute before ranking; score cannot override a failed gate.
7. Recommendation is deterministic for identical controlled inputs and returns traceable reasons/revisions.
8. No eligible candidate produces `NO_ELIGIBLE_CONNECTED_VEHICLE`, never a fixture fallback.
9. Operator confirmation creates a durable assignment snapshot.
10. Vehicle identity and configuration revision remain pinned across the mission lifecycle.
11. Reassignment/change invalidates all dependent evidence and requires revalidation.
12. Physical M3E lifecycle verification is reported only when a real M3E and its required approved evidence are actually present.
13. SIL, replay, demo and physical results remain distinguishable in UI, logs, tests and exported evidence.

## 13. Definition of done

The feature is complete only when a real connected physical vehicle can be discovered through a production adapter, identified, resolved to its exact model/configuration, admitted by the common compatibility and safety gates, ranked for a concrete mission, explicitly confirmed by an operator, pinned through the complete mission lifecycle, and represented by a traceable evidence package. Until then, the architecture contract may be implemented and SIL-tested, but physical fleet recommendation and physical lifecycle validation remain **NOT VERIFIED**.
