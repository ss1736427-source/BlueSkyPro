# ADMINISTRATOR — USER ROLE ASSIGNMENT UI

**Document ID:** ADMINISTRATOR_USER_ROLE_ASSIGNMENT_UI_001  
**Status:** BASELINE  
**Scope:** Administrator block / user and role assignment interface

## 1. Purpose

Define the simplest administrator workflow for assigning operational roles to a named user without exposing unnecessary internal authorization complexity.

## 2. Core principle

```text
USERS
  ↓
SELECT USER
  ↓
USER CARD
  ↓
ROLES
  ↓
SAVE
  ↓
SYSTEM BUILDS ACCESS
```

The Administrator assigns roles. The system derives the resulting access and available functions.

## 3. Main screen

```text
┌────────────────────────────────────────────────────┐
│ ADMINISTRATOR                                      │
├────────────────────────────────────────────────────┤
│ USERS                                              │
│                                                    │
│ 🔎 Search user                                     │
│                                                    │
│ Сергей Иванов                                      │
│ Иван Петров                                        │
│ Алексей Смирнов                                    │
└────────────────────────────────────────────────────┘
```

The first-level view contains only the information needed to find and select a user.

## 4. User card

```text
┌────────────────────────────────────────────────────┐
│ SERGEY IVANOV                                      │
├────────────────────────────────────────────────────┤
│                                                    │
│ ROLES                                              │
│                                                    │
│ ☑ PILOT                                            │
│ ☑ TECHNICIAN                                       │
│ ☑ ENGINEER                                         │
│ ☐ SYSTEM ADMINISTRATOR                             │
│                                                    │
│                                  [ SAVE ]          │
└────────────────────────────────────────────────────┘
```

Role assignment is the primary administrator action.

## 5. Role combination

One person may have several roles simultaneously.

```text
USER
 ├── PILOT
 ├── TECHNICIAN
 └── ENGINEER
       ↓
 EFFECTIVE ACCESS
```

No duplicate account is required solely because operational functions are combined.

## 6. Derived access

After roles are assigned, the system derives the user's available functions.

```text
ROLES
  ↓
SYSTEM RULES
  ↓
AVAILABLE FUNCTIONS
  ↓
PERSONAL MENU
```

The Administrator does not normally need to configure every individual capability manually.

## 7. Optional advanced view

Detailed capability and scope controls remain available only when required by the organization or when the Administrator opens the advanced configuration.

```text
ROLES
   ↓
[ Advanced ]
   ↓
Capabilities / Scope / Restrictions
```

The advanced layer must not overload the normal user-management screen.

## 8. User interface rules

- Search and select by user identity.
- Show current roles immediately.
- Use simple enable/disable role controls.
- Make multi-role assignment explicit.
- Keep advanced authorization settings behind an additional level.
- Do not expose technical authorization internals during ordinary role assignment.
- Do not allow UI configuration to bypass authorization.

## 9. Result

The administrator's normal workflow is intentionally short:

```text
USER → ROLES → SAVE
```

The system performs the remaining authorization composition automatically.

## 10. Relationship to the Administrator architecture

```text
ADMINISTRATOR
│
├── USERS & ROLES
│     └── USER ROLE ASSIGNMENT UI
│            ├── Users
│            ├── User Card
│            ├── Roles
│            └── Advanced Access
│
├── UAV & CONFIGURATION
├── DATA & PROCESSING
├── EXTERNAL CONNECTIONS
└── LOG / AUDIT
```

## 11. Next decomposition

Next define the role catalog itself and the system mapping from each role to the relevant Administrator and operational functions, while preserving the simple `USER → ROLES → SAVE` workflow.
