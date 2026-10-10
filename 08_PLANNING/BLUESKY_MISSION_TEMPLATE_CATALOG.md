# BlueSky PRO — Mission Template Catalog

**Status:** APPROVED UI BASELINE  
**Decision date:** 2026-10-01  
**Scope:** Operator-facing mission template list in the left **Missions** panel.

## 1. Decision

The left panel shall contain **13 practical mission templates**. The operator selects the operational task; BlueSky derives the applicable objective profile, planning strategy, algorithms, vehicle/payload requirements and constraints. The operator is not expected to select algorithms manually.

The catalog is a task taxonomy for the UI. It does not replace the Common Mission Model or Mission Objective Profiles.

## 2. Approved catalog

| ID | Template | Typical operational purpose |
|---|---|---|
| MT-01 | Картографирование территории | Area survey, aerial imagery, orthomosaic and terrain/surface products |
| MT-02 | 3D-картография / реконструкция | Photogrammetric or LiDAR capture for 3D models, point clouds and reconstruction |
| MT-03 | Инспекция объектов и инфраструктуры | Inspection of buildings, bridges, power lines, pipelines and other assets |
| MT-04 | Мониторинг строительства | Progress tracking, site changes, comparison against design and volume estimates |
| MT-05 | Мониторинг территории и периметра | Patrol, observation, change detection and situational awareness |
| MT-06 | Поиск и спасение | Search-area coverage, locating people/animals and support to rescue operations |
| MT-07 | Пожарный мониторинг и ЧС | Fire observation, hotspot assessment, disaster-area survey and damage assessment |
| MT-08 | Экологический и природный мониторинг | Observation of forests, water bodies, pollution, habitats and wildlife |
| MT-09 | Сельское хозяйство | Crop/field monitoring, plant-condition assessment and precision-agriculture tasks |
| MT-10 | Доставка грузов | Transport of cargo, samples, medicines or supplies between defined locations |
| MT-11 | Ретрансляция связи | Airborne communications relay and temporary extension of radio coverage |
| MT-12 | Аэрофотосъёмка и медиапроизводство | Photo/video capture, event coverage and live visual broadcast |
| MT-13 | C-UAS — обнаружение БПЛА | Detection, classification and tracking of aerial objects, subject to compatible sensors and authorized operating scope |

## 3. UI behavior

1. The operator creates or opens a mission and selects the operational task.
2. BlueSky identifies the applicable template(s) and highlights them in the left panel.
3. A mission may use more than one template where the task requires it (for example, inspection plus 3D reconstruction).
4. The selected task is translated into the Common Mission Model and the corresponding objective profile(s).
5. Vehicle, payload, route, environment, airspace, C2, energy and safety constraints are resolved and validated by the relevant system modules.
6. The UI presents task-relevant parameters and validation results; internal algorithm selection remains automatic.

Highlighting a template means it is applicable to the current mission; it does not by itself indicate that the mission is authorized or ready for flight.

## 4. Items that are not mission templates

The following concepts remain available in the system but belong to other configuration dimensions:

| Item | Correct classification |
|---|---|
| Drone-in-a-Box | Basing / launch-and-recovery infrastructure profile |
| BVLOS | Operating mode / operational conditions and regulatory constraints |
| Автоматическое обнаружение | Payload capability or analytics function; associate with a task when applicable |
| Групповая / роёвая миссия | Fleet coordination mode / multi-UAV mission structure |

These dimensions can be combined with any compatible task template. They must not be counted as additional task templates in the left-panel catalog.

## 5. Architectural alignment

This catalog is subordinate to and must remain consistent with:

- `BLUESKY_MISSION_MODEL.md` — user intent, mission identity, geometry, constraints, vehicle/payload references and lifecycle.
- `BLUESKY_MISSION_OBJECTIVE_PROFILES.md` — objective hierarchy, admissibility gates and task-specific evaluation.

The template label is a user-facing classification. It must not hard-code a single planning algorithm, UAV type, payload, flight mode or optimization strategy.

## 6. Change control

The approved baseline is **13 templates**. Additions, removals or renaming that change the operator-facing taxonomy require an explicit documentation update and consistency review against the Common Mission Model, objective profiles and HMI.

