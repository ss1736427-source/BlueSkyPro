---
id: UAV-REFERENCE-CATALOG-001
type: administrator_uav_reference_catalog
status: reference_only
schema_version: UAV-REFERENCE-CATALOG-001
---

# UAV Reference Catalog 001

## Purpose

This dataset is the initial Administrator reference catalogue for the ten canonical UAV platform classes and manufacturer-referenced aircraft models. Runtime source: \`qt/BlueSkyPRO-HMI/qml/UavReferenceCatalog.js\`.

## Classification

The ten type records preserve the identifiers already established in \`07_DATA/UAV_Types/UAV-TYPE-001.md\` through \`UAV-TYPE-010.md\`. They are mission/platform classes, not manufacturer model identities.

| ID | Type | Family |
|---|---|---|
| UAV-TYPE-001 | Multirotor | MULTIROTOR |
| UAV-TYPE-002 | Fixed-Wing | FIXED_WING |
| UAV-TYPE-003 | VTOL | VTOL |
| UAV-TYPE-004 | HALE | HALE |
| UAV-TYPE-005 | Cargo UAV | CARGO |
| UAV-TYPE-006 | Drone-in-a-Box UAV | DIB |
| UAV-TYPE-007 | Relay UAV | RELAY |
| UAV-TYPE-008 | Inspection UAV | INSPECTION |
| UAV-TYPE-009 | Search and Rescue UAV | SAR |
| UAV-TYPE-010 | Multi-Mission UAV | MULTI_MISSION |

## Initial model population

| Model ID | Manufacturer / model | Type |
|---|---|---|
| DJI-M350-RTK | DJI Matrice 350 RTK | UAV-TYPE-001 |
| DJI-M300-RTK | DJI Matrice 300 RTK | UAV-TYPE-001 |
| DJI-M30 | DJI Matrice 30 | UAV-TYPE-001 |
| DJI-M30T | DJI Matrice 30T | UAV-TYPE-001 |
| DJI-M4E | DJI Matrice 4E | UAV-TYPE-001 |
| DJI-M4T | DJI Matrice 4T | UAV-TYPE-001 |
| DJI-MAVIC-3E | DJI Mavic 3 Enterprise | UAV-TYPE-001 |
| DJI-MAVIC-4-PRO | DJI Mavic 4 Pro | UAV-TYPE-001 |
| DJI-AIR-3S | DJI Air 3S | UAV-TYPE-001 |
| DJI-MINI-4-PRO | DJI Mini 4 Pro | UAV-TYPE-001 |
| DJI-M30D | DJI Matrice 3D | UAV-TYPE-006 |
| DJI-M30TD | DJI Matrice 3TD | UAV-TYPE-006 |
| DJI-MAVIC-3T | DJI Mavic 3 Thermal | UAV-TYPE-001 |
| AUTEL-EVO-MAX-4T | Autel EVO Max 4T | UAV-TYPE-001 |
| WINGTRAONE-GEN2 | WingtraOne GEN II | UAV-TYPE-003 |
| TRINITY-PRO | Quantum-Systems Trinity Pro | UAV-TYPE-003 |

## Data semantics

- Numeric values are manufacturer-published reference maxima or reference test values, not BlueSky operational limits.
- A model record is \`REFERENCE_ONLY\`; its existence does not create an approved aircraft/configuration.
- Unknown values are represented as null/UNKNOWN and listed in the record's \`unknowns\` field.
- Published flight time is not operational endurance. The mission energy model must use a separately qualified configuration-specific profile.
- Wind resistance is condition-specific. For Mavic 3E, the 12 m/s figure is limited to take-off/landing; Matrice 4 series wind resistance is likewise qualified by the manufacturer as a take-off/landing figure.
- WingtraOne MTOW discrepancy (4.5 kg vs 4.8 kg in different manufacturer materials) remains unresolved; the value is intentionally null.
- Maximum angular velocity, pitch angle and maximum speed are not substitutes for validated turn radius, sustained lateral acceleration, braking envelope or bank limits.

## Flight Planning integration

Fleet entries carry the canonical platform type ID and reference-catalogue version. Mission Profile resolves the selected type and its reference model set through the same catalogue source. This data may inform mission context and later model/configuration selection.

It does **not** populate \`performanceProfile\`, set \`validationState=VALIDATED\`, set \`approvalState=APPROVED\`, or satisfy the dynamic flight-safety gate. An exact approved configuration and its qualified performance/safety envelope remain mandatory.

## Source basis

The initial records are transcribed from:
- \`02_SYSTEM_DESIGN/ADMINISTRATOR/UAV_CONFIGURATION_REFERENCE_CATALOG_001.md\`;
- \`02_SYSTEM_DESIGN/ADMINISTRATOR/UAV_OFFICIAL_REFERENCE_CATALOG_EXPANSION_002.md\`;
- \`07_DATA/UAV_Types/UAV-MANEUVERABILITY-REFERENCE-001.md\`.

Manufacturer URLs are stored on each runtime model record. The catalogue is an initial reference population, not a complete manufacturer database.


## Russian civil and industrial manufacturer additions (2026-11-15)

Catalog revision: `2026-11-15-RU-001`. The runtime catalogue now includes a manufacturer registry for three Russian manufacturers and nine model/configuration records. Manufacturer country is recorded separately from component sourcing.

| Model ID | Manufacturer / model | Canonical type | Source |
|---|---|---|---|
| GEOSCAN-201-GEO | ГК «Геоскан» — Геоскан 201 Геодезия | UAV-TYPE-002 Fixed-Wing | https://www.geoscan.ru/ru/products/geoscan201/geo |
| GEOSCAN-401-GEO | ГК «Геоскан» — Геоскан 401 Геодезия | UAV-TYPE-001 Multirotor | https://www.geoscan.ru/ru/products/geoscan401/geo |
| GEOSCAN-701 | ГК «Геоскан» — Геоскан 701 | UAV-TYPE-002 Fixed-Wing | https://www.geoscan.ru/ru/products/geoscan701 |
| GEOSCAN-801 | ГК «Геоскан» — Геоскан 801 | UAV-TYPE-001 Multirotor | https://www.geoscan.ru/ru/products/geoscan801 |
| GEOSCAN-GEMINI | ГК «Геоскан» — Геоскан Gemini | UAV-TYPE-001 Multirotor | https://www.geoscan.ru/ru/products/gemini |
| SUPERCAM-S150 | ГК «Беспилотные системы» — Supercam S150 | UAV-TYPE-002 Fixed-Wing | https://supercam.aero/catalog/supercams150 |
| SUPERCAM-S350 | ГК «Беспилотные системы» — Supercam S350 | UAV-TYPE-002 Fixed-Wing | https://supercam.aero/catalog/supercam-s350 |
| SUPERCAM-SX350 | ГК «Беспилотные системы» — Supercam SX350 | UAV-TYPE-003 VTOL | https://supercam.aero/catalog/supercam-sx350 |
| ALBATROS-M5 | ООО «Альбатрос» — Альбатрос М5 | UAV-TYPE-002 Fixed-Wing | https://www.alb.aero/catalog/bpla-samoletnogo-tipa/albatros-m5/ |

### Source and data-quality controls

- Only civil/industrial mapping, surveying, inspection and monitoring platforms with accessible manufacturer product pages were added in this batch.
- ZALA and other military/dual-use product families were not seeded in this batch; they require a separate scope decision and source/qualification review. No tactical or weapon-related performance records are included.
- `manufacturerCountry: "RU"` identifies manufacturer origin, not domestic content, component origin or import-substitution status.
- Supercam S350 MTOW is `UNKNOWN`: the Russian and English manufacturer pages publish different values (15.5 kg and 11.5 kg). The value is not selected by guesswork.
- Albatros M5 manufacturer page contains conflicting endurance/range figures (up to 4.5 h / 300 km in promotional text versus up to 4 h / 250 km in the detailed specification). These fields remain unknown pending a controlled manual/revision.
- The Geoscan 201 record is the Geoscan 201 Geodesy configuration only. Geoscan 401 Geodesy is kept separate from 401 Geophysics/Lidar variants.
- Speeds converted from km/h to m/s are reference display values only. They are not operational limits.
- Manufacturer reference data does not populate approved aircraft configuration, performance profile, dynamic safety envelope or READY evidence.


## 25. Explicit fleet-to-model identity boundary

The HMI seed entries `BS-001` through `BS-004` are marked `recordOrigin: DEMO_FIXTURE`. They carry empty `referenceModelId` and `configurationId`; their type IDs classify demo platform classes only. They are not physical aircraft records and must not be mapped to a manufacturer model by inference.

Production identity binding shall keep these identifiers distinct:

- `vehicleId`: stable BlueSky identity of the physical airframe;
- `serialNumber`: manufacturer/airframe serial;
- `referenceModelId`: exact catalogue model/variant ID;
- `referenceTypeId`: canonical platform type; must agree with the selected model's `typeId`;
- `configurationId` and `configurationRevision`: exact controlled assembly;
- `performanceProfile.vehicleId`: same physical vehicle;
- `performanceProfile.configurationId`: same configuration;
- `performanceProfile.modelId` and model version: exact model identity/revision used by the approved profile.

Mission Profile resolves a model only when an explicit `referenceModelId` exists. It reports missing model/configuration linkage and identifies demo fixtures. A present configuration ID is a reference only; it does not prove validation or approval.

The HMI performs no compatibility, configuration approval, performance-profile validation or safety validation. Those decisions remain with the authoritative Administrator/configuration provider and existing typed safety validator. No reference-catalogue field is copied into `performanceProfile`, `FlightSafetyEnvelope`, or lifecycle evidence.

A production Fleet provider remains necessary before physical aircraft assignment can be claimed. Until then, demonstration records remain fixtures and cannot acquire a concrete physical-model/configuration binding from the reference catalogue alone.
