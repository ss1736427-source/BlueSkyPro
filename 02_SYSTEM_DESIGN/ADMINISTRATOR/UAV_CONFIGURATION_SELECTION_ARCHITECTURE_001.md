# UAV CONFIGURATION SELECTION ARCHITECTURE 001

## 1. Purpose

Define the Administrator and Flight Planning configuration model so that operators are presented only with technically compatible components and, by default, the system automatically selects a suitable UAV configuration for the mission.

## 2. Architectural principle

Manual selection of arbitrary components is not the normal operational workflow.

The system shall maintain a controlled catalogue of UAV models, equipment and compatibility rules. Flight Planning shall use those rules to construct only valid configurations and shall hide incompatible components from operator selection.

## 3. Separation of concerns

### 3.1 UAV Model

Defines the aircraft platform and its manufacturer-documented characteristics.

### 3.2 Equipment

Defines batteries, payloads, propulsion, communication and navigation equipment as separately controlled catalogue objects.

### 3.3 Configuration

Defines a concrete combination of a UAV and compatible equipment. Configuration-specific calculated parameters are derived from its components.

### 3.4 Mission Profile

Defines the operational requirements of the current task. It is not a hardware configuration.

## 4. Administrator structure

```text
ADMINISTRATOR
├── UAV MODELS
├── BATTERIES
├── PAYLOADS
├── PROPULSION
├── COMMUNICATION
├── NAVIGATION
├── EQUIPMENT INTERFACES
├── COMPATIBILITY RULES
├── APPROVED CONFIGURATIONS
└── PARAMETER SOURCES / DOCUMENTATION
```

Administrator controls the catalogue, compatibility rules, revisions and approval state.

## 5. Compatibility engine

Compatibility shall be evaluated before an item is presented as selectable to the operator.

Required checks include, where applicable:

- aircraft/equipment compatibility;
- mounting/interface compatibility;
- battery voltage and electrical compatibility;
- connector/interface compatibility;
- mass and MTOW limits;
- payload capacity;
- dimensions and mechanical constraints;
- centre-of-gravity/balance constraints where data are controlled;
- power-system constraints;
- communication compatibility;
- navigation equipment compatibility;
- environmental and operating limits;
- configuration approval/status.

An incompatible component shall not appear in the operational selection list.

## 6. Automatic configuration

Default operational workflow:

```text
MISSION REQUIREMENTS
        ↓
UAV CANDIDATE SET
        ↓
COMPATIBILITY FILTER
        ↓
VALID CONFIGURATION SET
        ↓
PERFORMANCE / LIMITS FILTER
        ↓
ENERGY MODEL
        ↓
NAVIGATION / COMMUNICATION CHECK
        ↓
CONFIGURATION OPTIMIZATION
        ↓
RECOMMENDED CONFIGURATION
```

The system shall select from approved compatible combinations rather than inventing a combination at runtime.

## 7. Operator interface rule

When the operator selects a UAV model, the equipment selectors shall contain only equipment compatible with that UAV and the current configuration constraints.

Example:

```text
M350 RTK

Battery:
  TB65                 [available]
  incompatible items   [hidden]

Payload:
  compatible payload A [available]
  compatible payload B [available]
  incompatible items   [hidden]
```

The operator may accept the automatic recommendation or select among the remaining valid alternatives.

## 8. No-valid-configuration behavior

If no configuration satisfies the mission requirements, the system shall not expose incompatible hardware as an apparent solution.

The UI shall instead report that no valid configuration is currently available and identify the controlling constraint(s), where permitted by the system's explainability/correction mechanism.

## 9. Configuration state

Configurations shall have controlled lifecycle states:

- DRAFT
- VALIDATED
- APPROVED
- ACTIVE
- SUSPENDED
- OBSOLETE

Flight Planning shall use only configurations permitted by the applicable operational state rules.

## 10. Traceability

Every controlled parameter used for configuration or calculation shall retain its source, revision and applicability information. Configuration revision shall therefore be traceable to the component revisions from which it was constructed.

## 11. Energy Model integration

The selected configuration shall provide the Energy Model with configuration-specific inputs rather than generic UAV-class assumptions. At minimum the integration shall carry the applicable aircraft mass, battery characteristics, payload/equipment contribution and controlled operating constraints.

The Energy Model shall distinguish manufacturer-declared values from derived/modelled values.

## 12. Certification-oriented rule

Automatic configuration is a selection mechanism constrained by controlled data; it is not permitted to override an approved limitation, compatibility rule, or safety constraint.

Any future optimizer shall operate only on the valid configuration set produced by the compatibility engine.

## 13. Acceptance statement

The intended architecture is:

```text
Administrator controls WHAT IS ALLOWED.
Compatibility Engine determines WHAT CAN BE COMBINED.
Automatic Configurator determines WHAT BEST FITS THE TASK.
Flight Planning presents ONLY VALID OPTIONS.
```

This document establishes the architectural basis for subsequent data-model, UI and verification requirements.