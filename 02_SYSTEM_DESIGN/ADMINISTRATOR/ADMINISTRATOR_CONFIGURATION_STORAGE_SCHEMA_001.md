# ADMINISTRATOR CONFIGURATION STORAGE SCHEMA 001

## 1. Purpose

Define the persistence model for the Administrator block so the BlueSky PRO configuration engine can store UAV models, components, compatibility rules, configurations, validation results and approval state without duplicating technical logic in the UI.

## 2. Logical storage model

```text
UAV_MODEL
   │
   ├── UAV_INTERFACE
   ├── UAV_LIMIT
   └── UAV_MODEL_SOURCE

COMPONENT
   ├── COMPONENT_SPEC
   ├── COMPONENT_INTERFACE
   └── COMPONENT_SOURCE

COMPATIBILITY_RULE
   ├── RULE_CONDITION
   └── RULE_SOURCE

CONFIGURATION
   ├── CONFIGURATION_COMPONENT
   ├── CONFIGURATION_BATTERY_SET
   ├── CONFIGURATION_VALIDATION
   └── CONFIGURATION_APPROVAL

MISSION_REQUIREMENT
   ↓
CONFIGURATION_SELECTOR
```

## 3. Storage principles

1. Master catalogue records are immutable by revision.
2. A configuration references exact component revisions.
3. Compatibility rules reference exact entity revisions where technically significant.
4. Source provenance is stored with every controlled technical value.
5. Validation results are persisted and reproducible.
6. Operational approval is separate from technical compatibility.
7. CUSTOM configurations may reference operator-entered values, which remain explicitly classified as user-provided.

## 4. UAV_MODEL

```text
uav_model
-----------
id
manufacturer
model_name
model_variant
category
status
verification_state
source_id
source_revision
mtow_kg
empty_mass_kg
payload_capacity_kg
battery_architecture_id
power_architecture_id
performance_profile_id
energy_model_id
created_at
updated_at
```

`id` is the stable logical identity. Revision-controlled technical records shall not be overwritten in place.

## 5. COMPONENT

```text
component
-----------
id
component_type
manufacturer
model_name
variant
status
verification_state
source_id
source_revision
mass_kg
power_input_w
power_output_w
voltage_min_v
voltage_max_v
current_limit_a
created_at
updated_at
```

`component_type` minimum controlled vocabulary:

```text
BATTERY
PAYLOAD
CAMERA
SENSOR
COMMUNICATION
NAVIGATION
PROPULSION
MOUNT
ACCESSORY
OTHER
```

## 6. COMPONENT_SPEC

Variable technical properties shall not be forced into the common component table.

```text
component_spec
-----------
id
component_id
revision
parameter_code
value_numeric
value_text
unit
source_id
verification_state
effective_from
effective_to
```

This permits new equipment types to be added without changing the base schema.

## 7. INTERFACES

Physical and logical interfaces are normalized so compatibility can be evaluated rather than inferred by the UI.

```text
interface_definition
-----------
id
interface_type
standard_or_proprietary
name
version
attributes
```

```text
entity_interface
-----------
entity_type
entity_id
interface_id
role
position
quantity
revision
```

Example interface types:

```text
MECHANICAL_MOUNT
ELECTRICAL_POWER
DATA
CONTROL
COMMUNICATION
NAVIGATION
BATTERY_SLOT
```

## 8. COMPATIBILITY_RULE

```text
compatibility_rule
-----------
id
revision
source_entity_type
source_entity_id
target_entity_type
target_entity_id
relation
priority
status
verification_state
source_id
source_revision
effective_from
effective_to
condition_expression
constraint_expression
reason_code
```

Allowed relation values:

```text
COMPATIBLE
CONDITIONAL
INCOMPATIBLE
UNKNOWN
```

## 9. Rule conditions

Conditions shall be stored separately when they require structured evaluation.

```text
rule_condition
-----------
id
rule_id
parameter_code
operator
value_numeric
value_text
unit
logical_group
sequence
```

Examples:

```text
payload_mass <= remaining_payload_capacity
voltage within allowed_range
battery_quantity <= allowed_quantity
mission_temperature within operating_range
```

The executable representation may later be implemented as a rules DSL or expression engine, but the persisted meaning shall remain deterministic.

## 10. CONFIGURATION

```text
configuration
-----------
id
uav_model_id
configuration_type
name
revision
status
validation_state
approval_state
source_basis
created_by
created_at
updated_at
```

Configuration types:

```text
STANDARD
CUSTOM
DERIVED
```

## 11. CONFIGURATION_COMPONENT

```text
configuration_component
-----------
configuration_id
component_id
component_revision
quantity
installation_point
interface_assignment
role
parameter_overrides
```

A quantity greater than one is allowed only when the applicable compatibility and configuration rules permit it.

## 12. BATTERY_SET

Battery sets require explicit topology.

```text
configuration_battery_set
-----------
configuration_id
battery_component_id
battery_revision
quantity
topology
slot_assignment
connection_definition
usable_energy_wh
derivation_method
verification_state
```

`usable_energy_wh` shall not be populated by simple multiplication unless the controlled Energy Model explicitly permits that derivation.

For a CUSTOM configuration, the topology may be entered manually, but its verification state shall remain visible.

## 13. CONFIGURATION_VALIDATION

```text
configuration_validation
-----------
id
configuration_id
engine_version
validation_run_id
check_type
result
severity
rule_id
measured_value
limit_value
unit
message
source_basis
created_at
```

Result values:

```text
PASS
CONDITIONAL
BLOCK
UNKNOWN
```

Severity should distinguish at minimum:

```text
INFO
WARNING
ERROR
CRITICAL
```

## 14. CONFIGURATION_APPROVAL

```text
configuration_approval
-----------
id
configuration_id
approval_state
approved_by
approval_basis
approval_revision
approved_at
suspended_at
suspension_reason
```

Approval states:

```text
DRAFT
VALIDATING
VALIDATED
APPROVAL_PENDING
APPROVED
SUSPENDED
OBSOLETE
```

A technically valid configuration is not automatically an operationally approved configuration.

## 15. SOURCE_PROVENANCE

```text
source
-----------
id
source_type
publisher
title
reference_identifier
publication_date
revision
location
retrieved_at
hash
notes
```

Source types shall distinguish manufacturer documentation, controlled internal engineering data, verified derived data and operator-provided CUSTOM data.

## 16. Automatic selection view

The selector should consume a derived read model rather than query raw tables independently.

Conceptual view:

```text
ELIGIBLE_CONFIGURATION
----------------------
configuration_id
uav_model_id
configuration_revision
mission_capabilities
payload_capability
energy_feasibility
return_feasibility
performance_margin
weather_margin
mass_margin
availability_state
approval_state
```

Only configurations satisfying all mandatory hard constraints shall enter this view.

## 17. UI filtering contract

The Administrator and Flight Planning UI shall request selectable components using the current context:

```text
GET AVAILABLE COMPONENTS
    context = UAV + configuration + mission
              ↓
       Compatibility Engine
              ↓
 available / conditional / hidden / unknown
```

The UI shall display only `available` items in normal selection mode.

Known incompatible items remain queryable for diagnostics/audit but are not ordinary selectable options.

## 18. CUSTOM data handling

CUSTOM records shall preserve the distinction between:

```text
MANUFACTURER_CONTROLLED
ENGINEERING_DERIVED
USER_ENTERED
UNKNOWN
```

User-entered values must never overwrite manufacturer-controlled values.

A CUSTOM configuration can therefore contain:

```text
Battery-A × 2
Non-standard Payload-X
Custom Mount-Y
Additional Communication
Additional Navigation
```

and retain validation outcomes for every affected rule.

## 19. Revision integrity

When a referenced component receives a new revision:

```text
old configuration
      ↓
continues to reference old component revision

new configuration revision
      ↓
may reference new component revision
```

An approved configuration shall therefore remain reproducible against its original data basis.

## 20. Minimum indexes

Implementation should index at least:

```text
UAV_MODEL(manufacturer, model_name, model_variant)
COMPONENT(component_type, manufacturer, model_name, variant)
COMPATIBILITY_RULE(source_entity_type, source_entity_id, target_entity_type, target_entity_id, status)
CONFIGURATION(uav_model_id, configuration_type, status, approval_state)
CONFIGURATION_COMPONENT(configuration_id, component_id)
CONFIGURATION_VALIDATION(configuration_id, result, severity)
```

## 21. Transaction boundary

Saving a configuration shall be atomic with its component assignments and battery-set definition. Validation results shall reference the exact saved configuration revision.

## 22. Security and audit

Administrator changes to controlled technical data, compatibility rules and approval state shall be auditable.

At minimum retain:

```text
who
what
when
old_revision
new_revision
reason
source_basis
```

## 23. Implementation boundary

This schema defines persistence semantics. It does not prescribe a particular database engine.

The implementation may use SQLite, PostgreSQL or another qualified relational store, provided that revision integrity, constraints, transactions and auditability are preserved.

## 24. Relationship to existing BlueSky PRO architecture

```text
OFFICIAL REFERENCE CATALOG
          ↓
ADMINISTRATOR DATA STORE
          ↓
COMPATIBILITY RULES
          ↓
CONFIGURATION VALIDATOR
          ↓
ENERGY / RETURN FEASIBILITY
          ↓
AUTOMATIC CONFIGURATION SELECTOR
          ↓
FLIGHT PLANNING UI
```

This document is the storage-level implementation baseline for the Administrator configuration subsystem.