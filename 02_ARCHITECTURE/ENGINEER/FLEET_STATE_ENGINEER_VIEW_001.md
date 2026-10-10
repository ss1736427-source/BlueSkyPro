# BLUESKY PRO — FLEET STATE / ENGINEER VIEW 001

## Status
CANDIDATE_FOR_AGREEMENT

## Purpose
Define a lightweight Engineer-level view of the actual operational state of the UAV fleet during ground preparation and maintenance coordination.

## Core Principle
The Engineer needs a fleet-level operational picture, but does not need a separate MRO system inside BlueSky PRO.

BlueSky PRO shall consume relevant maintenance and operational state through its existing integration boundary and present only information required for the Engineer's current responsibilities.

## Engineer Fleet View

The Engineer shall be able to identify, at fleet level:

- UAVs ready for operation;
- UAVs unavailable for operation;
- UAVs requiring maintenance or inspection;
- UAVs approaching scheduled maintenance limits;
- UAVs with expired or approaching document validity;
- UAVs with operational restrictions;
- UAVs with equipment-related restrictions;
- UAVs requiring Engineer attention.

## Information Hierarchy

### Always visible

Only compact operational indicators required for immediate decision-making:

- UAV identifier;
- readiness state;
- maintenance/service warning state;
- documentation warning state;
- restriction state;
- attention indicator.

### Available on demand

Detailed information is opened only when the Engineer selects a UAV or an indicated condition:

- maintenance history;
- service intervals and due dates;
- documentation details;
- installed equipment;
- equipment status;
- defects/findings;
- restrictions and reasons;
- relevant technical records;
- source and timestamp of external data.

## Selection Behavior

Selecting a UAV from the fleet list opens its detailed operational state in the main work area.

The Engineer can then move freely through the relevant equipment and technical-information tabs without losing the fleet context.

If comparison is required, a second UAV may be opened in a separate adjacent work window. Each window retains independent navigation and functions.

## Maintenance Integration Boundary

External maintenance/MRO systems remain authoritative for data they own. BlueSky PRO consumes only the data required for planning, readiness and Engineer decisions.

Conceptually:

```text
External Maintenance / MRO
          ↓
      INTEGRATION
          ↓
   NORMALIZED STATE
          ↓
     BlueSky PRO
          ↓
 Engineer Fleet View
```

The integration boundary shall permit future connection to different maintenance systems without redesigning the Engineer UI or mission-planning core.

## Safety and Authority

The Engineer's fleet view supports technical readiness and operational decisions. It does not by itself authorize flight.

A UAV marked READY remains subject to all applicable mission, airspace, weather, equipment, personnel and release controls.

## UX Principle

The fleet view shall not become a dashboard containing every available technical parameter.

Frequently used operational information remains immediately visible. Detailed and infrequently used information remains behind contextual selection or submenus.

The interface shall prefer state indicators over persistent numerical/detail panels where the detailed value is not required for the Engineer's current action.

## Verification Considerations

Verify at minimum:

- correct fleet-level readiness indication;
- correct maintenance warning indication;
- correct documentation warning indication;
- correct restriction indication;
- correct handling of stale/unavailable external data;
- correct transition from fleet view to UAV detail;
- independent two-UAV comparison windows;
- preservation of role and access boundaries;
- preservation of mission Safety Gate and Release Gate.

## Traceability Note

This document complements the Administrator Engineer/Technician authority model and the external maintenance integration architecture. It does not create a standalone maintenance-management system.
