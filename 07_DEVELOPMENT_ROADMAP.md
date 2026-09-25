# Development Roadmap

## Phase A — Foundation
- product architecture
- requirements baseline
- data model
- core UI concept
- traceability framework

## Phase B — Operational core
- Flight Chart
- mission planning
- aircraft/fleet model
- readiness workflow
- telemetry and C2 integration

## Phase C — Optimization
- energy-aware planning
- environmental correction
- multi-UAV coordination
- mission result analysis

## Phase D — AI
- internal learning from operational corrections
- local/offline AI capability
- synchronization of learned knowledge
- external technology/market analysis layer

## Phase E — Assurance
- verification
- regulatory integration
- certification work packages
- operational evidence
- aircraft-level risk and insurance workflow

The architecture is designed so these phases can evolve without replacing the central operational model.


## Multi-UAV Planning Implementation Sequence

Within Phase C — Optimization, multi-UAV coordination is decomposed into the following implementation order:

1. **Zone Partition Engine**
2. **UAV ↔ Zone Assignment Engine**
3. **Route-in-Zone Generator**
4. **Wind + Performance integration**
5. **4D Trajectory Engine**
6. **4D Conflict Verification Engine**
7. **Ground Conflict Resolution**
   - temporal delay: 0–5 s;
   - permitted vertical correction.
8. **Final Verification Gate**
9. **Traceability and replay of planning runs**

The implementation should preserve the documented planning pipeline and keep each stage independently testable.

### Initial verification cases

The implementation should include at least:

- non-overlapping zones → no fleet conflict;
- touching zone boundaries → boundary-separation verification;
- unavoidable geometric crossing → 4D temporal verification;
- conflict resolvable by delay ≤5 s;
- conflict requiring permitted vertical correction;
- conflict remaining after allowed corrections → plan blocked;
- wind/performance timing change introducing a conflict → re-verification required;
- repeated planning run with identical inputs/configuration → reproducibility check.

### UAV ↔ Zone Assignment Engine verification cases

After Zone Partition verification, assignment testing should include at least:
- one feasible UAV per zone → complete assignment;
- more UAVs than zones → valid reserve/unassigned aircraft;
- fewer feasible UAVs than zones → explicit unassigned zones and assignment failure;
- incompatible payload/capability → candidate rejected;
- insufficient endurance/reserve → candidate rejected;
- unavailable or non-ready UAV → candidate rejected;
- launch/recovery infeasibility → candidate rejected;
- missing/invalid C2 condition → candidate rejected;
- authorization or mission restriction → candidate rejected;
- heterogeneous fleet → capability-aware assignment rather than equal-area assignment;
- identical inputs/configuration → reproducible assignment;
- assignment failure → no route generation or release until replanning;
- complete assignment → verified ZoneAssignmentSet becomes the sole input to Route-in-Zone generation.

### Route-in-Zone Generator verification cases

- route remains inside assigned zone → valid;
- required launch/recovery transition is explicit and valid;
- route attempts to enter another UAV zone → generation rejected or regenerated;
- restricted geometry intersection → route rejected;
- mandatory waypoint/corridor requirement → satisfied or route rejected;
- minimum turn radius/maneuverability violation → route rejected;
- complete sweep coverage → coverage PASS;
- boundary margin causing uncovered area → explicit uncovered report;
- unavoidable shared corridor/crossing → explicitly marked for 4D verification;
- valid alternative route exists → generator avoids unnecessary cross-zone conflict;
- heterogeneous aircraft route limits → route adapted to assigned UAV;
- identical inputs/configuration → reproducible route;
- no valid route for a required zone → multi-UAV plan blocked pending replanning.

### Wind + Performance Engine verification cases

- identical route with headwind/tailwind/crosswind → segment timing responds correctly;
- spatially varying wind → segment-level calculation rather than one global correction;
- forecast and observed wind remain separately traceable;
- payload/equipment change → performance and energy estimates change;
- battery degradation correction → bounded, versioned and traceable effect;
- insufficient energy/reserve → plan blocked;
- performance model outside validity envelope → plan blocked;
- wind outside configured aircraft envelope → plan blocked;
- material timing change → previous 4D verification invalidated and rerun;
- identical inputs/model/configuration → reproducible result;
- forecast-versus-actual flight data → approved correction candidate with source traceability.

### 4D Trajectory Engine verification cases

- identical PerformanceAdjustedRouteSet → reproducible trajectory;
- timestamps remain monotonic;
- segment timing matches performance-adjusted route;
- altitude profile and climb/descent limits are preserved;
- launch/recovery occupancy is represented explicitly;
- approved temporal delay appears as an explicit temporal event;
- vertical correction creates a new trajectory version;
- trajectory contains UAV_ID, route and zone references;
- insufficient trajectory resolution → validation failure;
- missing/invalid timing or altitude → downstream conflict verification blocked;
- timing uncertainty remains distinguishable from nominal timing;
- conflict engine consumes the authoritative TrajectorySet without reconstructing timing independently.

### 4D Conflict Verification Engine verification cases

- geometric crossing with sufficient temporal separation → NO_CONFLICT;
- geometric crossing with insufficient temporal separation → CONFLICT;
- sufficient horizontal separation but insufficient vertical separation → CONFLICT;
- sufficient vertical separation with required horizontal separation → NO_CONFLICT;
- shared launch/recovery volume with valid sequencing → NO_CONFLICT;
- shared volume with insufficient temporal/spatial separation → CONFLICT;
- uncertainty margin reduces separation below requirement → appropriate WARNING/CONFLICT or UNRESOLVED according to configuration;
- insufficient trajectory resolution → verification blocked rather than assumed safe;
- eligible delay within 0–5 s → RESOLUTION_REQUIRED with feasible delay window;
- correction changes trajectory → previous verification invalidated and rerun;
- vertical correction permitted → new trajectory and full re-verification;
- unresolved conflict after permitted corrections → plan blocked;
- identical inputs/configuration → reproducible ConflictReport;
- all relevant fleet pairs and shared volumes → covered by verification evidence.
