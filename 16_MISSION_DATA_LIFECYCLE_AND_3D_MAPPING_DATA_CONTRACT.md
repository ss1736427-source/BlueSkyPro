# BlueSky PRO — Mission Data Lifecycle and 3D Mapping Data Contract

**Status:** ARCHITECTURE BASELINE — proposed extension of the existing Data Layer
**Scope:** flight data, payload/sensor data, processing outputs, integrity, completeness, provenance and export
**Relationship:** extends the Common Mission Model and existing planning Data Contracts; does not replace or duplicate them.

## 1. Purpose

This contract defines the controlled lifecycle of data produced by a BlueSky PRO mission, with 3D mapping as the reference use case.

The contract separates:
- immutable source data;
- operational flight records;
- processed products;
- verification results;
- export/archive packages.

The planning contracts remain authoritative for planning artifacts. This contract governs the data generated and retained by execution and processing.

## 2. Canonical data flow

MISSION → FLIGHT EXECUTION → DATA INGEST → INTEGRITY CHECK → COMPLETENESS CHECK → QUALITY CHECK → PROCESSING → PRODUCT VERIFICATION → EXPORT / ARCHIVE

## 3. Mission Data Package

Every execution receives a stable missionId and produces a versioned Mission Data Package.

Logical structure:

MISSION_DATA_PACKAGE
├── mission.json
├── raw/
│   ├── imagery/
│   ├── lidar/
│   ├── telemetry/
│   ├── gnss/
│   └── sensor/
├── processed/
│   ├── pointcloud/
│   ├── mesh/
│   ├── ortho/
│   ├── dsm/
│   └── dtm/
└── verification/
    ├── manifest.json
    ├── completeness.json
    ├── quality.json
    └── provenance.json

This is a logical contract. Physical storage may use local storage, server storage or object storage without changing the domain model.

## 4. Data classes

| Class | Purpose | Mutability |
|---|---|---|
| RAW | Original payload/sensor files | immutable |
| FLIGHT | Flight log, navigation and execution record | append/version controlled |
| METADATA | Mission, UAV, payload, environment and configuration references | version controlled |
| PROCESSED | Derived products | immutable per processing version |
| VERIFICATION | Integrity, completeness and quality results | immutable per verification run |
| EXPORT | Reproducible delivery package | immutable |

## 5. Required metadata

Minimum:
- missionId;
- flightId;
- UAV identity and configuration reference;
- payload/sensor identity and configuration reference;
- mission definition/version;
- planning artifact/version references;
- execution start/end timestamps;
- coordinate reference system;
- environmental data references where applicable;
- software/processing version;
- configuration version;
- source file manifest;
- verification status.

Sensor-native metadata shall be preserved where available.

## 6. Source file manifest

Every ingested source file shall have:
- logical file identifier;
- relative object/path;
- media/type;
- byte size;
- acquisition timestamp where available;
- source device/sensor;
- missionId;
- flightId;
- SHA-256 checksum;
- ingestion timestamp;
- ingestion status.

The manifest itself is versioned and integrity protected.

## 7. Integrity

Integrity means that the received object is identical to the recorded source object.

Minimum mechanism:

SOURCE FILE → SHA-256 → MANIFEST

On controlled transfer or verification:

CURRENT HASH == MANIFEST HASH → PASS

Mismatch produces INTEGRITY_FAILURE and prevents the object from being treated as an unchanged source.

Cryptographic integrity does not establish data quality or completeness.

## 8. Completeness

Completeness is evaluated against the expected mission dataset, not only against file hashes.

The expected dataset shall be derived from mission and execution configuration where possible.

Examples:
- expected image/frame sequence;
- expected telemetry interval;
- expected GNSS coverage;
- expected sensor channels;
- expected mission segments;
- expected geographic coverage;
- expected processing inputs.

Result states:
- COMPLETE;
- INCOMPLETE;
- UNKNOWN;
- BLOCKED.

The system shall report missing intervals/files/segments rather than reducing the result to a single percentage.

## 9. 3D mapping quality checks

For 3D mapping, completeness and integrity are followed by content/geometry quality checks.

Where applicable:
- image metadata validity;
- timestamp continuity;
- GNSS/position availability;
- image sequence continuity;
- duplicate/corrupt frames;
- planned versus actual footprint;
- coverage percentage;
- overlap adequacy;
- altitude consistency;
- alignment/reconstruction quality;
- point-cloud density;
- gaps;
- georeferencing;
- control/check-point results when used.

A mathematically intact dataset may still be rejected as unsuitable for 3D reconstruction.

## 10. Processing provenance

Every derived product shall reference:
- source data manifest/version;
- processing algorithm/software version;
- processing configuration;
- coordinate reference system;
- processing timestamp;
- input artifact versions;
- verification result.

Example:

RAW DATA v1 → PROCESSING CONFIG v3 → PROCESSING ENGINE v5 → POINT CLOUD v2 → MESH v1

A new processing configuration creates a new product version. Previous products remain reproducible historical records.

## 11. Canonical product formats

| Product | Preferred interchange format |
|---|---|
| Point cloud | LAS / LAZ |
| Orthomosaic | GeoTIFF |
| DSM / DTM | GeoTIFF |
| GIS vector | GeoPackage / GeoJSON |
| 3D mesh | glTF / GLB, PLY or OBJ as required |
| Web 3D | 3D Tiles / glTF |
| Metadata/contracts | JSON |
| Flight/telemetry | native source + normalized representation |

The native source format remains part of RAW when supplied by the aircraft or payload.

## 12. Storage principle

Large binary sensor and product objects shall not be stored directly inside the primary transactional database.

Transactional DB:
- mission metadata;
- object identifiers;
- versions;
- status;
- references;
- traceability.

Object/File Storage:
- RAW;
- telemetry;
- imagery;
- point clouds;
- meshes;
- raster products.

Storage may be local, server-based or S3-compatible object storage. The domain contract remains storage-provider independent.

## 13. Data lifecycle states

RECEIVED → INTEGRITY_CHECK → COMPLETENESS_CHECK → QUALITY_CHECK → PROCESSING → PRODUCT_VERIFICATION → VERIFIED → EXPORTED / ARCHIVED

A failure at any gate produces an explicit failure state and reason.

## 14. Export

The export layer shall provide product-level and complete-package export.

Product-level examples:
- point cloud;
- 3D model;
- orthomosaic;
- DSM/DTM;
- GIS layers;
- flight record.

Complete Mission Data Package shall contain:
- source/reference manifest;
- selected RAW data or references;
- flight record;
- processed products;
- metadata;
- processing configuration/version;
- completeness result;
- integrity manifest;
- quality result;
- provenance.

ZIP or another archive container may be used for transport. The archive is not the canonical internal storage model.

## 15. Verification semantics

The following checks are independent:

INTEGRITY ≠ COMPLETENESS ≠ QUALITY ≠ REGULATORY / OPERATIONAL ACCEPTANCE

A product is eligible for operational use only when the required checks for its intended purpose have passed.

Example:

Integrity PASS + Completeness PASS + 3D Quality PASS + Required traceability PASS → PRODUCT VERIFIED

## 16. Relationship to existing BlueSky contracts

The existing Multi-UAV Planning Data Contract remains authoritative for:

ConstrainedOpenSpace → ZoneSet → ZoneAssignmentSet → RouteSet → PerformanceAdjustedRouteSet → TrajectorySet → ConflictReport → FinalCheckResult

This Mission Data Contract begins at execution/data acquisition and references those planning artifacts by ID/version.

It extends the existing architecture:

PLANNING CONTRACTS → EXECUTION → MISSION DATA CONTRACT → PROCESSING / VERIFICATION → EXPORT / ARCHIVE

No parallel Mission Core or parallel planning model is introduced.

## 17. Configuration and traceability

Every processing or verification result shall retain:
- source versions;
- UAV configuration;
- payload configuration;
- software version;
- algorithm version;
- processing configuration;
- environmental references where relevant;
- verification evidence;
- timestamp.

A change in a source or processing configuration invalidates dependent derived results for reuse until regenerated or explicitly revalidated.

## 18. Acceptance baseline

First implementation target:

MISSION
↓
FLIGHT
↓
RAW IMAGE + TELEMETRY INGEST
↓
SHA-256 MANIFEST
↓
COMPLETENESS CHECK
↓
QUALITY CHECK
↓
PROCESSING
↓
POINT CLOUD / ORTHO / DSM
↓
PRODUCT VERIFICATION
↓
EXPORT PACKAGE

Mandatory negative cases:
- corrupted source file;
- checksum mismatch;
- missing frame;
- telemetry gap;
- incomplete geographic coverage;
- invalid metadata;
- processing source-version mismatch;
- product verification failure;
- reproducibility/version mismatch.

## 19. Boundary

This contract defines data architecture and verification semantics. It does not prescribe one photogrammetry engine, one storage vendor, one database implementation or one proprietary processing algorithm.

## Status

**MISSION DATA LIFECYCLE: ARCHITECTURE BASELINE — initial contract**

The contract should be refined into executable schemas and implementation interfaces before production processing is introduced.
