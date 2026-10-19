# Virtual Flight synthetic assessment adapter — implementation checkpoint

**Status:** canonical planning-core adapter implemented in HMI; CI verification pending  
**Branch:** main  
**Fixture contract:** VF-FIXTURES-1.0

## What the adapter does

- Invokes the existing C++ planning-core route builder and airspace + NOTAM assessment functions; it does not reimplement their algorithms.
- Builds an explicit fixed synthetic three-waypoint AGL route and versioned synthetic airspace/NOTAM snapshots.
- Scenario 1 exercises a route with no intersecting synthetic NOTAM zone.
- Scenario 2 exercises an active prohibited synthetic zone intersecting the route.
- Scenario 3 demonstrates that a clear airspace/NOTAM assessment does not assess weather thresholds.
- Always reports flightReleaseEligible=false.

## Boundaries

- Route coordinates are test geometry only. They are not read from the operator's map or a real operating location.
- The fixture timestamp is simulated; it is not current time or evidence of current aeronautical information.
- Weather values are synthetic metadata but are not evaluated by this airspace/NOTAM assessment.
- Terrain, obstacle clearance, wind/energy performance, task requirements, permissions and final release gates are not evaluated.
- The adapter must not be used for flight operations and does not connect any live provider.
- Connecting actual manual geometry is a separate step. Current map geometry does not consistently carry an explicit altitude datum; the route builder rejects geometry without explicit AGL altitude.

## Verification

The virtual_flight_assessment_bridge_test asserts the clear case, prohibited-zone intersection, weather-scope boundary, invalid profile handling, and invariant that flight release is never eligible. CI must pass on the exact commit SHA before this checkpoint is closed.
