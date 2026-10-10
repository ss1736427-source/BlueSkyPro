# BLUESKY PRO — ENGINEER MENU BASELINE 001

## Status
CANDIDATE_FOR_AGREEMENT

## Purpose
Define the minimal first-level navigation for the ENGINEER role in BlueSky PRO, based on the Engineer's actual ground-maintenance and technical responsibilities.

## Core Principle
The Engineer menu shall contain only functions directly related to the Engineer's responsibilities.

The first navigation level shall not reproduce functions already available elsewhere in the system or expose internal system architecture as menu structure.

Frequently used information and actions shall remain directly accessible. Secondary, detailed and infrequently used information shall be opened contextually from the selected object or status indicator.

## First-Level Menu

```text
ENGINEER
│
├── FLEET
├── PERSONNEL
└── DOCUMENTS
```

This is the baseline first-level menu.

## 1. FLEET

`FLEET` is the Engineer's primary working area.

It provides a fleet-level view of the actual state of UAVs under the Engineer's responsibility and allows selection of a specific UAV for detailed work.

At fleet level, the Engineer should immediately see only compact information required for decisions, including:

- UAV identifier;
- readiness state;
- maintenance/service warning;
- documentation warning;
- restriction state;
- attention indication.

Selecting a UAV opens its detailed work area.

### UAV Detail

The selected UAV is presented in the central work area with the primary information required for current technical work available without additional navigation.

Detailed information is organized through contextual tabs, for example:

```text
GENERAL | EQUIPMENT | SERVICE | DOCUMENTS | FINDINGS | ...
```

The exact tab set may depend on the UAV and the Engineer's access scope.

Navigation between tabs is free. Selecting a tab replaces the central content with that tab's content without forcing the Engineer through a predefined sequence.

If two UAVs must be compared, a second work window may be opened beside the first. Each window retains independent navigation and available functions.

## 2. PERSONNEL

`PERSONNEL` is limited to personnel information required for the Engineer's technical responsibilities.

Search shall support:

- assigned personnel/service number;
- first name;
- patronymic;
- surname;
- combined name search where supported.

Selecting a person opens the relevant personnel record and technical authorization information available to the Engineer.

The Engineer uses this area to determine and manage which subordinate Technicians are authorized for the corresponding technical work within the Engineer's authority.

The Engineer shall not be required to search the entire personnel database through an administrative-style interface when performing ordinary technical work.

## 3. DOCUMENTS

`DOCUMENTS` provides the Engineer with document-validity information relevant to UAV technical readiness and operation.

The first-level view shall emphasize state rather than document volume:

```text
UAV / PERSONNEL / DOCUMENT
        │
        ├── VALID       ✓
        ├── EXPIRING    ⚠
        └── EXPIRED     !
```

Detailed document information is opened only when required.

The Engineer should not have to continuously inspect every document. The system should surface only conditions requiring attention and allow direct navigation to the affected record.

## Menu Exclusions

The following shall not be duplicated as first-level Engineer menu items when they already exist as contextual functions:

- equipment management as a separate top-level menu;
- maintenance as a separate top-level menu;
- findings as a separate top-level menu;
- UAV configuration as a separate top-level menu;
- release as a generic menu item;
- system administration;
- pilot functions;
- mission-planning functions unrelated to the Engineer's current technical responsibility.

These functions remain available where context requires them, primarily from the selected UAV, equipment, personnel or document record.

## Work Assignment Context

Technician work assignment is an Engineer responsibility but shall not automatically become a separate first-level menu item.

The Engineer shall be able to assign work from the relevant context, primarily from the selected UAV/equipment or from the appropriate personnel/task context.

```text
FLEET
  ↓
SELECT UAV
  ↓
SELECT EQUIPMENT / TECHNICAL AREA
  ↓
ASSIGN WORK
  ↓
SELECT AUTHORIZED TECHNICIAN
```

This keeps assignment close to the object being serviced and avoids a separate task-management layer on the main screen.

## Technician Result Visibility

When a Technician completes assigned work, BlueSky PRO shall notify the Engineer without requiring an additional approval action for normal completion.

For unfinished work or findings, the Engineer shall see the Technician's status and comment in the relevant technical context.

The Engineer should reach the affected task directly from the notification or attention indicator rather than search for it manually.

## Authority Boundary

The menu structure does not change the underlying authority model:

```text
ADMINISTRATOR
     ↓
assigns ENGINEER role
     ↓
ENGINEER
     ↓
defines TECHNICIAN authorization
     ↓
TECHNICIAN
     ↓
performs assigned work
```

Engineer access remains limited by the Engineer's assigned area of responsibility. Technician access remains limited by the permissions determined by the Engineer.

`Certification / Release Authority` remains an explicit Engineer-level authority and is not implied by the presence of the Engineer menu.

## UX Rules

1. First-level menu contains only Engineer responsibilities.
2. Do not duplicate the same function in several menu locations.
3. Keep the fleet state immediately accessible.
4. Keep frequently used technical information visible in the selected UAV work area.
5. Put detailed and infrequently used information behind contextual tabs/submenus.
6. Use compact state indicators where a full detail panel is unnecessary.
7. Open the relevant detail directly from the affected object or warning.
8. Do not make the Engineer traverse menus simply to complete a normal maintenance workflow.
9. Do not expose Administrator or Pilot functions in the Engineer menu unless explicitly required by the user's additional role and authority.
10. Preserve the overall BlueSky PRO principle of minimal information load during ground preparation.

## Verification Considerations

Verify that:

- only Engineer-relevant first-level functions are presented;
- no repeated first-level functions exist;
- Fleet is the primary Engineer work area;
- Personnel supports search by service number and full name components;
- UAV detail is opened from the fleet list;
- equipment and technical work are accessible contextually;
- two-UAV comparison can operate independently;
- Technician completion notifications do not create unnecessary Engineer approval;
- unfinished Technician work exposes status and comment to the Engineer;
- role and authorization boundaries remain enforced;
- the interface does not become a general-purpose MRO or administration system.

## Relation to Existing Architecture

This document defines navigation baseline only. It complements:

- `ADMINISTRATOR_ENGINEER_TECHNICIAN_AUTHORITY_MODEL_001.md`;
- `FLEET_STATE_ENGINEER_VIEW_001.md`;
- `TECHNICIAN_SHIFT_WORK_ASSIGNMENT_AND_COMPLETION_001.md`;
- `MAINTENANCE_RESULT_TO_FLEET_STATE_001.md`;
- external maintenance integration architecture.

It does not replace those documents and does not create a separate maintenance-management subsystem.
