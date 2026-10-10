# BlueSky PRO — New Formalized Requirements
## Consolidated from the latest project proposals

### REQ-MISSION-ECON-001 — Mission objective profiles
The system shall support SAFE, FAST, ECONOMIC and HIGH QUALITY mission optimization profiles.

### REQ-MISSION-ECON-002 — Objective traceability
The selected mission objective shall be recorded and associated with the generated mission and optimization result.

### REQ-SIM-001 — Mission Simulator
The system shall provide a dedicated Mission Simulation/Sandbox tab for virtual mission execution.

### REQ-SIM-002 — Full operational simulation
The simulator shall use the system's operational capabilities and logic as in real mission execution, within a virtual environment.

### REQ-SIM-003 — New-user familiarization
The simulator shall support an introductory virtual flight for new users as a safe familiarization/training experience.

### REQ-SIM-004 — Digital Twin
The simulator shall support a digital-twin representation sufficient to evaluate mission behavior before real execution.

### REQ-SIM-005 — Pre-execution lifecycle
The system shall support `SIMULATION -> EVALUATE -> OPTIMIZE -> APPROVE -> EXECUTE`.

### REQ-SIM-006 — Scenario coverage
Simulation shall support route, battery/energy, communication, detection, DAA, multi-UAV, weather and emergency scenarios.

### REQ-KNOW-001 — Mission Memory context
The system shall store contextual operational knowledge, not only mission records.

### REQ-KNOW-002 — Contextual retrieval
Relevant historical knowledge shall be surfaced automatically to operators when it applies to a new mission.

### REQ-KNOW-003 — Data-to-knowledge processing
Collected operational data shall be processed for system, algorithm and map-layer improvement.

### REQ-KNOW-004 — Cumulative learning
Useful new information shall be accumulated for future autonomous and degraded-connectivity operation.

### REQ-KNOW-005 — Update packages
The system shall support controlled preparation of system update packages based on accumulated knowledge.

### REQ-KNOW-006 — Annual update cycle
The project shall support an annual controlled update cycle including testing and approval before operational use.

### REQ-KNOW-007 — Learning report
Each update package shall produce a consolidated report describing what the system learned and what changed.

### REQ-UAV-ADAPT-001 — UAV capability discovery
The system shall interrogate connected UAV systems, where supported, to determine available capabilities and software/firmware state.

### REQ-UAV-ADAPT-002 — Capability adaptation
The system shall be capable of adapting applicable algorithms and mission planning to newly available UAV capabilities through controlled updates.

### REQ-REJ-001 — Automatic FPL generation
The system shall generate an FPL from the approved mission according to the configured regulatory profile.

### REQ-REJ-002 — Automatic FPL submission
The system shall submit the generated FPL through a configured communication channel.

### REQ-REJ-003 — REJ reception
The system shall receive, store and associate REJ responses with the exact submitted FPL version.

### REQ-REJ-004 — REJ analysis
The system shall analyze REJ information and identify affected FPL fields/parameters.

### REQ-REJ-005 — Automatic correction
Where an unambiguous correction rule exists, the system shall generate a corrected FPL automatically.

### REQ-REJ-006 — Correction transparency
Before/after values and the reason for every automatic correction shall be displayed to the operator.

### REQ-REJ-007 — Automatic resubmission
Where configured and permitted, the system shall automatically resubmit the corrected FPL.

### REQ-REJ-008 — Human control
The operator shall be able to stop, approve or reject an automatic correction/resubmission cycle.

### REQ-REJ-009 — REJ loop protection
The system shall limit repeated automatic REJ/correction cycles and prevent an infinite loop.

### REQ-REJ-010 — Flight-plan audit trail
All FPL versions, submissions, ACK/REJ responses and corrections shall be retained as an auditable chain.

### REQ-MAP-001 — Operational instrument
The map shall use the same visual language as the HMI and shall function as an operational instrument.

### REQ-MAP-002 — Interactive map modes
The map shall support 2D, 3D, satellite, zoom, pan, rotate, tilt and search.

### REQ-MAP-003 — Provider independence
The map provider shall not be a critical dependency of the Mission/HMI/Flight/Safety core.

### REQ-MAP-004 — Provider adapter
The architecture shall support multiple map providers and local data through a provider adapter.

### REQ-MAP-005 — Local map data
The system shall retain locally available map data for use after initial loading and during degraded connectivity.

### REQ-MAP-006 — Operational layer
The map shall display route, UAV, waypoint, boundaries, zones, NOTAM, weather and corrections as operational layers.

### REQ-MAP-007 — JSON styling plugin
The system shall support a JSON-based map styling plugin with a configuration panel.

### REQ-MAP-008 — Tablet style
The styling system shall support a dedicated tablet presentation profile.

### REQ-MAP-009 — Optimizer route
Optimizer-generated routes shall be rendered and manually editable.

### REQ-MAP-010 — Waypoint movement
Waypoints shall be movable horizontally and vertically, with the new position retained.

### REQ-MAP-011 — Waypoint editor
Waypoint interaction shall expose number, coordinates, altitude, manual coordinate/altitude entry and metre displacement controls.

### REQ-MAP-012 — Waypoint add/delete
The waypoint editor shall support adding and deleting points.

### REQ-MAP-013 — Offline seven-point stub
A seven-point route stub shall be available without map-source connectivity: DEPARTURE -> WP1..WP7 -> DESTINATION.

### REQ-MAP-014 — UAV route filtering
Selecting a UAV shall display only its route; selecting the mission shall display all mission routes.

### REQ-MAP-015 — 3D route projection
The route shall be shown in 3D with a semi-transparent projection onto the terrain/surface.

### REQ-MAP-016 — Projection styling
Projection line and plane display parameters shall be configurable.

### REQ-MAP-017 — Collision display
Collision points shall be displayed in red.

### REQ-MAP-018 — NOTAM zones
The map shall display restriction/prohibited zones derived from NOTAM, including zone number and vertical limits.

### REQ-MAP-019 — NOTAM detail
Selecting a zone shall display the NOTAM number and source content used to construct the zone, with a close control.

### REQ-MAP-020 — UAV visual object
The UAV shall be displayed as a moving volumetric arrow.

### REQ-MAP-021 — UAV visual configuration
The operator shall be able to configure UAV arrow shape, color and transparency and provide a custom aircraft image.

### REQ-MAP-022 — Track styling
Completed and future flight paths shall have independent color, width and transparency settings.

### REQ-MAP-023 — Direction indicators
The route shall display movement-direction indicators between waypoints with configurable color, transparency, width and length.

### REQ-MAP-024 — Terrain hazards
The map shall display obstacle elevation and dangerous terrain exceedances above 15 m.

### REQ-MAP-025 — Scale indicator
The map shall provide a flexible scale indicator.

### REQ-MAP-026 — Scale-dependent typography
Map fonts shall scale according to map scale using knowledge-base rules.

### REQ-MAP-027 — Persistent configuration
Display settings shall be saved automatically and retained across sessions.

### REQ-MAP-028 — Configuration rollback
The system shall support rollback to the last successfully saved configuration.

### REQ-MAP-029 — Common waypoint style
Waypoint radius, color, line width, fill and transparency shall be configurable globally.

### REQ-MAP-030 — Actual position
The system shall determine and display the user's/vehicle's actual position where positioning data are available.

### REQ-MAP-031 — Operator restriction zones
Operators shall be able to create local optimizer restriction zones by radius or polygon points.

### REQ-MAP-032 — Restriction zone geometry
Operator-created restriction zones shall be defined by coordinates and altitude.

### REQ-MAP-033 — No zone intersections
The system shall prevent intersecting operator-created restriction zones.

### REQ-MAP-034 — Restriction visibility
A locally created restriction shall default to the creating operator and shall support explicit sharing with all operators/users.

### REQ-INS-001 — Insurance integration
The system shall provide a standardized insurance integration layer.

### REQ-INS-002 — Provider independence
The insurance provider shall not be a dependency of the BlueSky core.

### REQ-INS-003 — Insurance data exchange
The integration shall support transmission of UAV, operator, mission and operation parameters.

### REQ-INS-004 — Insurance lifecycle
The system shall support quote request, coverage options, application status and electronic policy data.

### REQ-INS-005 — Policy validation
The system shall support checking policy validity.

### REQ-INS-006 — Insurance architecture
The target architecture is `BlueSky -> Insurance Service -> Provider Adapter -> Insurance Provider`.

### REQ-CLIENT-001 — Client intent
The system shall begin mission composition from the client's requested outcome.

### REQ-CLIENT-002 — Modular composition
The system shall compose mission capabilities modularly according to the client's request.

### REQ-CLIENT-003 — Adaptive improvement
The system shall use recurring customer needs and operational outcomes to improve templates, algorithms and knowledge, subject to safety and governance controls.



