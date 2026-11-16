# BlueSky PRO — Vehicle / Equipment Capability Model

**Status:** ARCHITECTURE BASELINE — P0 integration contract

## 1. Purpose

The Capability Model is the bridge between the abstract mission and the real fleet. BlueSky shall determine automatically whether a UAV + autopilot + equipment configuration can perform the requested task and which vehicle is the most appropriate.

The pilot does not select an algorithm or manually translate mission requirements into aircraft capabilities.

## 2. Position in the system

```text
MISSION OBJECTIVE
      ↓
REQUIRED CAPABILITIES
      ↓
FLEET CAPABILITY REGISTRY
      ↓
CAPABILITY MATCHING
      ↓
┌───────────────────────────────┐
│ UAV + AUTOPILOT + EQUIPMENT   │
│ + C2 + BATTERY                │
└───────────────────────────────┘
      ↓
FEASIBILITY / RANKING
      ↓
ALGORITHM ORCHESTRATOR
      ↓
MISSION ALLOCATION
```

## 3. Vehicle identity

Each vehicle profile shall have a stable identity and versioned configuration containing, as applicable:

- vehicle identifier;
- vehicle type/model;
- airframe configuration;
- propulsion configuration;
- autopilot type and version;
- supported protocol(s);
- firmware/software compatibility;
- navigation capabilities;
- communication capabilities;
- battery type/capacity/state model;
- equipment interfaces;
- operational limits;
- recovery capabilities;
- maintenance/resource state.

## 4. Flight capability model

The profile shall represent capabilities rather than merely nominal specifications.

Examples:

- minimum/maximum airspeed;
- cruise/economic speed ranges;
- climb/descent limits;
- altitude envelope;
- turning/trajectory limits;
- endurance model;
- usable energy model;
- wind tolerance/model;
- navigation sensor availability;
- GNSS-denied/recovery capability;
- C2 requirements;
- return/recovery constraints;
- environmental operating limits.

Where a parameter is configuration-dependent, the active configuration is authoritative for planning.

## 5. Energy capability

Energy capability shall be represented as a model, not as a single catalogue endurance number.

The model shall permit calculation using:

- battery usable energy;
- battery state/health;
- degradation coefficient;
- equipment mass and power;
- propulsion configuration;
- flight mode;
- wind;
- route geometry;
- environmental conditions;
- reserve requirement;
- recovery/return requirement.

The planner and validator shall consume the same energy model to avoid divergence between predicted feasibility and release validation.

## 6. Propulsion/resource capability

Where applicable, the profile shall include:

- engine/motor type;
- propulsion configuration;
- propeller configuration;
- resource/operating-hour state;
- applicable limits;
- configuration-specific performance data.

Resource constraints are part of candidate evaluation but shall never override mandatory safety or energy constraints.

## 7. Equipment capability

An equipment profile shall identify:

- sensor/camera type;
- equipment identifier and version;
- mass and balance impact;
- electrical power requirements;
- interface/protocol;
- supported modes;
- field of view and relevant optics parameters;
- resolution/GSD-related parameters;
- stabilization/gimbal capability;
- triggering/control capabilities;
- data output requirements;
- operational restrictions.

## 8. Task capability requirements

Mission requirements shall be translated into capability requirements before vehicle allocation.

```text
INSPECTION
 → required sensor
 → required viewpoint
 → required resolution
 → required endurance
 → required clearance

DELIVERY
 → equipment mass/volume
 → range
 → recovery margin
 → ETA

3D
 → imaging geometry
 → overlap
 → viewing angles
 → sensor capability
```

## 9. Matching logic

BlueSky shall distinguish:

**Hard capability requirements** — failure makes the vehicle unsuitable.

**Soft capability preferences** — influence ranking among suitable vehicles.

A vehicle may be rejected before route optimization if it cannot satisfy a mandatory capability.

## 10. Configuration validity

A vehicle is not considered available merely because its airframe profile exists.

The active configuration must be valid for the intended mission, including:

```text
AIRFRAME
 + AUTOPILOT
 + FIRMWARE
 + BATTERY
 + PROPULSION
 + EQUIPMENT
 + C2
 + REQUIRED EQUIPMENT
      ↓
CONFIGURATION VALID
```

Configuration changes shall invalidate affected calculations and trigger revalidation.

## 11. Multi-UAV allocation

For a group mission, BlueSky shall evaluate the fleet as a heterogeneous set.

Different UAVs may receive different subtasks when this produces a better admissible operational solution.

Allocation shall consider:

- capability match;
- energy margin;
- equipment suitability;
- route characteristics;
- C2 availability;
- coordination constraints;
- mission timing;
- resource state.

## 12. Autopilot independence

The capability model shall not embed ArduPilot-, PX4- or OEM-specific assumptions into the mission model.


## 13. Execution-mode-specific capability provenance

Capability profile resolution is mode-aware and fail-closed. The execution mode is an explicit input; a resolver must never infer or silently switch it.

**VIRTUAL**
- A new virtual aircraft may use the selected model/variant's manufacturer-published passport characteristics.
- The profile must pin the manufacturer source, source revision, model/variant, configuration revision and profile revision.
- Parameters calculated from a simulation assumption must be individually marked `DerivedSimulationEstimate`, with a traceable method/source revision. They are not manufacturer facts and must not be marked verified.
- Every parameter required by the current task and its validators must have admissible evidence. Missing values are blockers; generic or typical values must not be inserted implicitly.
- The resolved execution provenance is `SIL_FIXTURE` and the release target is `SIMULATOR_ONLY`.

**PHYSICAL**
- Only the exact identified airframe and its verified installed configuration may supply the operational capability profile.
- Required parameter evidence must be tied to that physical configuration and explicitly verified. Manufacturer passport data may describe the model but cannot satisfy physical operational validation.
- Missing identity, configuration, battery/energy state, equipment verification or required capability evidence blocks the relevant admission/validation.
- The resolved execution provenance is `PHYSICAL_LIVE` and the release target is `PHYSICAL_UAV`.

A VIRTUAL-to-PHYSICAL mode change invalidates the virtual profile resolution and all dependent calculations/attestations. The physical profile must be resolved again from the identified aircraft; no SIL result is promoted to physical evidence.

Published manufacturer maxima are reference facts, not automatically safe operating limits. The planning/validation chain must use task-applicable operational limits and explicit conservative simulation assumptions. A profile resolver establishes source admissibility; it does not replace flight-envelope, energy-reserve, equipment, task-quality or lifecycle validation.
