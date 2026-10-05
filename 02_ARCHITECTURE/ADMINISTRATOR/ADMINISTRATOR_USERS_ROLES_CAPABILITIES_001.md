# ADMINISTRATOR — USERS, ROLES & CAPABILITIES

**Document ID:** ADMINISTRATOR_USERS_ROLES_CAPABILITIES_001  
**Status:** BASELINE  
**Scope:** Administrator block / user-based role assignment and personal workspace

## 1. Purpose

Define a simple and extensible model in which the Administrator selects a user and assigns the basic roles required for that person. One authenticated person may combine several roles.

## 2. Core principle

```text
ADMINISTRATOR
      ↓
   USERNAME
      ↓
 ASSIGN ROLES
      ↓
 SYSTEM BUILDS BASE ACCESS
      ↓
 PERSONAL MENU
```

The Administrator works with the **user as the primary object**. Roles are assigned directly to the selected user.

## 3. User roles

The Administrator shall be able to assign one or several basic roles to the same user:

- SYSTEM ADMINISTRATOR — system-wide administration;
- TECHNICIAN — technical maintenance and service within the engineer-defined technical authorization;
- ENGINEER — engineering configuration, diagnostics, technical analysis and control of the technical authorization of subordinate technicians;
- PILOT / OPERATOR — flight preparation, execution and operational data.

Example:

```text
USER: SERGEY

Roles:
☑ PILOT
☑ TECHNICIAN
☑ ENGINEER
☐ SYSTEM ADMINISTRATOR
```

No duplicate account is required solely because functions are combined.

## 4. Administrator workflow

```text
Users
  ↓
Search / select user
  ↓
User card
  ↓
Roles
  ↓
Save
```

The Administrator assigns the basic roles. The system derives the resulting base access.

## 5. Role combinations

A role represents a functional area assigned to the user. Multiple roles can coexist on one user account.

```text
ONE USER
│
├── PILOT
├── TECHNICIAN
└── ENGINEER
```

The effective capabilities of the user are the combined result of assigned roles, applicable organizational rules and technical authorization boundaries.

## 6. Technical authorization boundary

The technical hierarchy is deliberately different from ordinary role assignment:

```text
ADMINISTRATOR
      ↓
assigns ENGINEER role
      ↓
ENGINEER
      ↓
full technical access to the fleet and technical data
      ↓
defines subordinate TECHNICIAN authorizations
      ↓
TECHNICIAN
      ↓
only authorized UAV / equipment / configuration / work
```

The Administrator does not manually distribute every maintenance operation between technicians when that decision belongs to the ENGINEER.

## 7. Engineer

ENGINEER has access to the entire UAV fleet and the technical data required for engineering work within the system's organizational boundary.

ENGINEER controls the technical authorization of subordinate TECHNICIAN personnel by defining, as applicable:

- UAV or UAV group;
- equipment;
- configuration;
- type of technical work;
- authorization level;
- validity or other organizational constraints.

## 8. Technician

TECHNICIAN may perform only operations covered by the technical authorization established by ENGINEER.

The existence of the `TECHNICIAN` role does not itself grant access to the whole technical fleet or all technical operations.

## 9. Personal menu

After authorization is established, the user can configure how permitted functions are displayed.

```text
USER
 ↓
ASSIGNED ROLES
 ↓
AUTHORIZED FUNCTIONS
 ↓
PERSONAL MENU
```

The personal menu controls presentation and convenience, not authorization.

Mandatory safety, security, compliance and operational controls remain visible whenever required by policy.

## 10. Administrator responsibilities

The Administrator manages the user's:

- identity / username;
- assigned basic roles;
- applicable system-level access controls;
- security-sensitive permissions;
- personal menu defaults;
- audit visibility where authorized.

Technical task-level authorization for TECHNICIAN is assigned by ENGINEER according to the established authority model.

## 11. Context for multi-role users

If a user has several roles, the interface can provide a simple context selector only when necessary.

```text
SERGEY
[PILOT ▾]
```

Selecting another assigned function changes the working view to the corresponding tools. It does not create another account or identity.

## 12. Safety and security

- UI visibility never grants authorization.
- A user can access only functions permitted by assigned roles and applicable authorization rules.
- Critical actions require explicit authorization according to policy.
- Changes to roles and security-sensitive access are auditable.
- Personal menu settings cannot bypass authorization.
- ENGINEER authorization of TECHNICIAN does not transfer ENGINEER-only authorities such as Certification / Release Authority unless separately granted.

## 13. Relationship to other Administrator branches

```text
ADMINISTRATOR
│
├── USERS
│     └── User → Roles → Base Access → Personal Menu
│
├── UAV & CONFIGURATION
│     └── fleet / equipment / configurations
│
├── DATA & PROCESSING
│     └── datasets / preparation / export / results
│
├── EXTERNAL CONNECTIONS
│     └── data and processing integrations
│
└── LOG / AUDIT
      └── administrative actions
```

## 14. Interface principle

The Administrator's daily workflow should be direct:

```text
USERS
 ↓
[SEARCH / SELECT USER]
 ↓
USER CARD
 ↓
[ROLES]
 ↓
[SAVE]
```

The system hides unnecessary internal complexity. Advanced configuration remains available only where required.

## 15. Status

**BASELINE.** The user-centric role assignment model is established. Detailed capability matrices and technical authorization rules are maintained in the corresponding Administrator authority documents.
