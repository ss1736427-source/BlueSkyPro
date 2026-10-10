# BlueSky PRO — CUSTOM UAV MODEL / MANUAL CONFIGURATION 001

## 1. Purpose

Define a special `CUSTOM` UAV model for cases where the aircraft, battery, payload or other equipment is not represented by a predefined manufacturer-supported configuration in the reference catalogue.

The `CUSTOM` model is intended to demonstrate and support manual configuration of an arbitrary UAV and its equipment while keeping the normal predefined-model workflow unchanged.

## 2. CUSTOM principle

```text
CUSTOM UAV
   ↓
MANUAL COMPONENT SELECTION
   ↓
MANUAL PARAMETERS
   ↓
CONFIGURATION
```

Unlike a predefined UAV model, the CUSTOM model does not limit the operator to a predefined manufacturer compatibility matrix during configuration entry.

The user may manually define/select, where the system data model permits:

- UAV body/platform;
- one or more batteries;
- payloads;
- propulsion components;
- communication equipment;
- navigation equipment;
- mounting/interface information;
- other mission equipment;
- mass and other required engineering parameters.

## 3. Important distinction from predefined models

For a predefined UAV:

```text
UAV MODEL
 ↓
COMPATIBILITY RULES
 ↓
ONLY COMPATIBLE ITEMS ARE SHOWN
```

For `CUSTOM`:

```text
CUSTOM
 ↓
ALL MANUALLY AVAILABLE COMPONENTS
 ↓
USER BUILDS CONFIGURATION
```

This is an explicit exception to the operator-facing compatibility filtering rule for predefined models.

## 4. Manual battery configuration

CUSTOM shall support configurations such as:

```text
Battery configuration:
 ├── Battery A × 1
 ├── Battery A × 2
 ├── Battery A × 4
 ├── Battery B × 2
 └── mixed battery set
```

The data model shall preserve quantity and position/role where relevant.

Example:

```text
BATTERY_SET
├── battery_id: BAT-A
├── quantity: 2
├── connection: parallel / series / manufacturer-defined / UNKNOWN
└── user-defined parameters
```

The system must not silently assume that batteries are electrically safe merely because the user selected two units. Electrical configuration is a parameter of the CUSTOM configuration and may be `UNKNOWN` until specified.

## 5. Non-standard equipment

CUSTOM shall permit manual attachment of equipment that is absent from the standard equipment catalogue.

Example:

```text
CUSTOM UAV
 ├── Battery × 2
 ├── Camera X
 ├── Thermal sensor Y
 ├── Laser rangefinder Z
 ├── Radio module Q
 └── Custom mount
```

A custom component may be created as a configuration-local record or subsequently promoted into the controlled Administrator catalogue.

## 6. Required manual data

The CUSTOM editor shall request only the parameters required for the selected component and the calculations that the user wants to perform.

Examples include:

- mass;
- dimensions;
- power consumption;
- voltage/current range;
- energy capacity;
- mounting/interface;
- centre of gravity contribution;
- environmental limits;
- propulsion characteristics;
- communication characteristics;
- navigation characteristics.

A parameter not supplied by the user shall remain `UNKNOWN`; the system shall not invent a value.

## 7. Two validation states

CUSTOM requires a strict distinction between **configuration entry** and **flight eligibility**.

### 7.1 Configuration entry

The user is allowed to assemble a CUSTOM configuration manually even when compatibility is not established.

### 7.2 Flight eligibility

Before a CUSTOM configuration can be used for an operational flight plan, required safety, engineering, performance and regulatory checks must be satisfied according to the applicable BlueSky PRO approval process.

Therefore:

```text
MANUAL CONFIGURATION ≠ AUTOMATICALLY APPROVED CONFIGURATION
```

## 8. CUSTOM status

Suggested lifecycle:

```text
DRAFT
 ↓
MANUAL_CONFIGURED
 ↓
VALIDATION_REQUIRED
 ↓
VALIDATED
 ↓
APPROVED
 ↓
ACTIVE
```

A newly created CUSTOM configuration shall begin in `DRAFT` / `MANUAL_CONFIGURED` state and shall not be treated as an approved operational configuration merely because all fields have values.

## 9. UI behaviour

When the user selects:

```text
UAV MODEL → CUSTOM
```

the interface shall change from the predefined compatibility-driven mode to the CUSTOM editor.

The editor shall expose manual selectors for:

```text
AIRFRAME
BATTERIES
PAYLOADS
PROPULSION
COMMUNICATION
NAVIGATION
OTHER EQUIPMENT
PARAMETERS
```

The user may add, remove and duplicate component instances and edit their quantities and applicable parameters.

## 10. Compatibility handling in CUSTOM

The predefined-model rule remains:

```text
INCOMPATIBLE → NOT RETURNED → NOT DISPLAYED
```

For CUSTOM, this rule is replaced by:

```text
USER SELECTS
 ↓
SYSTEM RECORDS RELATIONSHIP
 ↓
CHECKS WHAT CAN BE CHECKED
 ↓
FLAGS UNRESOLVED / INVALID CONDITIONS
```

The system shall never convert `UNKNOWN` into `COMPATIBLE`.

## 11. Energy Model integration

A CUSTOM configuration shall be accepted by the Energy Model only when sufficient inputs exist for the requested calculation.

The Energy Model shall distinguish:

- manufacturer-declared values;
- user-entered values;
- derived values;
- estimated values;
- unknown values.

If an essential input is unavailable, the calculation shall return a controlled `INSUFFICIENT_DATA` result rather than fabricate an estimate.

## 12. Example — double battery + non-standard payload

```text
CUSTOM UAV

Airframe:
    Custom Hexacopter

Battery:
    Battery-X × 2

Payload:
    Non-standard Camera-X

Additional equipment:
    Thermal Sensor-Y
    Radio-Z

Mount:
    Custom payload mount

User parameters:
    aircraft mass = entered value
    payload mass = entered value
    battery energy = entered/documented value
    payload power = entered/documented value
```

BlueSky PRO stores this as one explicit CUSTOM configuration. It does not silently replace the equipment with a known catalogue component or reinterpret the configuration as a predefined UAV.

## 13. Administrator promotion

A successful CUSTOM configuration may later be promoted into the controlled catalogue:

```text
CUSTOM CONFIGURATION
       ↓
DOCUMENTATION / SOURCE COLLECTION
       ↓
ENGINEERING REVIEW
       ↓
COMPATIBILITY DEFINITION
       ↓
VALIDATION
       ↓
APPROVED UAV / EQUIPMENT RECORD
```

This permits project-specific aircraft to become reusable standard configurations without contaminating the predefined reference catalogue with unverified data.

## 14. Architecture rule

BlueSky PRO therefore has two deliberate configuration modes:

### STANDARD

```text
OFFICIAL / CONTROLLED UAV
 ↓
AUTOMATIC CONFIGURATION
 ↓
ONLY COMPATIBLE EQUIPMENT
 ↓
OPTIMIZATION
```

### CUSTOM

```text
CUSTOM UAV
 ↓
FULL MANUAL CONFIGURATION
 ↓
USER-DEFINED COMPONENTS / QUANTITIES
 ↓
VALIDATION / CALCULATION CHECKS
```

The existence of CUSTOM does not weaken the compatibility controls of the standard catalogue.

## 15. Acceptance criteria

1. `CUSTOM` is selectable as a distinct UAV model type.
2. CUSTOM permits manual selection of multiple batteries.
3. CUSTOM permits non-standard equipment.
4. CUSTOM permits user-defined component parameters.
5. CUSTOM preserves component quantity and configuration relationships.
6. Missing parameters remain `UNKNOWN`.
7. CUSTOM configuration is not automatically considered flight-approved.
8. Standard UAV models continue to expose only compatible equipment.
9. CUSTOM configurations can subsequently be submitted for controlled validation and catalogue promotion.
10. Energy and navigation calculations consume the actual resolved CUSTOM configuration rather than substituting a predefined UAV model.
