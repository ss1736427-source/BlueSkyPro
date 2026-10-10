# BlueSky PRO — Panel Tool Allocation 001

Status: working controlled reference

## Core rule

Each HMI panel has a narrow operational responsibility. Only the minimum required information is visible by default. The complete tool set is available through that panel's configuration control. Configuration of a function belongs to its owning context and is not duplicated in other panels.

## Bottom toolbar

`LEFT ◀ | UAV | MAP | ADMIN | FPV | VIRTUAL FLT | RIGHT ▶ | TIME | ☰`

LEFT and RIGHT are fixed panel anchors. TIME is a system element. ☰ configures the bottom-toolbar tool set. UAV, MAP, ADMIN, FPV and VIRTUAL FLT are independent work contexts and may be reordered.

## Header

Role: aggregate mission/system overview.

Visible by default:
- ETD
- TOT
- TRIP
- ETA
- READY
- WARNING
- OPERATOR

Optional aggregate indicators remain configurable in Header settings. Header does not contain detailed UAV configuration.

## Left Panel — Mission Creation

Owns:
- mission creation/editing
- mission templates
- mission identity
- mission area/route definition
- mission parameters
- UAV assignment and task distribution
- mission constraints
- mission planning/optimization parameters

Does not own UAV technical configuration, C2 configuration, FPV configuration, administration, or simulation configuration.

## Flight Chart / MAP

Owns spatial presentation and map interaction:
- base map
- terrain/3D
- operational layers
- airspace/restrictions/NOTAM
- weather layers
- route/WP presentation
- UAV spatial presentation
- planned/actual/completed track
- map interaction and route editing

Map provider infrastructure is configured through system integration; map display/layers are configured here.

## Right Panel — Mission Control

Visible by default:
- checklist
- warnings/corrections
- readiness
- contextual validation
- send flight plan
- start mission

Configuration provides the available checklist/validation/action blocks. It does not duplicate technical configuration owned by UAV, ADMIN, MAP or FPV.

## UAV Panel / UAV work context

First step: select the concrete UAV.

After selection, the UAV context owns the configuration of that specific aircraft.

Order:
1. UAV Selection
2. Control / C2
3. UAV Configuration
4. Navigation
5. Energy
6. Payload / Equipment
7. Maintenance
8. Diagnostics

### Control / C2

For the selected UAV:
- Control Station
- transmitter
- receiver
- communication link
- primary/backup
- DIRECT-C2 / PROVIDER-C2
- relay
- link parameters
- degradation/loss criteria
- failover
- control-channel configuration

The C2 topology/configuration is owned here. ADMIN does not duplicate the configuration of a specific UAV's control circuit.

Runtime C2 state is consumed/displayed by HUB/UAV/FPV as applicable.

## ADMIN

Owns system administration only:
- Users
- Roles & Access
- Authentication
- System Settings
- Integrations
- Data & Sync
- Documents
- Audit Log

Engineer area:
- Fleet
- Equipment
- Maintenance
- Documents
- Personnel

Technician area:
- My Tasks
- Equipment
- Checklists
- Reports
- Documents

ADMIN provides system resources, adapters, providers, permissions and infrastructure. It does not duplicate the per-UAV C2 topology/configuration.

## FPV — manual-control operating context

FPV owns:
- video display
- flight data overlay
- video-channel state
- control-channel state
- manual-control preparation
- control input mapping/check
- manual-control activation
- manual-control operation
- return-to-auto workflow

FPV does not create a second copy of the UAV's C2 configuration. It uses the selected UAV's configured control path.

### AUTO → MANUAL

When a UAV is flying automatically:
1. Operator opens FPV.
2. FPV displays camera, flight data, C2/video/control-channel state.
3. System checks readiness for control transfer.
4. FPV shows READY FOR CONTROL.
5. Pilot explicitly confirms TAKE CONTROL.
6. Control channel becomes ACTIVE.
7. UAV enters MANUAL.

If readiness is not achieved, transfer is unavailable and the reason is shown.

### MANUAL → AUTO

The reverse transition is also explicit:
1. Pilot requests RETURN TO AUTO.
2. System checks mission state, route, UAV state, C2 and safety/validation conditions.
3. FPV shows READY FOR AUTO only when the controlled conditions are satisfied.
4. Pilot explicitly confirms AUTO.
5. AUTO becomes active.

A failed check does not force AUTO; the UAV remains in MANUAL and the blocking condition is shown.

For multi-UAV missions, these transitions are per selected UAV and do not automatically change the control state of other UAVs.

## VIRTUAL FLT

Owns:
- virtual UAV
- simulation setup
- simulation environment
- mission simulation
- scenarios/failures
- simulation controls
- planned/simulated/actual comparison
- simulation visualization and results

Simulation does not acquire execution authority.

## Runtime / display principle

Configuration belongs to the owning context. Runtime state may be surfaced elsewhere only as an operational indication when required.

Examples:
- C2 configuration: UAV → Control/C2.
- C2 runtime state: UAV/HUB/FPV; warning/readiness may surface in Right/Header.
- Video configuration: FPV.
- Video runtime state: FPV/UAV/Right as required.
- Map configuration: MAP.
- Mission configuration: LEFT.
- System administration: ADMIN.

No secondary panel becomes an alternative configuration owner.

## DS implementation implication

The next Design Studio pass should implement the panel shells and configuration entry points from this allocation without introducing duplicate settings. FPV should visibly support the AUTO → READY FOR CONTROL → TAKE CONTROL → MANUAL and MANUAL → READY FOR AUTO → CONFIRM AUTO → AUTO state model.
