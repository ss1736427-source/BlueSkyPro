# IF-LOG — Flight Log Interface

## Purpose
Define ingestion, storage and correlation of flight-control and telemetry logs from UAV systems.

## Sources
- onboard flight-controller logs
- telemetry/C2 logs
- payload/media logs
- BlueSky operational events

## Required functions
- identify source and vehicle
- preserve timestamps
- integrity/provenance
- download/retrieve
- correlate with mission and payload data
- support replay and post-flight analysis
- retain configuration/version context

## Key requirement
The authoritative Flight Record must preserve the relationship between mission intent, commands, vehicle state, events and resulting data.
