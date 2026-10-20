# Administrator Branch — Repository-Wide Coverage Audit

**Date:** 2026-10-20  
**Branch:** `main`  
**Purpose:** Cross-check the current Administrator workspace/navigation against the existing repository architecture, requirements, integration baseline, and implementation.

## 1. Audit scope and method

The complete Git tree for `main` was enumerated (1,778 tracked files; GitHub recursive tree was not truncated). The review then inspected the Administrator architecture baselines, role catalogue and assignment UI, configuration storage schema, external integration coverage, the integration-console/Virtual Flight implementation checkpoint, and current HMI files.

This is a focused requirements-to-navigation audit, not a claim that all 1,778 file contents were individually read or that every capability has been implemented.

Key sources reviewed:
- `02_ARCHITECTURE/ADMINISTRATOR/ADMINISTRATOR_MENU_STRUCTURE_001.md`
- `02_ARCHITECTURE/ADMINISTRATOR/ADMINISTRATOR_MENU_AND_ROLE_MODEL_001.md`
- `02_ARCHITECTURE/ADMINISTRATOR/ADMINISTRATOR_ROLE_CATALOG_001.md`
- `02_ARCHITECTURE/ADMINISTRATOR/ADMINISTRATOR_USERS_ROLES_CAPABILITIES_001.md`
- `02_ARCHITECTURE/ADMINISTRATOR/ADMINISTRATOR_USER_ROLE_ASSIGNMENT_UI_001.md`
- `02_ARCHITECTURE/ADMINISTRATOR/ADMINISTRATOR_DATA_INTEGRATION_ACCESS_MODEL_001.md`
- `02_SYSTEM_DESIGN/ADMINISTRATOR/ADMINISTRATOR_CONFIGURATION_STORAGE_SCHEMA_001.md`
- `01_REQUIREMENTS/INTEROPERABILITY/EXTERNAL_INTEGRATION_COVERAGE_001.md`
- `04_SOFTWARE/OPERATIONS/EXTERNAL_INTEGRATION_CONSOLE_AND_VIRTUAL_FLIGHT_FIXTURES_001.md`
- `qt/BlueSkyPRO-HMI/qml/MainContent.qml`
- `qt/BlueSkyPRO-HMI/qml/IntegrationConsole.qml`
- `qt/BlueSkyPRO-HMI/qml/AdminWorkspace.qml`
- `qt/BlueSkyPRO-HMI/src/CredentialVault.h` and `CredentialVault.cpp`

## 2. Finding: the current left menu is incomplete

The current `AdminWorkspace.qml` has these items: Overview, Users, Roles, Security, Settings, Integrations, Sync, Documents, Audit, Diagnostics.

This is a useful first shell but it does **not** preserve the full Administrator capability model already documented in the repository. In particular, the menu currently lacks explicit navigation to:
- UAV and technical configuration management;
- equipment, payloads, batteries/energy, communications and navigation components;
- configuration catalogue, automatic/custom configurations, compatibility and validation/approval;
- organization scope, feature flags, versions/change control, deployment/environment, backup/recovery and workspace/localization settings;
- the full external connection/adapter/interface-profile catalogue and its credentials, compatibility, availability, test status and exchange journal;
- individual external-data domains (navigation/aeronautical information, NOTAM, WX/METEO, airspace, operating/landing sites, maps, terrain, geofence, C2/communications, surveillance/traffic, external services, synchronization/DUM and connection health);
- data lifecycle: datasets, collection, telemetry/images/video/metadata, validation, normalization, merge/fusion, lineage, versioning, formats, external processing jobs, import/export, results, archive and data quality;
- technical documents/readiness and maintenance/service/firmware areas within the administrator's technical configuration branch.

Some of these are subfunctions and should not all be top-level buttons. The current UI must group them into expandable branches so the left column remains compact while no documented capability is lost.

## 3. Canonical Administrator navigation to preserve

The repository's existing baselines use slightly different summaries but agree on the underlying capabilities. Reconcile them into a single UI tree:

### A. System administration
- Overview / system status
- Users and user profiles
- Roles and multi-role assignments
- Permissions and security
- Organizations and access scope
- System configuration and feature flags
- Workspaces and localization
- Versions, updates and change control
- Deployment / environment
- Backup / recovery
- Audit and security events

### B. UAV and technical configuration
- UAV model catalogue: reference and custom
- Equipment: batteries/energy, payloads, cameras, navigation, communications
- Configurations: automatic and custom/manual
- Compatibility: compatible, restricted, incompatible
- Configuration validation and approval state
- Maintenance/service and technical condition
- Software/firmware
- Technical documents
- Technical validation/readiness
- Engineer-controlled technician authorizations
- Certification / Release Authority as a separately controlled permission, not implied by ENGINEER role alone

### C. External data and connections
- Connection catalogue
- Adapter catalogue
- Interface/connection profiles
- Authentication and credentials (protected vault; no plaintext)
- Connection availability and health
- Compatibility and test/verification status
- Exchange journal
- Navigation data / aeronautical information
- NOTAM / AIP
- Weather / METEO
- Airspace / restrictions / geofence
- Operating and landing sites
- Maps / GIS providers
- Terrain / DEM and obstacles
- Traffic / UTM / observation and surveillance
- C2 and communications
- ATM / flight-plan exchange and external authority services
- GNSS / RTK / PPK
- UAV / FCS / autopilot / MAVLink
- Payload / camera / video
- Insurance
- Flight records / logs
- Alerts / operational events
- Synchronization / DUM and external APIs

The catalogue is a statement of system domains, not a claim that all providers are currently connected. Each entry must show its actual implementation and verification status.

### D. Data and processing
- Datasets and mission data
- Collection: imagery/video, telemetry, navigation data, metadata
- Data validation and quality
- Normalization and formatting
- Merge / fusion / aggregation
- Data lineage and versioning
- Export: dataset, processing profile, output format, destination, transfer status
- External processing: compatible software, adapters, jobs
- Import and returned results
- Result-to-source dataset linkage and quality report
- Archive

### E. Logs and audit
- System events
- User actions
- Configuration and permission changes
- Data exchange
- External processing
- Audit records

This may be shown as a distinct first-level branch or nested under System Administration, according to the final compact-navigation decision. It must remain reachable.

## 4. Role and authorization rules that the UI must preserve

- A user has one identity and may have multiple roles; roles are not mutually exclusive.
- SYSTEM ADMINISTRATOR assigns base roles and system access.
- ENGINEER has access to the fleet/technical data in the engineering scope and defines the technical authorization of subordinate TECHNICIAN personnel.
- TECHNICIAN works only with the UAV, equipment, configuration and work types authorized by ENGINEER.
- PILOT sees flight-planning/operations functions permitted for the user.
- Certification / Release Authority is a separate controlled permission; assigning ENGINEER does not automatically grant it.
- At minimum, distinguish VIEW, CREATE, EDIT, VALIDATE, APPROVE, ACTIVATE, SUSPEND, ARCHIVE, EXPORT, IMPORT and ADMINISTER.
- Menu visibility/personalization is not an authorization boundary. Protected actions must be enforced by the relevant service/core.
- Each important role, permission, account, configuration and integration change must be auditable.

## 5. Integration behavior and current implementation truth

Current source-level checkpoint in `EXTERNAL_INTEGRATION_CONSOLE_AND_VIRTUAL_FLIGHT_FIXTURES_001.md` states:
- Live Open-Meteo weather and point-elevation HTTP queries exist as prototype connectivity probes.
- The Russian CAI NOTAM page is linked, but an authorized machine-readable NOTAM feed is not connected.
- MapTiler is part of the HMI integration catalogue and uses the existing credential path.
- The integration console is not yet a complete generic provider/adapter configuration system.
- Virtual Flight fixture data is synthetic and must not be presented as verified historical observations or live operational data.
- Current successful HTTP transport is not sufficient evidence that a provider's data is valid for operational planning.

The universal integration branch should therefore support a guided wizard but must report real state only: configured, needs credentials, checking, connected/data validated, authentication failed, unavailable/timeout, invalid response, unsupported format, stale/incomplete, or test-only.

## 6. Storage/configuration concerns

The existing CredentialVault is a protected secret-storage component with a fixed allowlist of provider IDs (`notam`, `icao`, `maptiler`, `weather`). The current integration console uses these IDs. Preserve backward compatibility if extending the registry. Provider endpoints and non-secret settings should not be conflated with secret values; credentials must not be written to source control or plaintext settings.

The existing configuration storage baseline also requires revision-controlled technical catalogues, source provenance, explicit compatibility/validation results, and a separate approval state. The UI must not replace those data/service rules with local QML-only decisions.

## 7. Required correction before claiming navigation completeness

1. Treat the current menu as an incomplete shell, not a complete Administrator branch.
2. Expand the sidebar into compact expandable groups (A–E above), preserving readable workspace on the right.
3. Ensure every submenu routes to a named workspace/page, even if some pages initially carry an explicit “not implemented yet” state.
4. Do not fabricate counts, health indicators or “connected” states.
5. Keep integration credentials protected and keep Virtual Flight data visibly separated from live sources.
6. Add a navigation coverage checklist linking each baseline branch to its UI route and implementation status.
7. After changes, run Qt HMI build and relevant tests; do not mark completion until CI results for the exact commit are known.

## 8. Conclusion

The repository contains a substantially broader Administrator model than the recently added sidebar exposed. The most important omissions are **UAV & Configuration**, **Data & Processing**, and the full **External Data & Connections** catalogue, plus system configuration/deployment/recovery and the detailed permission/technical-authorization model.

This audit records the missing coverage explicitly so the next UI iteration can reconcile the implementation with the established repository baseline rather than replacing it with a simplified, incomplete interpretation.
