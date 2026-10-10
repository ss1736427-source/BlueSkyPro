# IF-VEHICLE-STATE — BlueSky Vehicle State Interface

**Status:** STRUCTURAL DRAFT  
**Purpose:** Define the normalized vehicle state consumed by BlueSky from an external flight controller and communication stack.

## 1. Flow

```text
Autopilot / Sensors
        |
        v
MAVLink / Vehicle Adapter
        |
        v
State Normalization
        |
        v
HUB / Vehicle State
        |
   +----+----+
   |         |
   v         v
Safety    Pilot/UI
   |
   v
Mission Execution / Decision Logic
```

## 2. State domains

- connection/link state;
- heartbeat/vehicle identity;
- armed/disarmed state;
- flight mode;
- navigation state;
- position and altitude;
- attitude;
- ground/air speed and heading;
- GNSS status and navigation quality;
- battery state;
- vehicle health;
- active failsafe state;
- mission execution state;
- payload state;
- time synchronization state.

## 3. Requirements

The interface MUST distinguish source timestamp, reception timestamp, validity, freshness and quality. Loss, staleness or contradiction of critical state MUST be represented explicitly rather than replaced with a plausible value.

## 4. Multi-UAV

Every vehicle state MUST remain addressable to a unique vehicle identity and MUST support concurrent fleet state without ambiguity between vehicles.

## 5. Open items

- normalized state schema;
- required MAVLink message set;
- update rates and priority classes;
- stale-data thresholds;
- state conflict resolution;
- loss-of-link semantics;
- time synchronization;
- vehicle/component identity mapping.
