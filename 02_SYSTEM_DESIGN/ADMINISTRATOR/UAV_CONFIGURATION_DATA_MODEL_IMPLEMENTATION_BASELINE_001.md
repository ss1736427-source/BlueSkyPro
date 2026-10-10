# UAV CONFIGURATION DATA MODEL IMPLEMENTATION BASELINE 001

## 1. Purpose

Define the implementation-oriented data model for the BlueSky PRO Administrator block and its UAV automatic configuration engine.

The model separates aircraft, components, compatibility rules, configuration instances and mission requirements so that the UI never has to infer technical compatibility.

## 2. Entity model

```text
UAV_MODEL
   │
   ├── COMPATIBILITY_RULE ── COMPONENT
   │                              │
   │                              └── COMPONENT_SPEC
   │
   └── CONFIGURATION
             │
             ├── CONFIGURATION_COMPONENT
             │
             ├── CONFIGURATION_VALIDATION
             │
             └── APPROVAL_STATE

MISSION_REQUIREMENT
        │
        ↓
CONFIGURATION_SELECTOR
        │
        ↓
CONFIGURATION
```

## 3. UAV_MODEL

Required logical fields:

```text
uav_model_id
manufacturer
model_name
model_variant
category
source_reference
source_revision
mtow
empty_mass
payload_capacity
battery_architecture
power_architecture
interface_set
performance_profile_id
energy_model_id
status
verification_state
```

The record represents the aircraft platform, not an individual physical airframe.

## 4. COMPONENT

All configurable equipment shall use a common component registry.

```text
component_id
component_type
manufacturer
model_name
variant
mass
power_input
power_output
voltage_range
current_limit
interface_set
mounting_requirements
supported_uav_models
source_reference
source_revision
status
verification_state
```

`component_type` shall support at minimum:

- BATTERY;
- PAYLOAD;
- CAMERA;
- SENSOR;
- COMMUNICATION;
- NAVIGATION;
- PROPULSION;
- MOUNT;
- ACCESSORY;
- OTHER.

## 5. BATTERY

Battery is a specialized component with additional fields:

```text
battery_id
component_id
nominal_voltage
energy_wh
capacity_ah
maximum_current
mass
chemistry
allowed_quantity
allowed_topology
charging_constraints
source_reference
verification_state
```

A battery set shall be represented separately from a single battery item.

```text
BATTERY_SET
 ├── battery_id
 ├── quantity
 ├── topology
 └── derived_energy_model_id
```

The Energy Model shall calculate usable energy from the controlled battery-set definition. Quantity multiplication alone shall not establish usable energy.

## 6. PAYLOAD

```text
payload_id
component_id
payload_class
mass
power_demand
interface_set
mounting_position
operating_constraints
source_reference
verification_state
```

A payload may be integrated into the UAV model or represented as an external component. The distinction shall be explicit in the data record.

## 7. COMPATIBILITY_RULE

```text
rule_id
source_entity_type
source_entity_id
target_entity_type
target_entity_id
relation
condition_expression
constraint_set
priority
source_reference
source_revision
verification_state
effective_from
effective_to
```

Relations:

```text
COMPATIBLE
CONDITIONAL
INCOMPATIBLE
UNKNOWN
```

The rule itself is data. The engine interprets the rule; the UI does not duplicate it.

## 8. Configuration

A configuration is a concrete selected set for one UAV model.

```text
configuration_id
uav_model_id
configuration_type
name
revision
components[]
battery_sets[]
configuration_constraints
validation_state
approval_state
source_basis
created_by
created_at
updated_at
```

`configuration_type`:

- STANDARD;
- CUSTOM;
- DERIVED.

## 9. CONFIGURATION_COMPONENT

```text
configuration_id
component_id
quantity
installation_point
interface_assignment
role
parameter_overrides
```

No component shall be inserted into a standard configuration unless its compatibility relationship is established.

## 10. Validation result

Each configuration evaluation shall produce structured results.

```text
validation_id
configuration_id
rule_id
check_type
result
severity
message
measured_value
limit_value
source_reference
engine_version
timestamp
```

Result classes:

```text
PASS
CONDITIONAL
BLOCK
UNKNOWN
```

A `BLOCK` from a hard constraint prevents operational selection.

## 11. Automatic configuration selection

The selector receives mission requirements rather than a preselected component list.

```text
MISSION_REQUIREMENT
      ↓
UAV candidate set
      ↓
compatible component set
      ↓
configuration generation
      ↓
configuration validation
      ↓
Energy Model
      ↓
performance evaluation
      ↓
ranking
      ↓
recommended configuration
```

The selector shall generate candidates only from components that survive the compatibility filter.

## 12. Mission requirement interface

The selector shall be able to consume, where applicable:

```text
mission_type
area_or_route
required_payload
required_sensor
required_resolution
required_flight_time
required_range
required_speed
arrival_constraint
weather_constraints
wind_conditions
reserve_policy
communication_requirements
navigation_requirements
operational_constraints
```

## 13. Administrator UI behavior

The Administrator shall provide two modes.

### 13.1 Standard configuration

```text
Select UAV
   ↓
Select mission equipment
   ↓
show only compatible components
   ↓
validate complete configuration
   ↓
save/approve
```

Known incompatible items shall be hidden from the selectable list.

### 13.2 Custom configuration

```text
Select CUSTOM UAV
   ↓
manual component selection
   ↓
allow non-standard combinations
   ↓
run compatibility + engineering validation
   ↓
VALID / CONDITIONAL / UNKNOWN / INVALID
```

CUSTOM must not silently convert an unknown relation into an approved relation.

## 14. Component filtering contract

For each UI selection context the engine shall expose:

```text
available[]
conditional[]
hidden_incompatible[]
unknown[]
```

Normal operational selection uses `available[]` only.

`hidden_incompatible[]` is retained internally for audit/diagnostic purposes and is not shown as an ordinary selectable option.

`unknown[]` is not eligible for automatic operational selection.

## 15. Automatic configuration ranking

Ranking shall occur only after hard constraints pass.

Suggested deterministic order:

1. mission capability;
2. safety/operational compliance;
3. energy feasibility;
4. return feasibility;
5. required payload capability;
6. endurance/range margin;
7. weather margin;
8. mass margin;
9. resource/availability constraints;
10. efficiency objective.

The ranking algorithm shall never trade a hard compatibility failure for a better score.

## 16. Configuration lifecycle

```text
DRAFT
  ↓
VALIDATING
  ↓
VALIDATED
  ↓
APPROVAL_PENDING
  ↓
APPROVED
  ↓
SUSPENDED / OBSOLETE
```

CUSTOM configurations may remain `DRAFT`, `VALIDATING` or `VALIDATED` until the required approval basis exists.

## 17. Versioning

Changes to a UAV model, component, compatibility rule, energy parameter or configuration shall create a traceable revision.

A previously approved configuration shall not be silently changed by editing a referenced component record.

The system shall retain the exact data revision used for an operational decision.

## 18. Source and verification control

Each technically significant value shall retain:

```text
source_reference
source_revision
verification_state
qualification_method
```

Manufacturer-published values shall remain distinguishable from derived engineering values and operator-entered CUSTOM values.

## 19. Example: standard dual-battery configuration

```text
UAV_MODEL = M350 RTK
CONFIGURATION_TYPE = STANDARD
BATTERY_SET = TB65 × 2
PAYLOAD = approved payload

Compatibility Engine
    ↓
all relations confirmed
    ↓
Configuration validation
    ↓
Energy Model
    ↓
eligible configuration
```

## 20. Example: CUSTOM dual-battery configuration

```text
UAV_MODEL = CUSTOM
BATTERY_SET = Battery-A × 2
PAYLOAD = Non-standard Payload-X
MOUNT = Custom Mount-Y

Compatibility Engine
    ↓
manual configuration accepted for analysis
    ↓
engineering checks
    ↓
UNKNOWN / CONDITIONAL / BLOCK as applicable
    ↓
no operational approval until required validation is complete
```

## 21. Data integrity rules

1. Component IDs shall be unique.
2. UAV model IDs shall be unique by manufacturer/model/variant identity.
3. A compatibility rule shall reference existing entities.
4. An expired or suspended rule shall not be used as an active compatibility basis.
5. `UNKNOWN` shall not be promoted automatically.
6. A configuration shall identify its exact component quantities.
7. Battery topology shall be explicit when more than one battery is used.
8. Configuration validation shall be reproducible from stored revisions.
9. Automatic selection shall use only eligible configurations.
10. Operational approval shall remain distinct from technical compatibility.

## 22. Implementation principle

The Administrator database is the controlled source for configuration data. The Compatibility Engine is the decision layer. The automatic selector is the optimization layer. The Flight Planning UI is the presentation and operator interaction layer.

```text
ADMIN DATA
    ↓
RULE ENGINE
    ↓
VALID CONFIGURATIONS
    ↓
AUTOMATIC SELECTOR
    ↓
OPERATOR UI
    ↓
FLIGHT EXECUTION
```

This baseline is intended to become the implementation contract for the Administrator data store, compatibility service and configuration-selection service.