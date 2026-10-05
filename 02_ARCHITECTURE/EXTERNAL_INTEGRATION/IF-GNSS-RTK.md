# IF-GNSS-RTK — Navigation Corrections Interface

## Purpose
Define integration with GNSS, RTK and correction services used to improve navigation accuracy.

## Sources
- UAV GNSS receiver
- RTK base station
- NTRIP/correction service
- other approved correction sources

## Data
Position, fix type, satellites, accuracy estimates, correction age/status and health.

## Required behavior
BlueSky must know whether navigation is nominal, degraded or unavailable and expose that state to validation, safety and mission execution.

## Key requirement
Correction-source loss must be represented as an explicit state and must not silently become valid navigation data.
