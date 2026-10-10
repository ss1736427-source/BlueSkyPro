---
id: UAV-CUSTOM-CONFIGURATION-EXAMPLE-RULES-001
type: custom_configuration_rules
status: draft_for_agreement
---

# BlueSky PRO — CUSTOM Configuration Example Rules 001

## 1. Purpose

Define how a CUSTOM UAV may be assembled manually when the required hardware combination is not represented by an approved standard configuration.

CUSTOM is an engineering configuration mode, not an automatic authorization to fly.

## 2. Example: dual battery

CUSTOM shall support a battery set rather than a single battery field.

```text
BATTERY SET
├── Battery A
├── Battery B
├── Battery C ...
└── electrical/topology definition
```

For example, an administrator/engineer may enter:

```text
Battery A = Li-ion pack X
Battery B = Li-ion pack X
Quantity = 2
```

The system shall calculate the resulting configuration values only from the declared topology and controlled battery data. It shall not assume that two batteries are automatically equivalent to one battery with doubled capacity unless the electrical topology and model explicitly support that derivation.

A mixed set may be entered in CUSTOM when required by engineering work:

```text
Battery A = X
Battery B = Y
```

but the resulting state shall be marked according to validation status and shall not inherit the compatibility status of either component individually.

## 3. Example: non-standard payload

CUSTOM shall permit entry of a payload that is not present in the standard manufacturer compatibility catalogue.

Required data include, where applicable:

- payload identity;
- mass;
- dimensions;
- attachment/mounting method;
- power consumption;
- supply voltage/range;
- communication interface;
- data interface;
- centre-of-gravity effect;
- environmental limits;
- operational restrictions;
- source/documentation.

Example:

```text
UAV: CUSTOM
Payload: non-standard camera
Mass: controlled input
Power: controlled input
Mount: custom adapter
Interface: defined by engineering record
Status: REQUIRES VALIDATION
```

## 4. Example: custom equipment stack

CUSTOM may combine:

```text
UAV frame
+ battery set
+ custom propulsion
+ standard payload
+ non-standard payload
+ communication module
+ navigation equipment
+ auxiliary equipment
```

Every component remains an independent record. The configuration object stores references and derived values rather than duplicating authoritative component definitions.

## 5. Manual configuration workflow

```text
CUSTOM UAV
    ↓
SELECT / ENTER COMPONENTS
    ↓
DECLARE QUANTITY / TOPOLOGY
    ↓
ENTER REQUIRED TECHNICAL PARAMETERS
    ↓
RUN VALIDATION
    ↓
CALCULATE DERIVED VALUES
    ↓
SHOW WARNINGS / BLOCKERS
    ↓
SAVE CONFIGURATION
```

## 6. Validation classes

The validator shall distinguish:

### VALIDATED

Required technical data are present, applicable constraints are satisfied, compatibility is established and the configuration has passed the required engineering review.

### REQUIRES VALIDATION

The configuration can be saved for engineering work but is not eligible for operational use.

### INVALID

A known hard constraint is violated, such as an exceeded mass limit or incompatible electrical condition.

### UNKNOWN

Required information or evidence is missing. UNKNOWN is not equivalent to VALID.

## 7. Hard-block rules

CUSTOM shall not permit operational promotion when a known blocking condition exists.

Examples:

```text
TOTAL MASS > MTOW
required voltage outside component range
known incompatible connector/interface
known incompatible mounting arrangement
required navigation capability unavailable
energy feasibility cannot be established where required
operational limit exceeded
mandatory parameter missing for a safety-critical calculation
```

## 8. Double-battery example and Energy Model

The Energy Model must receive the resulting battery configuration explicitly:

```text
Configuration
    ↓
Battery topology
    ↓
Nominal / available energy model
    ↓
Current energy state
    ↓
Degradation / derating
    ↓
Consumption model
    ↓
Return energy requirement
```

The model shall not silently use standard-aircraft battery assumptions for a CUSTOM battery set.

## 9. Standard vs CUSTOM operator behaviour

```text
STANDARD UAV
    ↓
Only approved compatible components are shown

CUSTOM UAV
    ↓
Manual engineering configuration is available
    ↓
Validation status is explicit
    ↓
Only VALIDATED/APPROVED configurations can enter an operational workflow
```

Thus CUSTOM expands engineering flexibility without weakening the operational compatibility gate.

## 10. Saving and reuse

A validated CUSTOM configuration may be promoted into a reusable named configuration only after the applicable review/approval process.

Recommended lifecycle:

```text
CUSTOM-DRAFT
      ↓
ENGINEERING-VALIDATION
      ↓
VALIDATED
      ↓
APPROVED
      ↓
ACTIVE
```

## 11. Traceability

The record shall retain:

- configuration ID and revision;
- component IDs and revisions;
- source documents;
- entered custom parameters;
- derived parameters and derivation method;
- validation results;
- reviewer/approval data;
- Energy Model version;
- applicable mission constraints.

## 12. Design principle

CUSTOM is deliberately more permissive at the data-entry level than STANDARD, but it is not more permissive at the operational-safety level.

```text
CUSTOM = maximum engineering configurability
        + explicit validation
        + no silent assumptions
        + no bypass of safety gates
```

This document is the basis for subsequent UI and verification requirements for CUSTOM configuration.