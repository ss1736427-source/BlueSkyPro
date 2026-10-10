# BLUESKY PRO — PRACTICAL KML IMPORT RECOMMENDATION 001

## Status
CANDIDATE_FOR_AGREEMENT

## Decision
Include KML import as a small interoperability capability within mission preparation. Do not create a separate GIS subsystem.

## Rationale
External KML geometry can reduce manual recreation of an area, boundary or reference geometry in Flight Chart. The imported object is only source geometry and must not bypass BlueSky PRO planning and safety controls.

## UX Decision
Use a contextual action during mission preparation:

`Import → KML → Preview → Use as mission geometry`

No permanent top-level menu item is required solely for KML.

## Architectural Decision
KML shall enter the existing mission-planning data flow through an import boundary. The mission-planning core remains independent of the external file format so that other formats can be added later through the same mechanism.

## Safety Decision
The following distinction is mandatory:

`External geometry ≠ flight route ≠ authorized flight`

Imported geometry must be subjected to the applicable route, airspace, safety and mission validation before it can contribute to an executable mission.

## Verification
Verify import, validation, preview, coordinate integrity, unsupported/invalid data handling and preservation of all mandatory planning gates.

## Relationship to Existing Architecture
This recommendation complements the existing Flight Planning and external integration architecture and does not create an independent GIS or MRO subsystem.
