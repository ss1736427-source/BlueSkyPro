---
id: ADMIN-ARCH-DATA-INTEGRATION-001
type: architecture
status: BASELINE
version: 1.1
---

# ADMINISTRATOR — DATA & EXTERNAL INTEGRATION ACCESS MODEL

## 1. Purpose

Define the administrator-facing control path for collected data, data preparation, external processing integrations, and controlled return of results into BlueSky PRO.

The model separates **capability**, **permission**, and **personal display configuration** so that the complete system capability remains available without forcing every user to see every function.

## 2. Administrator first level

The first level of ADMINISTRATOR is intentionally limited to user-relevant domains:

```text
ADMINISTRATOR
│
├── USERS & ROLES
├── UAV & CONFIGURATION
├── NAVIGATION DATA
├── METEO
├── COMMUNICATIONS
├── EXTERNAL CONNECTIONS
├── DATA
└── SYSTEM
```

## 3. Data branch

```text
DATA
│
├── Collected Data
│   ├── Mission datasets
│   ├── Images / video
│   ├── Telemetry
│   ├── Navigation data
│   └── Metadata
│
├── Data Preparation
│   ├── Validate
│   ├── Normalize
│   ├── Merge
│   ├── Format
│   └── Version
│
├── Export
│   ├── Select dataset
│   ├── Processing profile
│   ├── Output format
│   ├── Destination
│   └── Transfer status
│
├── External Processing
│   ├── Available software
│   ├── Compatible profiles
│   ├── Adapters
│   └── Processing jobs
│
└── Results
    ├── Imported results
    ├── Link to source dataset
    ├── Quality / processing report
    └── Publication / further export
```

## 4. External connections branch

```text
EXTERNAL CONNECTIONS
│
├── Connection Catalog
├── Adapter Catalog
├── Interface Profiles
├── Authentication / Credentials
├── Availability
├── Compatibility
├── Test / Verification Status
└── Exchange Journal
```

Credentials are controlled separately from operational data and are never part of a mission dataset export package.

## 5. User access model

```text
SYSTEM CAPABILITY
       │
       ▼
ROLE / FUNCTION
       │
       ▼
USER ASSIGNMENT
       │
       ▼
TECHNICAL AUTHORIZATION WHERE APPLICABLE
       │
       ▼
PERSONAL WORKSPACE
       │
       ▼
VISIBLE MENU
```

A single person may have several functions, for example:

```text
USER
├── Administrator
├── Technician
├── Engineer
└── Pilot
```

The Administrator assigns the basic roles. For technical work, ENGINEER has access to the entire UAV fleet and required technical data and determines the technical authorization of subordinate TECHNICIAN personnel.

## 6. Visibility principle

Rules, mandatory controls, and safety-critical information remain part of the system even when not displayed in the user's normal workspace.

Visibility is configurable:

```text
FULL CAPABILITY
      ↓
AUTHORIZED FUNCTIONS
      ↓
USER DISPLAY PROFILE
      ↓
PERSONAL MENU
```

Hidden does not mean disabled. Access control is determined by authorization, not by visual hiding.

## 7. External processing user flow

### Automatic

```text
USER TASK
   ↓
SYSTEM IDENTIFIES REQUIRED DATA
   ↓
COMPATIBILITY CHECK
   ↓
ONLY COMPATIBLE PROCESSORS SHOWN
   ↓
DATA PACKAGE GENERATED
   ↓
ADAPTER / INTERFACE
   ↓
EXTERNAL PROCESSING
   ↓
RESULT PACKAGE
   ↓
DATA MANAGER
   ↓
MISSION / DATASET LINK
```

### Custom

Authorized engineering/technical users may manually define an external processing profile where automatic selection is insufficient.

```text
CUSTOM
├── Dataset
├── Components
├── Formats
├── Metadata
├── CRS
├── External application
├── Interface
└── Output
```

Custom configuration must remain subject to compatibility, authorization, and audit rules.

## 8. Compatibility principle

The interface must not present incompatible choices as selectable operational options.

```text
ALL COMPONENTS / CONNECTIONS
          ↓
COMPATIBILITY ENGINE
          ↓
┌───────────────┬───────────────┐
│ COMPATIBLE    │ INCOMPATIBLE  │
│ AVAILABLE     │ HIDDEN        │
└───────────────┴───────────────┘
```

For Custom mode, an explicit engineering override may expose non-standard combinations, but the system must identify the deviation, require appropriate authorization, and preserve the configuration record.

## 9. Integration with existing architecture

```text
ADMINISTRATOR
      │
      ├───────────────► UAV CONFIGURATION
      │
      ├───────────────► USERS / ROLES
      │
      ├───────────────► NAVIGATION / METEO / COMMS
      │
      └───────────────► DATA
                           │
                           ▼
                         DUM
                           │
                           ▼
                         HUB
                           │
                           ▼
                DATA EXCHANGE CONTRACT
                           │
                           ▼
                        ADAPTER
                           │
                           ▼
                 EXTERNAL PROCESSING
                           │
                           ▼
                       RESULTS
```

## 10. Separation of concerns

- **ADMINISTRATOR** — defines users, basic roles, system-level access controls, configurations, connections and policy-controlled visibility.
- **ENGINEER** — has the engineering technical view of the fleet and determines technical authorizations for subordinate TECHNICIAN personnel.
- **TECHNICIAN** — operates only within the technical authorization assigned by ENGINEER.
- **DUM / Data Manager** — manages data lifecycle and data preparation.
- **HUB** — provides the operational aggregation/communication point for applicable data flows.
- **Integration layer** — provides controlled exchange with external systems.
- **External software** — performs specialized processing outside the BlueSky PRO core.
- **Traceability / Audit** — records relevant operations and relationships.

## 11. Design decision

ADMINISTRATOR shall be the controlled entry point for management of users, basic roles, system capabilities and external integrations, while the user's working interface shall expose only the functions authorized and configured for that user.

For the technical hierarchy, the Administrator assigns the ENGINEER role; ENGINEER has access to the entire fleet and technical data and determines the TECHNICIAN technical authorization. This technical authorization model is maintained separately from personal menu presentation.

The next implementation layer is therefore not another top-level menu item, but the deep functional chain behind each selected domain.
