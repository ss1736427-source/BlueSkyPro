# BlueSky PRO — Role Workspaces and Administrator Branch

**Status:** architecture baseline / planned structure. This document records the agreed direction; it does not claim that every screen or security control is already implemented.

## 1. Application entry sequence

1. Start the application in a locked state.
2. Show the sign-in screen. Do not expose operational workspaces before successful authentication.
3. Authenticate a named user against the configured identity store.
4. Check that the account is active, not locked, and has the required permissions.
5. Create an authenticated session and write an audit event.
6. Resolve the user's authorized role/workspace and open that workspace.
7. Enforce permissions in the service/core layer as well as in the UI. Hiding a button is not authorization.
8. On logout, session expiry, or account disablement, close protected access and return to sign-in.

Failed authentication must not reveal whether a particular login exists. Apply throttling/temporary lockout, record security events, and never log passwords or secret values.

## 2. Account provisioning and authority

### First-run bootstrap
- The first privileged account is created through an explicit, controlled first-run setup—not a hard-coded default username/password.
- Require a unique login and a user-chosen strong password; store only a modern password hash (Argon2id or an appropriately configured equivalent), never plaintext.
- Mark bootstrap complete and prevent unauthorised re-entry into first-run setup.
- Provide a documented, controlled recovery process for loss of the highest-privilege account.

### Ongoing administration
- Each staff member has an individual account; shared staff logins are prohibited.
- An authorized administrator creates, activates, disables, and resets accounts; assigns roles and scoped permissions.
- Grant the least privilege necessary. Separate ordinary user administration from granting critical/system-owner privileges.
- Password reset uses a controlled one-time process; administrators do not learn or set a user's permanent password.
- Account, role, permission, login, logout, failed-login, and security-setting changes are audited with actor, timestamp, target, outcome, and reason where appropriate.
- The exact identity-store implementation (local single-device vs centralized multi-workstation service) is a deployment decision. The authorization model should support either without placing trust decisions in QML.

## 3. Role workspaces

Roles define a default workspace and a baseline permission set. Permissions remain enforceable independently of the selected screen.

### A. System Administrator
Landing workspace: **Administrator Dashboard**.

Branches:
1. **Overview / System Health** — service state, storage/database state where applicable, synchronization, active integration status, recent critical alerts. Display real measured status only; no fabricated counters.
2. **Users, Roles & Access** — accounts, role assignment, permissions, account activation/disablement, reset workflow, sessions and security policy.
3. **External Integrations** — guided connection wizard, provider/channel registry, credential handling, adapter compatibility, connection/data/schema/freshness checks, diagnostics.
4. **System Settings** — application configuration, environment, time and logging settings, operational policies; sensitive changes require permission and audit.
5. **Data & Synchronization** — synchronization jobs, source timestamps, import/export status and errors where supported.
6. **Documents & Compliance** — requirements, verification evidence, controlled documents, configuration/release records.
7. **Audit Log** — searchable security, account, configuration, integration, and administrative events.
8. **Maintenance / Diagnostics** — diagnostic bundles and service diagnostics with secrets redacted; advanced details are collapsed by default.

### B. Engineer
Landing workspace: **Fleet / Engineering**.
- UAV fleet and configuration
- Equipment and payload configuration
- Technical documentation and engineering checks
- Maintenance/engineering status as authorized
- Reports and change history

### C. Technician
Landing workspace: **My Tasks / Maintenance**.
- Assigned maintenance tasks
- Equipment and UAV records relevant to assignments
- Checklists and work completion
- Technical status, faults, and service history
- Reports and task history

### D. Pilot / Flight Operator
Landing workspace: **Flight Operations**.
- Assigned flights/missions and operational briefing
- Flight-planning and map workspace
- UAV selection and permitted control/telemetry functions
- Checklists, flight conditions, alerts, and flight logs
- Flight actions only when the relevant operational/safety gates authorize them

### E. Optional read-only / auditor role
Landing workspace: **Read-only Review**.
- Authorized status, controlled documents, and audit evidence
- No account administration, configuration mutation, or flight control by default

Roles may be combined only through explicit grants. A role label alone must not silently grant every permission in that workspace.

## 4. Administrator navigation tree

```text
Sign-in
└── Authentication + account/session authorization
    └── Authorized workspace
        └── Administrator Dashboard
            ├── Overview / System Health
            ├── Users, Roles & Access
            │   ├── Users
            │   ├── Roles and permissions
            │   ├── Account status / reset
            │   └── Sessions and security policy
            ├── External Integrations
            │   ├── Integration catalogue
            │   ├── Add / configure source (guided wizard)
            │   ├── Connection and data validation
            │   ├── Credentials (protected vault)
            │   └── Diagnostics / test history
            ├── System Settings
            ├── Data & Synchronization
            ├── Documents & Compliance
            ├── Audit Log
            └── Maintenance / Diagnostics
```

## 5. External Integrations — administrator workflow

Keep the default interface minimalist: one current task, one primary action, a short explanation, and a clear result. Advanced diagnostics are available on demand.

1. Select the integration purpose (e.g. NOTAM, WX, maps, terrain, telemetry, insurance).
2. Enter the connection address/endpoint or the appropriate channel-specific endpoint fields.
3. Validate syntax and basic reachability when possible.
4. Ask for authentication only when the selected provider requires it. Store secrets in the protected credential vault; never persist them in repository files or ordinary settings.
5. Select or identify a compatible adapter based on supported transport, authentication, and payload format. Transport and payload format are distinct.
6. Test authentication and obtain a response.
7. Parse and validate the expected schema and required fields.
8. Check freshness, coverage, units, coordinate/vertical reference, and other domain-specific constraints where applicable.
9. Report a truthful result in plain language: success only when the required checks pass; otherwise state the observed failure and one recommended next action.
10. Save/activate only after required validation; record configuration changes and test outcomes in the audit log.

A successful HTTP response alone is not proof of a usable integration. Unknown sources must be reported as unsupported until an adapter is implemented and tested. Do not automatically infer a provider's full schema from an arbitrary URL.

## 6. Integration status vocabulary

- **Not configured** — required connection details are missing.
- **Needs credentials** — provider requires authentication and no usable secret is available.
- **Checking** — a bounded test is in progress.
- **Connected; data validated** — connection, authentication (if required), parsing, and required domain checks passed.
- **Connected; data invalid** — response arrived but schema/content checks failed.
- **Authentication failed** — provider explicitly rejected credentials.
- **Unavailable / timeout** — connection could not be established within the configured timeout.
- **Unsupported source/format** — no compatible adapter exists.
- **Stale / incomplete** — data was received but freshness or coverage requirements were not met.
- **Test only** — source is a simulator, stub, or non-operational demonstration source; never present this as live operational data.

Each failure state should explain what was observed and suggest the next safe action. Do not claim live connection or operational suitability without evidence.

## 7. Common navigation and interaction rules

- Persistent application identity, current user/role, notification indicator, and logout.
- Consistent left navigation for the current workspace; active branch visibly marked.
- Breadcrumb/back navigation to the parent branch and administrator dashboard.
- Minimalist default view; detailed diagnostics and advanced settings are collapsible.
- Never show example counts or green “healthy” states as if they were live telemetry.
- On permission denial, explain that the action is not authorized; do not silently proceed.
- Changing role/workspace requires server/core authorization and must not bypass the login/session flow.

## 8. Implementation sequence

1. Inventory current QML navigation and identify the existing role-selection preview.
2. Design and implement authentication/session boundary and first-run bootstrap.
3. Implement user/role/permission data model, audit events, and enforcement tests.
4. Make the administrator dashboard the landing page for an authenticated administrator.
5. Add administrator navigation branches with clear page boundaries.
6. Implement the External Integrations branch first: catalogue and guided wizard, then migrate existing NOTAM/WX/terrain/MapTiler test controls without losing working tests.
7. Add real health/status data and audit integration changes.
8. Implement engineer, technician, and pilot workspaces after the administrator branch is stable.

## 9. Current implementation caveat

This file is the agreed architecture and work sequence. It does not certify that authentication, user management, authorization enforcement, or every listed administrator page already exists. Existing role-selection UI must not be treated as security. Verify repository implementation and CI evidence as each step is delivered.

## 10. Acceptance criteria for this architecture

- Unauthenticated users cannot enter any protected workspace or invoke protected actions.
- Unknown, disabled, locked, or invalidly authenticated accounts are denied.
- Every user has an individual account and role/permissions are enforced outside presentation-only QML logic.
- The first privileged account is provisioned without a hard-coded default password.
- Passwords and API secrets never appear in plaintext logs, source control, or ordinary configuration.
- Role/workspace navigation is explicit and unauthorized actions are denied.
- Integration success/failure messages reflect actual connection, authentication, parsing, and applicable data validation results.
- Important authentication, access-control, account, role, and integration configuration events are auditable.
