# BLUESKY PRO — PC / TABLET / HUB SYNCHRONIZATION ARCHITECTURE 001

## Status
CANDIDATE_FOR_AGREEMENT

## Purpose
Define the architectural principle for synchronization between BlueSky PRO workstations and BlueSky HUB without creating a separate mission-management subsystem.

## Architectural Principle
PC, tablet and HUB shall work with the same logical mission and operational data model. Synchronization is a controlled exchange of revisions and state, not a blind file replacement.

## Operational Roles

```text
ENGINEER / PC
      │
      │ mission preparation / configuration
      ↓
   BLUE SKY HUB
      │
      │ controlled synchronization
      ↓
PILOT / TABLET
      │
      │ execution / local operational data
      ↓
   BLUE SKY HUB
```

The actual direction of data exchange depends on the operation. The architecture shall not assume that the PC is always the source or that the tablet is always a consumer.

## Mission Identity
Each synchronized mission shall have a stable mission identifier. Changes shall be represented by a revision/version identifier and associated with the originating device or system context.

## Synchronization States
The user-facing system shall be able to represent, as applicable:
- Synchronized;
- Local changes pending synchronization;
- Remote changes pending synchronization;
- Conflict requiring resolution;
- Synchronization failed;
- Data stale or unavailable.

## Conflict Principle
A local modification shall never be silently overwritten by a remote revision. When concurrent changes affect the same controlled object, the system shall preserve the relevant versions and require a defined resolution process.

## HUB Role
BlueSky HUB may act as a synchronization and communication point between authorized BlueSky PRO clients. HUB shall not become an unnecessary duplicate of the mission-planning core.

## Offline Relationship
A client may continue working with locally available data when disconnected. After reconnection, locally created or modified revisions shall be synchronized through the defined synchronization mechanism.

```text
ONLINE
PC ←→ HUB ←→ TABLET

OFFLINE
PC   → LOCAL WORK
TABLET → LOCAL WORK

RECONNECT
LOCAL REVISIONS
      ↓
CONTROLLED SYNC
      ↓
CONFLICT CHECK
      ↓
CONSISTENT STATE
```

## Data Scope
Synchronization shall be limited to data for which the user and device have appropriate authorization. Examples include mission data, approved configuration references, operational status and other explicitly synchronized objects.

Technical maintenance data and external-system records shall remain subject to the existing Integration Layer and access-control architecture.

## Safety Boundary
Synchronization shall not itself authorize a flight, release an aircraft, or bypass mandatory validation. A synchronized mission must still pass the applicable current safety and operational gates before execution.

## UX Principle
Synchronization state should normally be shown through compact status indicators. Detailed synchronization diagnostics, revisions and conflict resolution shall be available only when required.

## Architecture Extension
The synchronization mechanism shall be transport-independent so that local connection, network connection and future approved communication channels can use the same logical synchronization model.

## Verification Considerations
Verify at minimum:
- synchronization of a mission from PC to tablet;
- synchronization of tablet-generated operational changes where permitted;
- offline modification and later synchronization;
- revision identification;
- conflict detection and preservation of both relevant versions;
- failed synchronization recovery;
- authorization boundaries;
- preservation of safety and release gates.

## Traceability
This document extends the offline-first operational architecture and complements the external maintenance and external processing integration architectures. Before baseline approval, duplicate and overlap checks shall be completed against the Master Requirements Register.
