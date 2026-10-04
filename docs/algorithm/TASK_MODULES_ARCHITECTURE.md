# BlueSky PRO — Task Modules Architecture

**Status:** ARCHITECTURE BASELINE  
**Date:** 2026-10-04  
**Role:** Defines the reusable task-module layer between mission templates and the authoritative Task Model / Planning Kernel.

## 1. Purpose

BlueSky PRO separates three levels that MUST NOT be conflated:

1. **Mission Template** — defines the class of the overall operator mission and its primary solution method.
2. **Task Module** — defines a reusable capability or task component required to solve a part of that mission.
3. **Planning Kernel** — combines the selected template and modules into one authoritative, feasible mission.

The approved operator-facing 13-template catalog remains unchanged and is authoritative for UI taxonomy.

Task modules do not replace templates and do not become independent planners.

## 2. Canonical hierarchy

```
OPERATOR GENERAL TASK
        ↓
MISSION PROFILE
        ↓
MISSION TEMPLATE
        ↓
TASK MODULE SELECTION
        ↓
MODULE PARAMETERS
        ↓
TASK MODEL
        ↓
PLANNING KERNEL
        ↓
OPTIMIZATION LAYER
        ↓
SAFETY / CONFLICT RE-VERIFICATION
        ↓
MISSION RELEASE
```

The operator describes the required outcome first. The system determines which modules are necessary for the selected mission template and records their requirements in the Task Model.

## 3. Definitions

### 3.1 Mission Template

A template answers:

**What class of overall mission is being solved?**

The operator-facing taxonomy is the approved 13-template catalog in `08_PLANNING/BLUESKY_MISSION_TEMPLATE_CATALOG.md`:

1. Картографирование территории
2. 3D-картография / реконструкция
3. Инспекция объектов и инфраструктуры
4. Мониторинг строительства
5. Мониторинг территории и периметра
6. Поиск и спасение
7. Пожарный мониторинг и ЧС
8. Экологический и природный мониторинг
9. Сельское хозяйство
10. Доставка грузов
11. Ретрансляция связи
12. Аэрофотосъёмка и медиапроизводство
13. C-UAS — обнаружение БПЛА

The older functional planning classes (LONG FLIGHT, FAST FLIGHT, PUNCTUAL ARRIVAL, MAX COVERAGE, MAX PAYLOAD, MULTI-UAV DISTRIBUTION, etc.) are **planning objective profiles / solution patterns**, not a competing operator-facing template catalog. They may remain as internal planning profiles where required.

### 3.2 Task Module

A task module answers:

**What reusable component of the task must be solved?**

A module may be required by many different templates.

A module contains:

- module ID;
- purpose;
- required inputs;
- derived parameters;
- applicable geometry/methods;
- dependencies;
- output requirements;
- hard constraints;
- optimization criteria;
- verification requirements;
- linked interfaces.

### 3.3 Planning Service

A Planning Service is different from a Task Module.

Examples:

- Constraint Engine;
- Route Engine;
- Energy/Performance Engine;
- Trajectory Engine;
- Conflict Engine;
- Optimization Layer;
- Safety Verification.

These are authoritative system services and MUST NOT be duplicated as task modules.

## 4. Initial Task Module Registry

### TM-01 — POINT ROUTE

Purpose: connect ordered points or legs.

Supports:

- start/destination;
- ordered route points;
- route legs;
- speed/altitude requirements;
- actions at points.

Typical methods:

- waypoint;
- constrained route;
- direct/avoiding route.

Used by: LONG FLIGHT, FAST FLIGHT, PUNCTUAL ARRIVAL, INSPECTION, SEARCH, COMPLEX.

---

### TM-02 — MANDATORY PASSAGE

Purpose: preserve operator-defined mandatory passage/observation points.

Supports:

- geographic position;
- altitude/band;
- order;
- tolerance/corridor;
- dwell/observation;
- reason/type.

The authoritative representation is the common Task Model object already defined for mandatory points.

Used by any template.

---

### TM-03 — AREA COVERAGE

Purpose: systematically cover an area according to required coverage and quality.

Supports:

- polygon/3D area;
- required coverage;
- lane spacing;
- boundary extension;
- coverage verification.

Typical methods:

- grid;
- adaptive grid;
- crosshatch/double-grid where required.

Used by: AREA MONITORING, PHOTOGRAMMETRY, LiDAR, MAX COVERAGE, SEARCH, COMPLEX.

---

### TM-04 — CORRIDOR COVERAGE

Purpose: cover a linear or elongated operational area.

Supports:

- centreline;
- corridor width;
- parallel passes;
- terrain/surface following;
- segment-specific requirements.

Used by: CORRIDOR INSPECTION, INSPECTION, PHOTOGRAMMETRY, LiDAR, SEARCH, COMPLEX.

External planning systems also treat corridor planning as a distinct reusable planning method rather than a separate aircraft mission class. citeturn0search2turn0search9

---

### TM-05 — ORBIT / CIRCUMFERENTIAL OBSERVATION

Purpose: observe or acquire data around an object or point from controlled viewpoints.

Supports:

- centre/target;
- radius or standoff;
- altitude layers;
- camera/sensor orientation;
- overlap/viewpoint requirements.

Used by: INSPECTION, PHOTOGRAMMETRY, SEARCH, AREA MONITORING, COMPLEX.

---

### TM-06 — VERTICAL / SURFACE-FOLLOWING ACQUISITION

Purpose: acquire data from vertical, inclined or irregular surfaces while controlling sensor-to-surface geometry.

Supports:

- target surface;
- standoff distance;
- surface normal;
- local altitude;
- camera/sensor orientation;
- surface-relative clearance.

Used by: INSPECTION, CORRIDOR INSPECTION, PHOTOGRAMMETRY, LiDAR, COMPLEX.

Professional planning tools expose vertical-surface and surface-relative planning as reusable acquisition capabilities for facades, dams, towers and rock faces. citeturn0search1turn0search10

---

### TM-07 — PROFILE / TRANSECT

Purpose: acquire data along a defined line or sequence of cross-sections.

Supports:

- profile line;
- sampling interval;
- altitude/surface-relative distance;
- sensor orientation;
- repeat passes.

Used by: INSPECTION, CORRIDOR INSPECTION, LiDAR, PHOTOGRAMMETRY, SEARCH, COMPLEX.

---

### TM-08 — TARGET DETAIL ACQUISITION

Purpose: perform a localized high-detail observation/acquisition at a target.

Supports:

- target object;
- required viewpoint(s);
- sensor/payload;
- resolution/quality;
- dwell;
- evidence/data requirements.

This is deliberately not called INSPECTION: inspection remains a mission template, while this module is the reusable detailed-acquisition component.

Used by: INSPECTION, CORRIDOR INSPECTION, SEARCH, AREA MONITORING, PHOTOGRAMMETRY, LiDAR, COMPLEX.

---

### TM-09 — SEARCH / ADAPTIVE SEARCH

Purpose: systematically or adaptively search an area or route for targets.

Supports:

- search region;
- search pattern;
- detection requirements;
- probability/priority metadata;
- candidate target insertion;
- operator confirmation;
- replanning.

Used by: SEARCH, AREA MONITORING, INSPECTION, MULTI-UAV DISTRIBUTION, COMPLEX.

---

### TM-10 — DATA ACQUISITION / PRODUCT GENERATION

Purpose: define the data product that the flight must produce and the acquisition conditions required to produce it.

Supports:

- sensor/payload;
- GSD or point-density target;
- overlap;
- trigger/capture rules;
- orientation;
- georeferencing;
- calibration;
- storage;
- completeness criteria;
- re-acquisition criteria.

Used by: PHOTOGRAMMETRY, LiDAR, INSPECTION, CORRIDOR INSPECTION, AREA MONITORING, SEARCH, COMPLEX.

---

### TM-11 — TIME / DEADLINE

Purpose: impose temporal requirements on mission execution.

Supports:

- ETD;
- ETA;
- deadline;
- time windows;
- sequencing;
- synchronization;
- allowed delay.

Used by: FAST FLIGHT, PUNCTUAL ARRIVAL, LONG FLIGHT, AREA MONITORING, MULTI-UAV DISTRIBUTION, COMPLEX.

This module does not replace the PUNCTUAL ARRIVAL template; it provides temporal constraints that other templates may also use.

---

### TM-12 — ENERGY / ENDURANCE

Purpose: ensure that task execution remains within UAV energy and reserve limits.

Supports:

- current SOC;
- degradation;
- payload;
- predicted consumption;
- reserve;
- contingency;
- energy objective.

Used by all flight templates where energy is relevant.

The authoritative energy calculation remains the Energy/Performance service.

---

### TM-13 — MULTI-UAV DISTRIBUTION

Purpose: divide and coordinate work among multiple UAVs.

Supports:

- UAV capabilities;
- task allocation;
- sector allocation;
- departure timing;
- synchronization;
- workload balance;
- inter-UAV constraints.

Used by: MULTI-UAV DISTRIBUTION, PHOTOGRAMMETRY, LiDAR, SEARCH, AREA MONITORING, MAX COVERAGE, INSPECTION, CORRIDOR INSPECTION, COMPLEX.

This module does not replace the authoritative Conflict Engine or TICAS/deconfliction mechanism.

---

### TM-14 — PAYLOAD / CAPABILITY MATCHING

Purpose: ensure that the selected UAV/payload combination can produce the required task result.

Supports:

- sensor requirements;
- payload mass;
- power;
- field of view;
- range;
- resolution;
- endurance impact;
- UAV capability matching.

Used by: MAX PAYLOAD, PHOTOGRAMMETRY, LiDAR, INSPECTION, SEARCH, AREA MONITORING, COMPLEX.

---

### TM-15 — REPEAT / MONITORING CYCLE

Purpose: execute repeated acquisition or monitoring of the same task area/object.

Supports:

- repeat interval;
- reference mission;
- change detection;
- consistent acquisition geometry;
- temporal comparison;
- trigger conditions.

Used by: AREA MONITORING, INSPECTION, SEARCH, PHOTOGRAMMETRY, LiDAR, COMPLEX.

---

### TM-16 — RE-ACQUISITION / QUALITY RECOVERY

Purpose: recover incomplete or failed task output without declaring the whole mission successful.

Supports:

- failed coverage areas;
- weak image/data regions;
- missing observations;
- insufficient overlap;
- failed quality checks;
- targeted re-flight.

Used by: PHOTOGRAMMETRY, LiDAR, INSPECTION, CORRIDOR INSPECTION, AREA MONITORING, SEARCH, COMPLEX.

## 5. Module composition rule

A mission may use several modules simultaneously.

Example:

```
PHOTOGRAMMETRY / 3D MAPPING
    │
    ├── AREA COVERAGE
    ├── MANDATORY PASSAGE
    ├── SURFACE-FOLLOWING
    ├── DATA ACQUISITION
    ├── TIME / DEADLINE
    ├── ENERGY / ENDURANCE
    ├── MULTI-UAV DISTRIBUTION
    └── RE-ACQUISITION
```

This remains **one mission**.

The modules contribute requirements and constraints to the common Task Model. They do not create separate missions or independent route states.

## 6. Module selection

Module selection has two sources:

### Operator-selected

The operator explicitly requests a requirement.

Example:

- mandatory point;
- deadline;
- area;
- required data product;
- number of UAVs.

### System-derived

The system derives a required module from the task description, template, equipment or environment.

Example:

- selected 3D mapping product → DATA ACQUISITION;
- three UAVs required → MULTI-UAV DISTRIBUTION;
- steep terrain → SURFACE-FOLLOWING;
- deadline → TIME / DEADLINE;
- failed coverage after acquisition → RE-ACQUISITION.

The system MUST preserve the provenance of each module requirement:

`SOURCE = OPERATOR | TEMPLATE | SYSTEM-DERIVED | REPLANNING`

## 7. Compatibility principle

Modules are reusable, but not every combination is valid.

Before planning:

1. resolve template;
2. resolve required modules;
3. validate module dependencies;
4. detect incompatible requirements;
5. construct the Task Model;
6. enter the Planning Kernel.

Example incompatibility:

- required data product demands a sensor not available on the selected UAV;
- required surface standoff cannot be maintained within legal/obstacle constraints;
- deadline conflicts with mandatory reserve;
- multi-UAV allocation cannot satisfy separation constraints.

Such cases produce a structured infeasibility state, not a degraded hidden compromise.

## 8. Template-to-module baseline

| Template | Primary modules |
|---|---|
| Картографирование территории | AREA COVERAGE, DATA ACQUISITION, ENERGY, RE-ACQUISITION |
| 3D-картография / реконструкция | AREA COVERAGE, SURFACE-FOLLOWING, DATA ACQUISITION, PAYLOAD/CAPABILITY, RE-ACQUISITION |
| Инспекция объектов и инфраструктуры | TARGET DETAIL, ORBIT/SURFACE, MANDATORY PASSAGE, DATA ACQUISITION, RE-ACQUISITION |
| Мониторинг строительства | AREA COVERAGE, DATA ACQUISITION, REPEAT/MONITORING, TIME/DEADLINE |
| Мониторинг территории и периметра | AREA COVERAGE, SEARCH, DATA ACQUISITION, REPEAT/MONITORING |
| Поиск и спасение | SEARCH, AREA COVERAGE, TARGET DETAIL, TIME/DEADLINE, MULTI-UAV where applicable |
| Пожарный мониторинг и ЧС | AREA COVERAGE, SEARCH, TARGET DETAIL, TIME/DEADLINE, PAYLOAD/CAPABILITY |
| Экологический и природный мониторинг | AREA COVERAGE, DATA ACQUISITION, REPEAT/MONITORING, PAYLOAD/CAPABILITY |
| Сельское хозяйство | AREA COVERAGE, DATA ACQUISITION, REPEAT/MONITORING, PAYLOAD/CAPABILITY, ENERGY |
| Доставка грузов | POINT ROUTE, MANDATORY PASSAGE, TIME/DEADLINE, ENERGY, PAYLOAD/CAPABILITY |
| Ретрансляция связи | POINT ROUTE, MANDATORY PASSAGE, TIME/DEADLINE, ENERGY, PAYLOAD/CAPABILITY |
| Аэрофотосъёмка и медиапроизводство | POINT ROUTE, AREA COVERAGE, ORBIT/SURFACE, DATA ACQUISITION, PAYLOAD/CAPABILITY |
| C-UAS — обнаружение БПЛА | AREA COVERAGE, SEARCH, TARGET DETAIL, TIME/DEADLINE, PAYLOAD/CAPABILITY, MULTI-UAV where applicable |

This is a **baseline mapping**, not a restriction. Optional modules may be added when the operator task or environment requires them.

## 9. Relationship to existing architecture

Task Modules do not alter the authoritative chain:

```
OPERATOR UI
→ Mission Interface
→ Task Model Interface
→ Planning Kernel Interface
→ Constraint / Route / Energy / Trajectory / Conflict
→ Optimization
→ Safety Verification
→ Mission Release
```

The module layer is a requirement-composition step inside the Mission/Task definition stage.

Map, Profile and 3D remain linked views of the same Task Model and trajectory. Mandatory points retain the existing common ID and constraint semantics.

## 10. Relationship to Optimization Layer

Modules provide optimization inputs; they do not implement optimization independently.

Examples:

- AREA COVERAGE supplies coverage and geometry objectives;
- TIME/DEADLINE supplies temporal constraints;
- ENERGY supplies energy/reserve constraints and metrics;
- MULTI-UAV supplies allocation and workload objectives;
- DATA ACQUISITION supplies product-quality objectives;
- SEARCH supplies target-value/priority objectives.

The common Optimization Layer then evaluates the complete mission.

## 11. Relationship to Safety

Task modules MUST NOT override:

- airspace restrictions;
- NOTAM;
- altitude limits;
- terrain/obstacle clearance;
- UAV performance;
- energy reserve;
- multi-UAV separation;
- final safety verification.

All module-generated requirements enter the common Constraint/Safety chain.

## 12. No duplicate algorithms

The following remain centralized services:

- route feasibility;
- constraint checking;
- wind model;
- energy model;
- trajectory generation;
- multi-UAV conflict resolution;
- optimization;
- safety verification.

A module may request these services through linked interfaces but MUST NOT maintain a competing implementation.

## 13. Traceability

Every module requirement must be traceable:

`OPERATOR INPUT / TEMPLATE / SYSTEM DERIVATION → MODULE → TASK MODEL → PLANNING RESULT → VERIFICATION RESULT`

The source and version of module selection must be recorded with the mission planning state.

## 14. Engineering rule

**Template defines the overall mission class.**

**Module defines a reusable task capability.**

**Task Model stores the concrete requirements.**

**Planning Kernel composes the complete feasible mission.**

**Optimization improves the complete mission.**

**Safety Verification remains authoritative.**

## 15. Status

This document establishes the Task Module layer without changing the approved 13 mission templates or the existing Planning Kernel, Optimization Layer, mandatory-point model, or linked-interface architecture.

Next implementation step: formalize module schemas, dependency rules and machine-readable compatibility metadata before implementing additional template-specific solution methods.
