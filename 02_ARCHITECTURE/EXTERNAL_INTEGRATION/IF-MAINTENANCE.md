# IF-MAINTENANCE — Aircraft and Equipment Lifecycle Interface

## Purpose
Connect operational flight use with technical status and maintenance records.

## Inputs
Aircraft identity, component identity, flight hours/cycles, battery state and history, engine/propeller/resource data, faults, maintenance status and configuration baseline.

## Outputs
Readiness constraints, maintenance due status, component restrictions and evidence required for operational release.

## Required behavior
Maintenance state MUST be evaluated before readiness approval and correlated with the exact aircraft configuration used for the mission.

## Principle
Technical configuration, operational readiness and maintenance release are separate states and must not be conflated.
