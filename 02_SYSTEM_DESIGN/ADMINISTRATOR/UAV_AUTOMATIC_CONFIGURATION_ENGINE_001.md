# UAV AUTOMATIC CONFIGURATION ENGINE 001

## 1. Purpose

Define the deterministic selection logic that converts mission requirements into an approved UAV configuration without allowing incompatible component combinations.

## 2. Core rule

The engine shall never optimize over the complete equipment catalogue.

It shall optimize only over the set of configurations that have already passed compatibility and controlled-constraint validation.

```text
CATALOGUE
   ↓
COMPATIBILITY / CONSTRAINT VALIDATION
   ↓
VALID CONFIGURATION SET
   ↓
MISSION FILTER
   ↓
PERFORMANCE / ENERGY EVALUATION
   ↓
RANKING
   ↓
RECOMMENDED CONFIGURATION
```

## 3. Inputs

The engine shall accept, as applicable:

- mission type;
- mission geometry and area;
- required altitude;
- required sensor capability;
- required accuracy/GSD;
- endurance requirement;
- range requirement;
- environmental conditions;
- payload requirements;
- operational constraints;
- available UAV fleet;
- available equipment;
- configuration lifecycle/status.

## 4. Candidate generation

Candidate UAV models shall be selected from controlled `UAV_MODEL` records.

Only models with the required operational status and valid source/provenance chain may enter candidate generation.

## 5. Compatibility filtering

For each candidate UAV, the engine shall resolve applicable compatibility relationships for:

- batteries;
- payloads;
- propulsion;
- communication;
- navigation;
- mounting/interfaces;
- other controlled equipment classes.

Rules:

```text
COMPATIBLE   → candidate remains
CONDITIONAL  → remains only if condition is machine-checkably satisfied
INCOMPATIBLE → candidate removed
UNKNOWN      → candidate removed from automatic operational selection
```

## 6. Configuration constraints

The engine shall evaluate configuration-level constraints, including where applicable:

- maximum take-off mass;
- payload capacity;
- battery quantity and permitted combinations;
- electrical/power limits;
- mechanical interfaces;
- dimensional limits;
- centre-of-gravity constraints when controlled;
- environmental limits;
- communication and navigation requirements.

A component shall be removed before presentation if adding it would make the current configuration invalid.

## 7. Automatic versus manual configuration

### 7.1 Automatic mode

Automatic mode starts from mission requirements and returns one or more ranked valid configurations.

The engine chooses components; the operator does not construct an arbitrary combination.

### 7.2 Manual mode

Manual mode is available through the `CUSTOM` model/configuration workflow.

The operator/administrator may construct a non-standard configuration, including multiple batteries or non-standard equipment, subject to the separate validation and approval process defined for CUSTOM configurations.

## 8. Multiple batteries

Battery selection shall be represented as a set rather than a single scalar field.

```text
BATTERY_SET
├── battery_id
├── quantity
├── position/slot where applicable
├── compatibility state
└── configuration constraints
```

The engine shall not assume that two individually compatible batteries are automatically compatible as a pair.

Pairing, quantity and mixed-battery rules must be explicitly represented or derived only from controlled technical constraints.

## 9. Non-standard equipment

Non-standard equipment shall not be silently treated as equivalent to manufacturer-approved equipment.

For CUSTOM configurations the record shall retain:

- equipment identity;
- mass;
- power demand where applicable;
- interface/mount information;
- dimensions where applicable;
- source/documentation;
- validation status;
- uncertainty/unknown fields.

## 10. Energy evaluation

After compatibility and hard constraints have been passed, the engine shall call the Energy Model using the resolved configuration.

The Energy Model shall evaluate the configuration-specific energy inputs and return the applicable flight/return feasibility results.

A configuration failing an applicable energy feasibility constraint shall not be ranked as an operational solution.

## 11. Mission suitability

The engine shall distinguish hard constraints from optimization objectives.

Hard constraints eliminate a configuration.

Optimization objectives rank the remaining valid configurations.

Example:

```text
HARD:
- required payload capability
- MTOW limit
- compatibility
- operational limits
- minimum required endurance

OPTIMIZE:
- energy margin
- flight time
- payload suitability
- route efficiency
- other approved mission objectives
```

## 12. Ranking

The ranking mechanism shall be deterministic for identical inputs and controlled data revisions.

The result shall include the selected configuration and the principal factors that determined its ranking.

## 13. Operator presentation

The operator interface shall receive only the filtered result set.

```text
ENGINE RESULT
      ↓
VALID CONFIGURATIONS ONLY
      ↓
UI
```

Incompatible equipment shall not be returned merely to be displayed disabled.

## 14. No-solution result

If no valid configuration satisfies all hard constraints, the engine shall return a structured `NO_VALID_CONFIGURATION` result.

The result shall identify the blocking constraint categories where this information is available and controlled.

The engine shall not weaken constraints automatically to produce a result.

## 15. Explainability / Corrections

Where the system provides a correction or explanation, it shall identify the relevant controlled input or constraint and distinguish it from the final recommendation.

The explanation shall not imply that an incompatible configuration is acceptable.

## 16. Traceability

Each automatic recommendation shall be traceable to:

```text
MISSION_REQUIREMENTS revision
        ↓
UAV_MODEL revision
        ↓
COMPONENT revisions
        ↓
COMPATIBILITY rule revisions
        ↓
CONFIGURATION revision
        ↓
ENERGY_MODEL revision
        ↓
SELECTION_ENGINE revision
        ↓
RECOMMENDATION
```

## 17. Configuration snapshot

When a configuration is selected for a mission, the system shall retain a configuration snapshot/reference sufficient to reproduce the calculation against the same controlled data revision.

## 18. Safety principle

Automatic configuration is a constrained decision mechanism, not an authorization mechanism.

The engine shall not override an approved limitation, missing mandatory data, unresolved compatibility relationship, or failed operational constraint.

## 19. Acceptance criteria

The design is acceptable when the implementation can demonstrate that:

1. incompatible components are excluded before operator selection;
2. `UNKNOWN` compatibility is not accepted for automatic operational selection;
3. multiple-battery configurations are evaluated as sets;
4. non-standard CUSTOM configurations remain distinguishable from approved standard configurations;
5. automatic selection operates only on valid configurations;
6. failed hard constraints eliminate candidates;
7. energy feasibility is evaluated before operational recommendation;
8. identical inputs and revisions produce deterministic selection results;
9. every recommendation is traceable to its controlled input revisions;
10. no-solution conditions are reported without silently relaxing constraints.
