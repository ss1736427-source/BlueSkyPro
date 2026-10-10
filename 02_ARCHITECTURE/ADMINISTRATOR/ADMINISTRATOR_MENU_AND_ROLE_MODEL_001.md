---
id: ADMINISTRATOR-MENU-ROLE-001
type: architecture
status: BASELINE
version: 1.0
title: Administrator Menu and Multi-Role User Model
---

# ADMINISTRATOR — MENU AND MULTI-ROLE USER MODEL

## 1. Design principle

ADMINISTRATOR retains the complete functional model required by the system, while the user interface presents only functions permitted to the current user and selected for the user's working profile.

Functional completeness and visual simplicity are separate concerns.

## 2. First-level menu

The first level shall remain minimal:

```text
ADMINISTRATOR
│
├── SYSTEM ADMINISTRATION
├── TECHNICAL
├── EXTERNAL DATA & CONNECTIONS
└── DATA & PROCESSING
```

No required system capability is removed because it is not shown on the first level.

## 3. Second-level expansion

Each first-level block expands only when selected.

### SYSTEM ADMINISTRATION

```text
Users & Roles
Permissions
System Configuration
Organizations
Configuration / Feature Flags
Versions & Change Control
Audit
Security
Backup / Recovery
Deployment / Environment
```

### TECHNICAL

```text
UAV
Energy / Batteries
Equipment / Payload
Configurations
Compatibility
Maintenance / Service
Software / Firmware
Technical Documents
Technical Validation / Readiness
```

### EXTERNAL DATA & CONNECTIONS

```text
Navigation Data
Aeronautical Information
NOTAM
Weather / METEO
Airspace
Operating / Landing Sites
Maps
Terrain
Geofence
C2
Communications
Observation / Surveillance
External Services
Synchronization / DUM
Connection Health
```

### DATA & PROCESSING

```text
Datasets
Collection
Normalization
Validation
Fusion / Aggregation
Data Lineage
Formats
External Processing
Export
Import / Results
Archive
Data Quality
```

## 4. Data and service boundary

Administrator UI is a control surface. Processing itself remains in the relevant system service.

```text
ADMINISTRATOR
      ↓
configuration / policy / source / permissions
      ↓
DUM / HUB / DOMAIN SERVICES
      ↓
normalized authoritative data
      ↓
Mission / Planning / Safety / Analysis
```

External processing follows:

```text
DATASET
  ↓
DATA MANAGER
  ↓
DATA EXCHANGE CONTRACT
  ↓
ADAPTER
  ↓
EXTERNAL SOFTWARE
  ↓
RESULT PACKAGE
  ↓
DATA MANAGER / HUB
```

## 5. Multi-role user model

A user shall have one identity and may have multiple roles and functional assignments.

Example:

```text
USER
 ├── primary role: PILOT
 ├── additional role: TECHNICIAN
 └── additional role: ENGINEER
```

Roles are not mutually exclusive.

## 6. Role composition

The effective access model is:

```text
USER IDENTITY
      ↓
ROLES
      ↓
FUNCTIONS / PERMISSIONS
      ↓
TECHNICAL AUTHORIZATION WHERE APPLICABLE
      ↓
VISIBILITY
      ↓
PERSONAL WORKSPACE
```

A role grants a controlled set of capabilities; visibility customization cannot grant permissions.

## 7. Technical authority model

The technical hierarchy is fixed as follows:

```text
SYSTEM ADMINISTRATOR
        ↓
     ENGINEER
        ↓
TECHNICIAN
```

The Administrator assigns the base role `ENGINEER` or `TECHNICIAN` to the user. ENGINEER has access to the entire UAV fleet and technical data required for engineering work. ENGINEER determines the technical authorization of subordinate TECHNICIAN personnel.

The TECHNICIAN works only within the authorization established by ENGINEER.

## 8. Example combined specialist

```text
Ivanov
│
├── PILOT
│   ├── mission preparation
│   ├── preflight functions
│   └── flight functions
│
├── TECHNICIAN
│   ├── UAV condition
│   ├── batteries
│   ├── maintenance
│   └── faults within authorization
│
└── ENGINEER
    ├── entire UAV fleet
    ├── technical data
    ├── configurations
    ├── compatibility
    ├── equipment
    ├── technical parameters
    ├── maintenance status
    └── technician authorizations
```

The user may expose only the functions required for the current work profile.

## 9. Permission granularity

Permissions shall distinguish at least:

```text
VIEW
CREATE
EDIT
VALIDATE
APPROVE
ACTIVATE
SUSPEND
ARCHIVE
EXPORT
IMPORT
ADMINISTER
```

A role or function shall not imply all permissions automatically.

Critical permissions such as approval, activation and operational authorization shall remain separately controlled.

`Certification / Release Authority` is an ENGINEER-level authority and remains separately controlled; assignment of the ENGINEER role does not by itself imply that authority.

## 10. Personalized menu

A user may configure the presentation of functions available to that user.

```text
AVAILABLE FUNCTIONS
        ↓
USER SELECTION / PROFILE
        ↓
MY MENU
```

The personal menu may:

- pin frequently used functions;
- hide non-frequent permitted functions;
- reorder permitted functions;
- save workspace arrangement;
- restore system default.

Hidden functionality remains accessible through the full authorized menu or search/navigation mechanism.

## 11. Required functions vs visibility

The system shall maintain two independent properties:

```text
FUNCTION EXISTS      → system capability
USER MAY USE         → authorization
USER WANTS TO SEE    → personalization
```

Thus:

```text
NO PERMISSION → NOT AVAILABLE
PERMISSION + HIDDEN → AVAILABLE BUT NOT SHOWN IN PERSONAL MENU
PERMISSION + SHOWN → SHOWN
```

## 12. Role overlap and separation

Several users may have the same role. One user may have multiple roles.

Separate approval authority shall not be created merely by assigning an additional technical role.

Where required by safety, certification or organizational policy, approval may require a distinct authority role or organizational rule.

## 13. Technical block visibility

Technical information shall be filtered according to user permissions and purpose.

For example:

```text
TECHNICIAN
→ condition / service / battery / faults within engineer-defined authorization

ENGINEER
→ entire fleet / technical data / parameters / configuration / compatibility / technical model

PILOT
→ operational readiness / applicable configuration / relevant status
```

A combined user may receive the union of authorized functions while still using a compact personal menu.

## 14. External data visibility

The full external-data catalogue shall remain available to authorized administrators, while each operational user sees only relevant permitted domains.

Examples:

```text
Administrator → all administrative domains
Engineer      → engineering and technical domains required for work
Technician    → assigned technical/service scope
Pilot         → operationally relevant data
```

## 15. Audit

Role assignment, permission changes, profile changes affecting protected functions, configuration changes and administrative actions shall be auditable.

The audit record shall retain actor, action, target, time, resulting state and applicable revision/context.

## 16. Security boundary

The UI shall never be the authority for access control.

```text
QML/UI
  ↓ request
C++ Core / Authorization Service
  ↓ decision
Protected resource / operation
```

Personalization can reduce displayed information, but cannot bypass authorization.

## 17. Relationship to existing architecture

This model connects:

```text
ADMINISTRATOR
   ├── Users / Roles / Permissions
   ├── Technical Configuration
   ├── External Data & Connections
   └── Data & Processing
          ↓
       DUM / HUB
          ↓
   DOMAIN SERVICES
          ↓
 Mission / Pilot / Safety / Analysis
```

It complements the existing DUM, HUB, Data Governance, Configuration Manager, External Processing and traceability architecture without creating a parallel data authority.

## 18. Status

**BASELINE.** The user-centric role model and technical authority hierarchy are established. Detailed capability matrices and certification-specific approval rules remain subject to the applicable requirements baseline.
