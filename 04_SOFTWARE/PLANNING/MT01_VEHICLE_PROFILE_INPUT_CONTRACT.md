# MT-01 vehicle profile and route lineage input contract

The HMI bridge accepts the following planning evidence through the MT-01 parameter map. Values must be supplied by the selected, versioned vehicle/configuration profile and the active terrain/airspace snapshot services. The bridge does not synthesize nominal values.

## Vehicle motion model

Required keys:

- `vehicleMotionModelId` — must equal the assigned vehicle instance ID.
- `vehicleMotionModelVersion`
- `vehicleMotionConfigurationId` — must equal the assigned configuration ID.
- `vehicleMotionClass` — `FIXED_WING` or `MULTIROTOR`.
- `vehicleMotionEvidence` — `OFFICIAL_VALIDATED` or `FLIGHT_TEST_VALIDATED`; derived scenarios are advisory only.
- `vehicleMotionReferenceSpeedMps`, `vehicleMotionMinimumSpeedMps`, `vehicleMotionMaximumSpeedMps`
- `vehicleMotionBankAngleRad`, `vehicleMotionLateralAccelerationMps2`
- `vehicleMotionSpeedReference` — explicit reference, e.g. `TAS` for fixed wing.
- `vehicleMotionSourceReference` — versioned evidence source.

The current MT-01 kinematic connector implementation generates fixed-wing Dubins transitions only. Multirotor motion profiles are represented by the shared model but do not yet have an MT-01 turn-connector implementation.

## Flight safety envelope

Required identity and provenance keys:

- `flightSafetyVehicleModelId`, `flightSafetyModelVersion`, `flightSafetyConfigurationId`
- `flightSafetyMotionClass`, `flightSafetyEvidence`
- `flightSafetySpeedReference`, `flightSafetyMinimumSpeedBasis`
- `flightSafetySourceReference`

Required limits:

- `flightSafetyMinimumSpeedMps`, `flightSafetyMaximumSpeedMps`
- `flightSafetyMaximumBankAngleRad` for fixed wing; or
  `flightSafetyMaximumLateralAccelerationMps2` for multirotor.

The envelope identity, model version, configuration, motion class and speed reference must match the vehicle motion model exactly.

## Canonical route lineage and environment

Required keys:

- `routeId`, `routeVersion`
- `routeGeneratorId`, `routeGeneratorVersion`
- `calculationInputVersion`
- `terrainSnapshotId` — immutable DEM snapshot ID with AOI coverage, supplied by the terrain service.
- `minimumTerrainClearanceM` — positive configured clearance.

The NOTAM snapshot parsed by the bridge becomes the route's airspace snapshot reference. Weather/wind references remain empty at this stage because the agreed sequence is base route generation first, weather correction second.

## Fail-closed behavior

Missing or mismatched vehicle evidence is reported as a finding and cannot produce a kinematic route. Missing route lineage or terrain snapshot prevents export of a canonical route candidate to the HMI RouteController. A structural route candidate is not a validated trajectory and does not imply release eligibility. Flight release remains false at this stage.
