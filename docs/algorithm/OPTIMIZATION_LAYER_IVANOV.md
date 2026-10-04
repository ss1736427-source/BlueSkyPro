# BlueSky PRO — Optimization Layer / Ivanov Method Integration

**Status:** ARCHITECTURE BASELINE  
**Date:** 2026-10-04  
**Source basis:** Ivanov S. V., 2022, *A method for constructing suboptimal routes for a group of unmanned aerial vehicles based on bioinspired algorithms in the presence of obstacles*.

## 1. Decision

The bio-inspired route-planning methods described by Ivanov are adopted in BlueSky PRO as a **separate optimization layer**.

They MUST NOT replace or weaken the existing deterministic planning, constraint, safety, energy, trajectory, or multi-UAV conflict mechanisms.

The optimization layer searches for better candidates among routes that are already feasible or generates candidates that are subsequently subjected to the complete safety/constraint verification chain.

## 2. Operator task definition is the entry point

The planning algorithm MUST begin with the task formulated by the operator during mission preparation.

The operator's input is not treated as UI-only configuration. It is converted into the formal **Mission Profile / Task Model** used by the Planning Kernel and Optimization Layer.

The common sequence is:

OPERATOR TASK DEFINITION  
→ MISSION PROFILE  
→ TASK MODEL  
→ ENVIRONMENT / CONSTRAINTS  
→ PLANNING KERNEL  
→ OPTIMIZATION LAYER  
→ 4D TRAJECTORY  
→ SAFETY VALIDATION  
→ READY

The Mission Profile may contain, depending on mission type:

- mission template/type;
- start and destination / operational area;
- departure time and, where applicable, arrival/deadline;
- mission priority;
- selected UAV / UAV group;
- payload and sensor requirements;
- required altitude or altitude range;
- energy reserve requirement;
- route preference (time, energy, distance, balanced);
- acceptable risk or operational policy;
- task-specific constraints and requirements;
- additional operator-defined conditions.

The system MUST distinguish between:

1. **operator-provided requirements** — what the mission must accomplish;
2. **system-derived environment data** — NOTAM, airspace, terrain, obstacles, wind, UAV performance, battery state, etc.;
3. **computed planning results** — route, timing, energy, conflicts, trajectory and optimization result.

Operator inputs that represent objectives, priorities, deadlines, values or trade-offs become formal optimization criteria or constraints and MUST NOT be discarded after mission creation.

Example:

Operator task:
> Fly from A to B at 10:30, high priority, maximize energy preservation, reserve 25%, altitude 120 m.

Formal task model:

- MISSION = LONG_FLIGHT
- START = A
- DESTINATION = B
- ETD = 10:30
- PRIORITY = P1
- OBJECTIVE = energy minimization
- ENERGY_RESERVE >= 25%
- ALTITUDE = 120 m

The planner then combines this task model with system-derived environmental and vehicle data.

## 3. Existing planning chain remains authoritative

MISSION / OPERATOR TASK  
→ TASK / OBJECTIVES  
→ ENVIRONMENT  
→ CONSTRAINT ENGINE  
→ CONSTRAINED OPEN SPACE  
→ ZONE PARTITION  
→ UAV ASSIGNMENT  
→ ROUTE GENERATION  
→ WIND / PERFORMANCE  
→ ENERGY / RESERVE  
→ 4D TRAJECTORY  
→ MULTI-UAV CONFLICT  
→ CONFLICT RESOLUTION  
→ RE-VERIFY  
→ OPTIMIZATION LAYER  
→ FINAL SAFETY GATES  
→ UAV-SPECIFIC COMPILATION

The Optimization Layer is an extension of the Planning Kernel, not a second independent planner.

## 3A. Interactive operator-defined mandatory flight points

The map and flight profile are interactive planning surfaces and form part of the operator input, not merely visualization.

The operator may place **mandatory passage points** directly on:

- the interactive map;
- the altitude/flight profile;
- the 3D trajectory view, when available.

These points are stored in the Task Model as explicit route constraints.

Each mandatory point shall retain at minimum:

- unique point ID;
- geographic position;
- required altitude or altitude band, when specified;
- required passage order, when specified;
- point type/reason (mandatory passage, observation, inspection, target, etc.);
- tolerance/corridor, when defined by the mission method;
- operator-defined dwell/observation requirement, when applicable.

The Planning Kernel MUST preserve mandatory points during route generation and optimization. Optimization may change the geometry between mandatory points, but MUST NOT remove, reorder, or violate a mandatory point constraint unless the operator explicitly changes the task.

The map and profile are bidirectionally linked:

MAP POINT ↔ 3D TRAJECTORY POINT ↔ PROFILE POINT ↔ TASK MODEL CONSTRAINT

Selecting or moving a point in one view updates the corresponding representation in the other views and triggers the required recalculation.

A mandatory point is not automatically considered feasible merely because the operator placed it. The Planning Kernel must verify airspace, terrain/obstacle clearance, altitude limits, UAV performance, energy and other applicable constraints at and between mandatory points.

If a mandatory point cannot be legally or safely incorporated, the mission is NOT FEASIBLE / REQUIRES OPERATOR ACTION; the optimizer must not silently bypass it.

This rule applies to all mission templates. Template-specific methods may add additional semantics to the point, but the authoritative constraint representation remains common to the Planning Kernel.

## 4. Hard constraints vs optimization criteria

### Hard constraints

The optimizer MUST NOT override:

- NOTAM / forbidden airspace;
- regulatory airspace restrictions;
- altitude limits;
- terrain/obstacle clearance;
- UAV performance limits;
- endurance and mandatory reserve;
- mission safety requirements;
- multi-UAV separation/conflict constraints;
- final safety validation results.

A candidate violating a hard constraint is rejected.

### Soft optimization criteria

The optimizer MAY optimize:

- energy consumption;
- flight time;
- distance;
- risk/cost;
- task value;
- task priority;
- coverage;
- UAV workload/balance;
- computational cost.

## 5. Optimization objective

The canonical multi-objective representation is:

J = wE*Energy + wT*Time + wD*Distance + wR*Risk + wC*Complexity - wP*PriorityValue

Weights are mission-profile dependent.

Recommended profiles:

- FAST — time dominant;
- ECONOMY — energy dominant;
- BALANCED — balanced objective;
- PRIORITY — priority/deadline dominant;
- MAX COVERAGE — coverage/task-value dominant.

## 6. Bio-inspired methods

The following methods are retained as interchangeable search operators inside the Optimization Layer:

### Genetic Algorithm (GA)
Global exploration of candidate task order / route structure.

### Ant Colony Optimization (ACO)
Search for promising transitions and task sequences.

### Simulated Annealing (SA)
Local improvement and escape from local minima.

They are NOT authoritative safety mechanisms.

The implementation may use one method, a hybrid, or a mission-specific strategy. The choice must be controlled by the optimization policy and validated by the same safety gates.

## 7. Pareto optimization

For conflicting objectives, BlueSky PRO may retain a Pareto-optimal set instead of forcing an immediate single solution.

Example:

Route A: low energy / longer time  
Route B: balanced  
Route C: shortest time / higher energy

The final selection is made according to the mission optimization profile and operator policy.

## 8. Task priority

Tasks shall carry explicit optimization metadata:

- priority: P0 Mandatory / P1 Critical / P2 Normal / P3 Optional;
- value;
- deadline, when applicable;
- sensor/payload requirements;
- dwell/observation time;
- required quality.

Priority affects route ordering and allocation but never permits violation of hard constraints.

## 9. Opportunistic Task Insertion

The optimizer may test insertion of nearby tasks into an already planned route.

For candidate insertion C into A → B, calculate:

- ΔTime;
- ΔEnergy;
- ΔDistance;
- added risk/cost;
- task priority/value.

Insert only when the resulting route remains feasible and the configured optimization policy accepts the trade-off.

## 10. Directed graph requirement

BlueSky PRO MUST NOT assume symmetric transition cost.

For realistic planning:

Cost(A → B) != Cost(B → A)

because wind, altitude profile, UAV performance, energy consumption and restrictions can make the two directions different.

Therefore the optimization graph is directed.

## 11. Multi-UAV integration

Optimization MAY operate at:

1. task allocation level;
2. task ordering level;
3. route geometry level;
4. multi-UAV distribution level.

After optimization, the complete multi-UAV conflict/deconfliction and safety verification chain MUST be executed again.

No optimized route is considered valid until re-verification succeeds.

## 12. Template development

Mission templates shall use the Optimization Layer as a configurable service.

Template-specific optimization policies may define:

- objective weights;
- priority behavior;
- task insertion policy;
- Pareto retention policy;
- GA/ACO/SA strategy;
- iteration/time budget;
- acceptable energy/time trade-offs.

The template MUST NOT duplicate the safety/constraint engine.

Conceptually:

MISSION TEMPLATE  
→ OPERATOR TASK MODEL  
→ CONSTRAINT ENGINE  
→ BASE ROUTE  
→ TEMPLATE OPTIMIZATION POLICY  
→ OPTIMIZATION LAYER  
→ SAFETY / CONFLICT RE-VERIFY  
→ FINAL ROUTE

## 13. Reference mapping to Ivanov (2022)

The source contributes the following concepts to BlueSky PRO:

- bio-inspired search for suboptimal routes;
- hybrid GA / ACO / SA approach;
- unequal target/task priority;
- obstacle-aware route selection;
- multi-UAV route distribution;
- multi-criteria optimization;
- Pareto-optimal solution reduction;
- computationally bounded heuristic search.

The source's simplifying assumptions are NOT copied unchanged into BlueSky PRO. In particular, BlueSky PRO must account for directed transition costs caused by wind and UAV performance.

## 14. Engineering rule

**Operator-defined task → formal Task Model → deterministic feasibility → heuristic optimization → final safety re-verification**

The optimization layer is therefore an enhancement of the existing BlueSky PRO algorithm and must not become a replacement for its authoritative safety and constraint mechanisms.

## 15. Implementation status

This document is the architectural baseline for subsequent development of mission-template algorithms.

Any future template algorithm that uses GA, ACO, SA, Pareto optimization, priority routing, opportunistic task insertion, or operator-defined optimization objectives MUST reference this layer rather than implementing an independent optimization architecture.
