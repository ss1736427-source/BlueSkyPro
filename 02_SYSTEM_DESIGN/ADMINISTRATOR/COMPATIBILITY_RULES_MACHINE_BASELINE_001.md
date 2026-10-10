# COMPATIBILITY RULES MACHINE BASELINE 001

## 1. Purpose

Machine-oriented baseline for the BlueSky PRO Compatibility Engine. This document converts the controlled compatibility model into explicit rule records suitable for subsequent database/schema implementation.

## 2. Decision contract

```text
COMPATIBLE   = eligible candidate
CONDITIONAL  = candidate only after all conditions evaluate TRUE
UNKNOWN      = excluded from automatic operational selection
INCOMPATIBLE = rejected and hidden from normal selection
```

Explicit `INCOMPATIBLE` has priority over any weaker positive relation.

## 3. Canonical rule record

```yaml
rule_id: CR-<domain>-<number>
source_item_type: UAV|BATTERY|PAYLOAD|PROPULSION|COMMUNICATION|NAVIGATION|EQUIPMENT|CONFIGURATION
source_item_id: <catalogue-id>
target_item_type: <type>
target_item_id: <catalogue-id>
relation: COMPATIBLE|CONDITIONAL|UNKNOWN|INCOMPATIBLE
conditions: []
constraints: []
source_reference: <controlled source>
source_revision: <revision/date>
status: ACTIVE|SUSPENDED|OBSOLETE
verification_state: UNREVIEWED|REVIEWED|VERIFIED
```

## 4. Initial reference rules

| Rule ID | Source | Target | Relation | Condition / constraint | Verification basis |
|---|---|---|---|---|---|
| CR-UAV-M350-BAT-001 | M350 RTK | TB65 | COMPATIBLE | Battery set permitted by manufacturer | Official DJI documentation |
| CR-UAV-M350-BAT-002 | M350 RTK | TB60 | COMPATIBLE | Battery set permitted by manufacturer | Official DJI documentation |
| CR-UAV-M350-BAT-003 | M350 RTK | TB60 + TB65 | INCOMPATIBLE | Mixed battery types prohibited | Official DJI FAQ |
| CR-UAV-M350-PAY-001 | M350 RTK | H30 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-002 | M350 RTK | H30T | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-003 | M350 RTK | L2 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-004 | M350 RTK | L1 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-005 | M350 RTK | P1 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-006 | M350 RTK | H20 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-007 | M350 RTK | H20T | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-008 | M350 RTK | H20N | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-009 | M350 RTK | S1 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-010 | M350 RTK | V1 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M350-PAY-011 | M350 RTK | arbitrary third-party payload | UNKNOWN | Explicit support not established | Source gap |
| CR-UAV-M400-BAT-001 | M400 | TB100 | COMPATIBLE | Manufacturer-specified battery | Official DJI documentation |
| CR-UAV-M400-BAT-002 | M400 | dual-battery arrangement | INCOMPATIBLE | Manufacturer specifies single battery | Official DJI documentation |
| CR-UAV-M400-PAY-001 | M400 | L3 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M400-PAY-002 | M400 | H30 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M400-PAY-003 | M400 | H30T | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M400-PAY-004 | M400 | L2 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M400-PAY-005 | M400 | P1 | COMPATIBLE | Manufacturer-supported payload | Official DJI documentation |
| CR-UAV-M400-X-001 | M400 | M350 accessory | INCOMPATIBLE | Manufacturer ecosystems not interchangeable | Official DJI documentation |
| CR-UAV-M30-PAY-001 | M30 | external arbitrary gimbal | UNKNOWN | No blanket compatibility established | Source gap |
| CR-UAV-M30T-PAY-001 | M30T | external arbitrary gimbal | UNKNOWN | No blanket compatibility established | Source gap |
| CR-UAV-M4-PAY-001 | M4E/M4T | arbitrary payload | UNKNOWN | Explicit support required | Source gap |
| CR-UAV-M4-X-001 | M4E/M4T | supported E-Port/PSDK accessory | CONDITIONAL | Installation/interface restrictions satisfied | Official DJI documentation |
| CR-UAV-M4-X-002 | M4E/M4T | accessory installed after power-on | INCOMPATIBLE | Manufacturer installation requirement | Official DJI documentation |
| CR-UAV-EVO4T-BAT-001 | EVO Max 4T | ABX40 | COMPATIBLE | Manufacturer battery variant | Official Autel documentation |
| CR-UAV-EVO4T-BAT-002 | EVO Max 4T | ABX41 | COMPATIBLE | Manufacturer battery variant | Official Autel documentation |

## 5. Generic machine constraints

The following constraints are evaluated after pairwise compatibility:

```text
C-MASS-001:
  total_mass <= configured_MTOW

C-PAYLOAD-001:
  payload_mass <= remaining_payload_capacity

C-POWER-001:
  electrical_load <= approved_power_capacity

C-INTERFACE-001:
  required_interface == available_interface

C-BATTERY-001:
  battery_count within permitted_cardinality

C-BATTERY-002:
  battery_topology == documented_topology

C-ENERGY-001:
  predicted_energy_reserve >= required_energy_reserve

C-RETURN-001:
  predicted_return_requirement <= available_energy_margin
```

A constraint shall only become operationally enforceable when its parameter source and acceptance basis are controlled.

## 6. Filtering algorithm

```text
function getSelectableComponents(configuration, catalogue, rules):
    candidates = catalogue

    for item in candidates:
        result = evaluateRules(configuration, item, rules)

        if result == INCOMPATIBLE:
            hide(item)
        elif result == UNKNOWN:
            hide(item)              # automatic operational mode
        elif result == CONDITIONAL:
            if conditionsSatisfied(configuration, item):
                show(item)
            else:
                hide(item)
        elif result == COMPATIBLE:
            show(item)

    return visibleItems
```

## 7. Configuration-level validation

```text
validate(configuration):
    1. validate catalogue records
    2. evaluate explicit compatibility rules
    3. evaluate interfaces
    4. evaluate electrical constraints
    5. evaluate mass / MTOW
    6. evaluate payload limits
    7. evaluate performance constraints
    8. evaluate energy model
    9. evaluate mission constraints
   10. evaluate approval state
```

Any blocking failure produces `INVALID_CONFIGURATION`.

## 8. Automatic configuration

The automatic configurator receives only candidates that pass the selection gate.

```text
MISSION
  ↓
UAV CANDIDATES
  ↓
SELECTABLE COMPONENTS
  ↓
VALID CONFIGURATION COMBINATIONS
  ↓
ENERGY + PERFORMANCE
  ↓
RANKING
  ↓
RECOMMENDED CONFIGURATION
```

The optimizer shall not use `UNKNOWN` items to improve a recommendation.

## 9. CUSTOM mode

CUSTOM bypasses catalogue completeness, not validation.

Manual input may define:

```text
battery_count = N
battery_models = [...] 
payloads = [...]
custom_equipment = [...]
custom_interfaces = [...]
custom_mass = ...
custom_power = ...
```

The engine evaluates every supplied element and the complete configuration.

Recommended result states:

```text
VALIDATED
REQUIRES_VALIDATION
INVALID
UNKNOWN
```

`REQUIRES_VALIDATION`, `INVALID` and `UNKNOWN` shall not be represented as approved operational configurations.

## 10. Administrator data rule

Administrator shall be able to create, revise, suspend and retire rules without changing application code. Every active rule must reference its source and verification state.

## 11. Source discipline

The machine baseline does not create new technical compatibility claims. Each concrete relation must originate from the controlled reference catalogue or an approved source record. Missing evidence remains `UNKNOWN`.

## 12. Next implementation artefacts

1. Database entities for catalogue and rules;
2. rule expression schema;
3. deterministic evaluation service;
4. filtered UI data contract;
5. automatic configuration ranking contract;
6. verification procedures for compatibility decisions;
7. traceability links to source evidence.
