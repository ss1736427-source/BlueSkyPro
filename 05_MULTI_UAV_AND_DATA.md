# Multi-UAV Operations and Data

A mission can be decomposed between several UAVs when the operational objective requires it.

The system concept includes:

- task decomposition;
- aircraft capability matching;
- temporal separation and coordination;
- individual aircraft mission state;
- combined mission status;
- post-flight data association;
- consolidation of mission outputs.

Each aircraft remains an identifiable operational entity with its own state, authorization and flight record. The mission layer coordinates the aircraft without erasing individual accountability.


## Zone-Based Trajectory Separation

For coordinated coverage, the preferred operating model is **one UAV per assigned operational zone/sector**. Route generation must attempt to partition the usable mission space so that trajectories remain separated by construction.

The sequence is:

1. determine the constrained open space;
2. partition it into operational zones;
3. assign each UAV to one zone;
4. generate the route only inside its assigned zone;
5. generate the time-dependent trajectory using wind and aircraft performance;
6. perform 4D conflict verification across the fleet.

### Conflict fallback

If the mission geometry makes complete zonal separation impossible, the system may resolve the remaining planned conflict using:

- a temporal delay of **0–5 seconds**;
- a permitted vertical correction.

The correction is applied to the planned trajectory, not as an uncontrolled ad-hoc maneuver. After correction, the complete 4D conflict verification is repeated.

### Safety hierarchy

```
ZONE SEPARATION
      ↓
ROUTE SEPARATION
      ↓
4D CONFLICT VERIFY
      ↓
TEMPORAL / VERTICAL RESOLUTION
      ↓
4D CONFLICT VERIFY AGAIN
      ↓
FINAL CHECK
```

If the conflict cannot be resolved within the permitted temporal and vertical constraints, the mission remains unresolved and must not be released as a conflict-free multi-UAV plan.
