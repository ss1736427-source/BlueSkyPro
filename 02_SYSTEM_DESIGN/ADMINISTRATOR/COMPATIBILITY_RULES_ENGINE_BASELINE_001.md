# COMPATIBILITY RULES ENGINE BASELINE 001

## 1. Purpose

Define the controlled rule model used by BlueSky PRO to determine whether a UAV, battery, payload, propulsion, communication, navigation or other equipment item may participate in a configuration.

## 2. Core rule

The Compatibility Engine is authoritative for configuration eligibility.

```text
CATALOGUE ITEMS
      ↓
COMPATIBILITY RULES
      ↓
VALID CONFIGURATION SET
      ↓
UI SELECTION
```

The operational UI shall receive only items that pass the applicable selection rules.

## 3. Compatibility result

Each evaluated relation shall resolve to one of:

- `COMPATIBLE` — explicitly permitted by controlled data;
- `INCOMPATIBLE` — explicitly prohibited or technically impossible;
- `CONDITIONAL` — permitted only when specified conditions are satisfied;
- `UNKNOWN` — insufficient controlled information.

For automatic operational configuration, `UNKNOWN` shall not be treated as `COMPATIBLE`.

## 4. Rule structure

A compatibility rule shall contain at least:

```text
rule_id
source_item_type
source_item_id
target_item_type
target_item_id
relation
conditions
constraints
source_reference
source_revision
status
effective_from
effective_to
verification_state
```

## 5. Rule categories

### 5.1 Platform compatibility

Determines whether equipment is approved for a particular UAV model.

### 5.2 Interface compatibility

Checks mechanical, electrical, data and mounting interfaces.

### 5.3 Electrical compatibility

Checks applicable voltage, current, connector and power constraints using controlled manufacturer data.

### 5.4 Mass compatibility

Checks component mass against aircraft payload, MTOW and other controlled limits.

### 5.5 Performance compatibility

Checks whether the proposed configuration remains within controlled flight-performance limits.

### 5.6 Energy compatibility

Passes the resulting configuration to the Energy Model and rejects configurations that fail required energy/return-feasibility constraints.

### 5.7 Operational compatibility

Checks mission-specific requirements, environmental limits, equipment availability and configuration approval state.

## 6. Evaluation order

The engine shall apply deterministic filtering in the following order:

1. catalogue validity;
2. platform compatibility;
3. explicit prohibitions;
4. interface compatibility;
5. electrical/power constraints;
6. mass and payload constraints;
7. performance constraints;
8. energy constraints;
9. navigation and communication requirements;
10. mission-specific constraints;
11. configuration approval state.

The exact implementation may optimize evaluation order, but the resulting decision shall be equivalent and deterministic.

## 7. Explicit incompatibility has priority

If one controlled rule explicitly prohibits a combination, an optimization or recommendation algorithm shall not override that prohibition.

```text
INCOMPATIBLE
     ↓
REJECT
     ↓
DO NOT DISPLAY AS SELECTABLE
```

## 8. Conditional compatibility

A conditional relation shall carry machine-readable conditions. Example:

```text
UAV + PAYLOAD
relation = CONDITIONAL
condition = payload_mass <= remaining_payload_capacity
```

A conditional component is displayed only when the current configuration satisfies the condition.

## 9. Multi-component configuration

Compatibility shall be evaluated against the complete configuration, not only individual pairs.

```text
UAV
 + Battery set
 + Payload
 + Communication
 + Navigation
 + Other equipment
        ↓
Configuration-level validation
```

This prevents a set of individually compatible components from being accepted when their combined mass, power, balance, energy or interface requirements are invalid.

## 10. Multiple batteries

The data model shall support battery cardinality and battery sets.

A configuration may contain:

- one battery;
- multiple identical batteries;
- multiple batteries with defined topology;
- other manufacturer-documented arrangements.

A second battery shall not automatically be interpreted as doubled usable energy. Electrical topology, permitted installation and battery-model parameters must be controlled before the Energy Model uses the resulting set.

## 11. CUSTOM configuration

CUSTOM permits manual construction of a non-standard configuration.

CUSTOM may contain combinations that are absent from the standard configuration catalogue, but the engine shall still perform validation.

```text
CUSTOM INPUT
     ↓
RULE CHECK
     ↓
VALID / CONDITIONAL / UNKNOWN / INVALID
```

CUSTOM therefore provides freedom of construction without granting automatic operational approval.

## 12. UI contract

The Flight Planning interface shall not display known incompatible components in normal selection lists.

Filtering shall occur before presentation:

```text
Full catalogue
     ↓
Compatibility Engine
     ↓
Filtered catalogue
     ↓
Operator UI
```

The UI shall not be responsible for determining technical compatibility; it shall consume the engine result.

## 13. Automatic configuration

The automatic configurator shall operate only on the valid configuration set.

```text
Mission Requirements
        ↓
Candidate UAVs
        ↓
Compatibility Engine
        ↓
Valid configurations
        ↓
Energy / performance evaluation
        ↓
Ranking
        ↓
Recommended configuration
```

The optimizer shall never search the incompatible configuration space.

## 14. No-solution handling

If no configuration satisfies the mission requirements, the engine shall return:

```text
NO_VALID_CONFIGURATION
```

with the applicable blocking constraints. It shall not silently relax a hard compatibility or safety constraint.

## 15. Traceability

Every compatibility decision shall be traceable to the rule and source data used to produce it. This includes manufacturer documentation, controlled internal data and verified derived parameters where applicable.

## 16. Verification states

Compatibility rules shall be distinguishable from their verification status. Suggested controlled states:

- `UNREVIEWED`
- `REVIEWED`
- `VERIFIED`
- `SUSPENDED`
- `OBSOLETE`

A rule that has not reached the required verification state shall not be silently promoted to an approved operational rule.

## 17. Administrator responsibilities

Administrator controls:

- catalogue records;
- compatibility relations;
- conditions and constraints;
- source documents;
- revisions;
- verification state;
- configuration approval state.

The Administrator interface shall expose sufficient information to audit why a component is or is not selectable.

## 18. Resulting system principle

```text
ADMINISTRATOR
    ↓ controls data and rules
COMPATIBILITY ENGINE
    ↓ determines valid combinations
AUTOMATIC CONFIGURATOR
    ↓ selects the best valid combination
FLIGHT PLANNING
    ↓ presents only valid choices
ENERGY MODEL
    ↓ evaluates configuration-specific feasibility
VERIFICATION / APPROVAL
```

This baseline is the controlled design basis for subsequent implementation requirements, database schema, UI behavior and verification cases.