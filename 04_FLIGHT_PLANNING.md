# Flight Planning

The planning concept is based on an energy-aware and constraint-aware mission model.

The planner can account for:

- aircraft capabilities and installed equipment;
- mission objective;
- route geometry;
- altitude constraints;
- restricted areas and operational limitations;
- weather and wind;
- mandatory waypoints;
- lateral and vertical bypass conditions;
- energy reserve and aircraft degradation history;
- multi-UAV task decomposition.

The interface presents one active operational route while alternatives can be evaluated in the planning process.

The design goal is not simply the shortest route. The route is evaluated against the mission objective, operational constraints, environmental conditions and energy preservation.


## 4D Zone-Based Multi-UAV Route Generation and Conflict Resolution

For multi-UAV coverage missions, route generation follows a hierarchical conflict-avoidance sequence. The system must first remove the need for trajectory crossing through spatial decomposition; temporal and vertical conflict resolution is a fallback, not the primary planning mechanism.

```
MISSION / COVERAGE
        ↓
CONSTRAINED OPEN SPACE
        ↓
ZONE PARTITION
        ↓
UAV ↔ ZONE ASSIGNMENT
        ↓
ROUTE-IN-ZONE
        ↓
WIND + PERFORMANCE
        ↓
4D TRAJECTORY
        ↓
4D CONFLICT VERIFY
        ↓
if zonal separation is impossible
        ↓
GROUND CONFLICT RESOLUTION
        ├─ delay: 0–5 s
        └─ vertical correction
        ↓
FINAL CHECK
```

### Planning rules

1. **Constrained Open Space** is the mission area after exclusion of restricted, prohibited, unavailable or otherwise constrained regions.
2. **Zone Partition** divides the usable area into operationally clean sectors so that each UAV has a defined working zone.
3. **UAV ↔ Zone Assignment** assigns each aircraft to one zone according to capability, mission requirements and operational constraints.
4. **Route-in-Zone** generates the coverage route inside the assigned zone. The default objective is to avoid trajectory intersections between aircraft rather than generate intersecting routes and resolve them later.
5. **Wind + Performance** modifies route speed, heading, altitude and energy estimates using environmental conditions and aircraft performance.
6. **4D Trajectory** represents the resulting route as a time-dependent trajectory: position, altitude and time.
7. **4D Conflict Verify** checks spatial and temporal separation between all active UAV trajectories, including crossings, convergence and insufficient separation margins.
8. If a conflict remains and **zonal separation cannot resolve it**, the planner applies **Ground Conflict Resolution** to the planned trajectories:
   - temporal delay within **0–5 seconds**;
   - vertical correction when permitted by the mission and aircraft constraints.
9. **Final Check** repeats constraint, performance and 4D conflict verification after every correction. A mission is not released while an unresolved conflict remains.

### Design principle

The conflict-resolution hierarchy is therefore:

**zone separation → route-in-zone separation → 4D verification → limited temporal/vertical correction → final verification.**

This prevents the system from relying on continuous collision correction when the conflict can be eliminated earlier through mission decomposition and route generation.
