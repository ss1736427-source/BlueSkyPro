# BlueSky PRO — External Integration

## Purpose

This folder defines the interfaces that connect BlueSky PRO with aircraft, flight-control systems, communications infrastructure and external aviation/operational systems.

## Interface map

| ID | Interface | Priority | Status |
|---|---|---:|---|
| IF-AUTOPILOT | Vehicle/autopilot abstraction | P0 | STRUCTURE |
| IF-MAVLINK | MAVLink session and transport | P0 | STRUCTURE |
| IF-COMMAND | Command/ACK execution | P0 | STRUCTURE |
| IF-MISSION | Mission translation and transfer | P0 | STRUCTURE |
| IF-PARAMETER | Parameter synchronization | P0 | STRUCTURE |
| IF-C2 | Command-and-control links | P0 | STRUCTURE |
| IF-VEHICLE-STATE | Normalized aircraft state | P0 | STRUCTURE |
| IF-SAFETY-CONFIG | Safety configuration reconciliation | P0 | STRUCTURE |
| IF-AIRSPACE-ATM | Airspace/ATM/FPL exchange | P0 | STRUCTURE |
| IF-PAYLOAD | Payload control/state/data | P1 | STRUCTURE |
| IF-VIDEO | Live/recorded video | P1 | STRUCTURE |
| IF-COMPANION | Companion computer | P1 | STRUCTURE |
| IF-GNSS-RTK | GNSS/RTK/NTRIP | P1 | STRUCTURE |
| IF-TRAFFIC | Traffic/ADS-B | P1 | STRUCTURE |
| IF-GIS-TERRAIN | GIS/DEM/obstacles | P1 | STRUCTURE |
| IF-WEATHER | Weather data | P1 | STRUCTURE |
| IF-LOG | Flight-log acquisition | P1 | STRUCTURE |
| IF-PERIPHERAL | DroneCAN/serial/Ethernet peripherals | P1 | STRUCTURE |
| IF-GROUND | Ground infrastructure | P1 | STRUCTURE |
| IF-MAINTENANCE | Maintenance/resource exchange | P2 | STRUCTURE |
| IF-SECURITY | Identity and secure communications | P2 | STRUCTURE |
| IF-ENTERPRISE | External business/data APIs | P2 | STRUCTURE |

## Design rule

BlueSky's internal mission and operational models are not coupled directly to a particular autopilot or transport protocol. External systems are connected through adapters/contracts.

## Next work order

1. Baseline P0 contracts.
2. Define protocol/message mappings.
3. Define failure and recovery behavior.
4. Define verification cases.
5. Implement representative ArduPilot/MAVLink adapter.
6. Repeat the same lifecycle in simulation.
