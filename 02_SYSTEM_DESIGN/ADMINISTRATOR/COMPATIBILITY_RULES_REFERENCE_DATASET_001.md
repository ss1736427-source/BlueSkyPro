# COMPATIBILITY RULES REFERENCE DATASET 001

## 1. Purpose

Controlled reference dataset for the BlueSky PRO Compatibility Engine. This dataset instantiates compatibility relations from the existing UAV configuration and official-reference materials. It is a reference dataset, not an operational approval register.

## 2. Decision semantics

| Result | Meaning | Automatic selector |
|---|---|---|
| `COMPATIBLE` | Explicitly supported by controlled source data | Eligible |
| `CONDITIONAL` | Eligible only when stated conditions pass | Eligible after condition evaluation |
| `INCOMPATIBLE` | Explicitly prohibited or incompatible | Excluded |
| `UNKNOWN` | No sufficient controlled evidence | Excluded from automatic operational selection |

## 3. Reference UAV rules

| RULE_ID | UAV_MODEL | TARGET_TYPE | TARGET_ID | RELATION | STATUS | SOURCE BASIS |
|---|---|---|---|---|---|---|
| CR-M350-TB65-001 | M350 RTK | BATTERY | TB65 | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-TB60-001 | M350 RTK | BATTERY | TB60 | COMPATIBLE | VERIFIED_REFERENCE | Existing compatibility reference matrix |
| CR-M350-TB60-TB65-001 | M350 RTK | BATTERY_SET | TB60+TB65 | INCOMPATIBLE | VERIFIED_REFERENCE | Existing compatibility reference matrix |
| CR-M400-TB100-001 | M400 | BATTERY | TB100 | COMPATIBLE | VERIFIED_REFERENCE | Existing compatibility reference matrix |
| CR-M400-MULTIBAT-001 | M400 | BATTERY_SET | MULTIPLE | INCOMPATIBLE | VERIFIED_REFERENCE | Existing compatibility reference matrix |

## 4. M350 RTK payload reference rules

| RULE_ID | UAV_MODEL | TARGET_TYPE | TARGET_ID | RELATION | STATUS | SOURCE BASIS |
|---|---|---|---|---|---|---|
| CR-M350-H30-001 | M350 RTK | PAYLOAD | H30 | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-H30T-001 | M350 RTK | PAYLOAD | H30T | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-L2-001 | M350 RTK | PAYLOAD | L2 | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-L1-001 | M350 RTK | PAYLOAD | L1 | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-P1-001 | M350 RTK | PAYLOAD | P1 | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-H20-001 | M350 RTK | PAYLOAD | H20 | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-H20T-001 | M350 RTK | PAYLOAD | H20T | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-H20N-001 | M350 RTK | PAYLOAD | H20N | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-S1-001 | M350 RTK | PAYLOAD | S1 | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |
| CR-M350-V1-001 | M350 RTK | PAYLOAD | V1 | COMPATIBLE | VERIFIED_REFERENCE | Existing official-reference dataset |

## 5. M350 equipment rules

| RULE_ID | UAV_MODEL | TARGET_TYPE | TARGET_ID | RELATION | STATUS | SOURCE BASIS |
|---|---|---|---|---|---|---|
| CR-M350-M400ACC-001 | M350 RTK | EQUIPMENT | M400_ACCESSORY | INCOMPATIBLE | VERIFIED_REFERENCE | Existing compatibility reference matrix |

## 6. Other reference platforms

For M300 RTK, M30/M30T, M4E/M4T and Autel EVO Max 4T, only relations explicitly present in the existing controlled reference materials shall be instantiated here. No additional payload or battery compatibility is inferred from physical similarity, connector similarity, product-family membership or third-party claims.

| RULE_ID | PLATFORM | RULE_POLICY | RESULT |
|---|---|---|---|
| CR-M300-DEFAULT-001 | M300 RTK | Use only explicitly controlled component relations | UNKNOWN unless source relation exists |
| CR-M30-DEFAULT-001 | M30/M30T | Use only explicitly controlled component relations | UNKNOWN unless source relation exists |
| CR-M4-DEFAULT-001 | M4E/M4T | Use only explicitly controlled component relations | UNKNOWN unless source relation exists |
| CR-AUTEL-EVOMAX-DEFAULT-001 | EVO Max 4T | Use only explicitly controlled component relations | UNKNOWN unless source relation exists |

## 7. Configuration-level rules

Pairwise compatibility is insufficient. The engine shall evaluate the complete configuration.

| RULE_ID | CHECK | CONDITION | FAILURE RESULT |
|---|---|---|---|
| CR-CFG-MASS-001 | Total mass | Configuration mass within applicable controlled aircraft limit | INCOMPATIBLE |
| CR-CFG-PAYLOAD-001 | Payload capacity | Payload/configuration load within applicable controlled limit | INCOMPATIBLE |
| CR-CFG-POWER-001 | Electrical compatibility | Required voltage/current/power constraints satisfied | INCOMPATIBLE |
| CR-CFG-INTERFACE-001 | Interface | Required mechanical/electrical/data interface exists and is supported | INCOMPATIBLE |
| CR-CFG-ENERGY-001 | Energy feasibility | Energy Model accepts configuration for mission | INCOMPATIBLE |
| CR-CFG-RETURN-001 | Return feasibility | Required return scenario remains feasible under approved model | INCOMPATIBLE |
| CR-CFG-UNKNOWN-001 | Missing evidence | Required compatibility fact is UNKNOWN | UNKNOWN / BLOCK |

## 8. Automatic selector contract

The automatic selector shall consume only rules with an eligible result after all applicable conditions are evaluated.

```text
MISSION
  ↓
CANDIDATE UAV
  ↓
COMPONENT CANDIDATES
  ↓
COMPATIBILITY FILTER
  ↓
CONFIGURATION VALIDATION
  ↓
ENERGY / RETURN FEASIBILITY
  ↓
VALID CONFIGURATION SET
  ↓
RANKING
```

The selector shall not substitute a best-effort component when a required compatibility relation is UNKNOWN.

## 9. UI filtering contract

The UI shall present only components surviving the current compatibility filter.

```text
CATALOGUE
   ↓
CURRENT UAV + CURRENT CONFIGURATION
   ↓
RULE EVALUATION
   ↓
VISIBLE SELECTABLE ITEMS
```

Known incompatible items are hidden from normal selection. Unknown items are also excluded from automatic operational selection. An Administrator audit view may expose excluded items and the reason for exclusion.

## 10. CUSTOM rule policy

CUSTOM configurations use the same validation engine but permit manual entry of non-standard components and relationships.

Examples:

```text
CUSTOM UAV
 + Battery A × 2
 + Non-standard payload
 + Additional communication module
```

For every manually entered relationship:

- explicit compatibility evidence → `COMPATIBLE`;
- conditionally permitted relation → `CONDITIONAL`;
- explicit conflict → `INCOMPATIBLE`;
- insufficient evidence → `UNKNOWN`.

A CUSTOM configuration with unresolved required `UNKNOWN` relations shall not receive automatic operational approval.

## 11. Source control

Every rule record must retain its source reference, source revision/date and verification state. Rules shall be revised rather than silently overwritten when manufacturer documentation changes.

## 12. Dataset boundary

This file deliberately does not invent missing technical characteristics. Values such as voltage, current, power consumption, connector type, mounting geometry, maximum payload, battery energy and performance coefficients shall be populated only from the controlled UAV/component source records or explicitly approved derived calculations.

## 13. Next implementation layer

The next implementation artifact shall map this reference dataset into the Administrator persistence model and define CRUD operations, rule precedence, condition evaluation and configuration-validation result codes.