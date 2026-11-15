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
