# Weather-aware route candidate handoff

## Purpose

The optimizer generates and validates route candidates **without wind correction**. The HMI requests one union weather-coverage region for the candidate set, downloads one coherent GFS forecast run, decodes it once, and invokes the weather assessor for every candidate. The planning orchestrator then ranks the weather-corrected metrics using the mission's declared objective priorities.

This interface is a weather re-ranking contract. It is not a flight-release authorization.

## Planning result payload

The existing `planning.result` envelope remains unchanged. Its `result` object may provide:

- `routeCandidates`: array of complete, independently validated route candidates (1–64).
- `selectedCandidateId`: candidate to display before weather re-ranking.
- `objectivePriorities`: ordered objective keys consumed by `AlgorithmOrchestrator`.

Weather re-ranking currently accepts only objectives whose values are recalculated from the wind-corrected trajectory: `completion_time`, `time`, `fast`, `energy`, `endurance`, `reserve`. `minimum_cost` and `route_efficiency` are not accepted in this path until their weather-corrected objective-score model is defined.

Each route candidate must contain:

| Field | Requirement |
|---|---|
| `candidateId` | Non-empty and unique within the set |
| `routeId`, `routeVersion` | Non-empty; route identity unique within the set |
| `missionId`, `missionVersion` | Same across all candidates and equal to the active mission |
| `generatorId`, `generatorVersion` | Non-empty lineage |
| `calculationInputVersion` | Same across all candidates |
| `coordinateReferenceSystem` | `EPSG:4326` |
| `environmentSnapshotRefs.terrainSnapshotId` | Required |
| `environmentSnapshotRefs.airspaceSnapshotId` | Required |
| `environmentSnapshotRefs.weatherSnapshotId`, `windSnapshotId` | Both may be empty before weather assessment; otherwise both must be present |
| `trajectoryGeometryComplete` | `true`; includes all flown legs, turns and transitions |
| `validationState` | `VALIDATED` |
| `flightSafetyValidationState` | `ALLOWED` |
| `flightSafetyEnvelopeId` | Non-empty; same across the set |
| `flightSafetyVehicleId` | Non-empty; same across the set and equal to the assigned UAV |
| `estimatedTimeS`, `estimatedEnergyWh`, `estimatedReserveWh`, `objectiveScore` | Finite nominal metrics; time positive, energy and reserve non-negative |
| `waypoints` | At least two unique, ordered waypoints with valid coordinates and AGL altitude |
| `segments` | Exactly one ordered segment per waypoint pair; unique IDs, positive distance/speed and explicit dynamic data required by the safety gate |

Waypoint task execution data is optional, but `taskExecutionDurationS` and `taskExecutionEnergyWh` must be supplied together and be finite, non-negative values.

## Processing order

1. The optimizer creates candidates and applies route, airspace/NOTAM and aircraft safety checks. No wind correction is applied at this stage.
2. The HMI rejects mixed mission versions, calculation-input versions, safety envelopes or aircraft identities.
3. The weather coverage bridge builds a union request over every candidate route and all required AGL levels.
4. GFS products are fetched for one model run and a common set of forecast valid times.
5. GRIB2 files are identity/hash checked and decoded into one immutable weather snapshot.
6. The same snapshot and assigned-UAV performance input are used to assess each candidate.
7. `AlgorithmOrchestrator` rejects candidates with incomplete or infeasible weather assessment and ranks the remaining candidates using the declared objective priorities.
8. The HMI selects the returned `selectedCandidateId` and displays the weather-corrected metrics.

## Fail-closed rules

- Missing or incomplete flown geometry is not reconstructed from a graph path or waypoint chain.
- Missing route validation or flight-safety attestation blocks candidate-set ingestion.
- A safety attestation for a different UAV or envelope blocks the set.
- Mixed mission/input/safety lineage blocks the set.
- Missing objective priorities blocks weather re-ranking.
- Missing, invalid, mismatched or undecodable forecast data blocks weather re-ranking.
- A decoded forecast with no feasible candidate does not produce a selected route.
- Weather re-ranking always reports `flightReleaseEligible: false`; the independent final flight-readiness gate remains authoritative.

## Current integration boundary

The HMI now accepts `result.routeCandidates`, retains the validated set, requests union coverage, and has a multi-candidate weather evaluation path that decodes one snapshot and uses the real `AlgorithmOrchestrator`.

The upstream planning producer must still emit this contract. The current generic A*/Dijkstra graph solvers publish structural graph paths, not complete flown trajectories with validated transition geometry and per-candidate safety evidence. Those paths must not be relabelled as complete trajectories to satisfy this interface.
