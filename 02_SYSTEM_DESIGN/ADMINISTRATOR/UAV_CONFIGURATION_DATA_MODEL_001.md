# UAV CONFIGURATION DATA MODEL 001

## 1. Scope

This document defines the controlled data model for UAV models, equipment, compatibility and configurations used by the Administrator and consumed by Flight Planning.

## 2. Core entities

```text
UAV_MODEL
   │
   ├── UAV_PERFORMANCE
   ├── UAV_LIMITS
   └── COMPATIBILITY_RULES
            │
            ├── BATTERY
            ├── PAYLOAD
            ├── PROPULSION
            ├── COMMUNICATION
            └── NAVIGATION
                     │
                     ↓
               CONFIGURATION
                     │
                     ↓
              ENERGY / NAVIGATION
```

## 3. UAV_MODEL

Required identity fields:

- `uav_model_id`
- manufacturer
- model
- variant
- aircraft category/type
- source documentation reference
- source revision/date
- data status
- record revision

Technical fields are stored separately from identity and provenance metadata.

## 4. EQUIPMENT records

Each equipment class shall have a stable identifier and revision:

- `battery_id`
- `payload_id`
- `propulsion_id`
- `communication_id`
- `navigation_id`
- interface/mount identifiers where applicable

Each record contains manufacturer-declared parameters, source/provenance, applicability and controlled status.

## 5. Compatibility relationship

Compatibility is represented explicitly and shall not be inferred only from component names.

Conceptual relation:

```text
COMPATIBILITY
├── subject_type
├── subject_id
├── object_type
├── object_id
├── compatibility_state
├── constraints
├── source_reference
├── revision
└── status
```

`compatibility_state` shall distinguish at minimum:

- COMPATIBLE
- INCOMPATIBLE
- CONDITIONAL
- UNKNOWN

`UNKNOWN` shall not be treated as compatible for automatic operational selection.

## 6. Configuration

A configuration is an approved concrete assembly of compatible components.

```text
CONFIGURATION
├── configuration_id
├── configuration_revision
├── uav_model_id
├── battery set
├── payload set
├── propulsion set
├── communication set
├── navigation set
├── derived mass
├── derived power inputs
├── derived energy inputs
├── operational limits
├── source/revision links
└── lifecycle status
```

The configuration record references component records rather than duplicating their authoritative data.

## 7. Configuration validity

A configuration is valid only when all required component relationships and constraints are satisfied.

Conceptual predicate:

```text
VALID_CONFIGURATION =
    approved(UAV)
AND approved(all required components)
AND compatible(all required relationships)
AND satisfied(all applicable constraints)
AND valid(source/revision chain)
```

A configuration with an unresolved mandatory compatibility relationship shall not become an operational configuration.

## 8. Automatic configuration input

Flight Planning shall submit mission requirements, not arbitrary hardware combinations.

Example requirement set:

```text
MISSION_REQUIREMENTS
├── mission_type
├── area / geometry
├── altitude
├── required accuracy / GSD
├── required sensor capability
├── endurance requirement
├── range requirement
├── environmental conditions
├── payload requirements
└── operational constraints
```

## 9. Selection pipeline

```text
MISSION_REQUIREMENTS
        ↓
ELIGIBLE UAV MODELS
        ↓
COMPATIBILITY FILTER
        ↓
VALID CONFIGURATIONS
        ↓
PERFORMANCE / OPERATIONAL FILTER
        ↓
ENERGY MODEL
        ↓
RANKING / OPTIMIZATION
        ↓
RECOMMENDED CONFIGURATION
```

The optimizer is prohibited from creating combinations outside the valid configuration set.

## 10. Operator-facing filtering

When a component selector is displayed, its data source shall already be filtered by the active UAV, current configuration state and applicable mission constraints.

Therefore:

```text
INCOMPATIBLE → NOT RETURNED → NOT DISPLAYED → NOT SELECTABLE
```

The UI shall not rely on visual disabling alone as the compatibility control.

## 11. Conditional compatibility

A `CONDITIONAL` relationship may be exposed only when its conditions are machine-checkable and satisfied by the current configuration/mission context.

Otherwise it shall be excluded from operator selection.

## 12. No-result handling

If no valid configuration remains, the system shall return a controlled no-solution result with the blocking constraint(s). It shall not offer an incompatible component as a workaround.

## 13. Energy integration

The configuration object is the boundary between Administrator data and the Energy Model.

The Energy Model receives the resolved configuration and its controlled inputs, including applicable aircraft, battery, payload and operating parameters. Manufacturer-declared and derived/modelled values remain distinguishable.

## 14. Lifecycle

```text
DRAFT
  ↓
VALIDATED
  ↓
APPROVED
  ↓
ACTIVE
  ↓
SUSPENDED / OBSOLETE
```

Only configurations permitted by lifecycle and operational rules may be used for mission planning.

## 15. Initial reference population

The model is intended to be populated initially from the controlled official-TTX reference dataset and subsequent qualified sources. Existing reference platforms shall be imported as records rather than hard-coded into application logic.

## 16. Design rule

The Administrator defines the controlled universe of possible components and permitted relationships. The configuration engine constructs and filters valid combinations. Flight Planning consumes only the resulting valid set.

No operational UI control shall bypass this chain.