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
