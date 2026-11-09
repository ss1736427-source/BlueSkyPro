# UAV Maneuverability Reference Dataset 001

**Status:** SOURCE-QUALIFIED REFERENCE DATA; NOT FLIGHT-VALIDATED  
**Date:** 2026-11-09  
**Purpose:** Populate BlueSky PRO vehicle-motion inputs from public manufacturer/autopilot documentation without inventing aircraft limits.

## 1. Data policy

Every parameter is classified as one of:
- `OFFICIAL_SPEC`: explicitly published by the aircraft manufacturer.
- `AUTOPILOT_PARAMETER`: configurable controller limit; not proof of the airframe's physical capability.
- `DERIVED`: calculated from explicitly identified source values and a stated physical model.
- `UNKNOWN`: not established by the reviewed sources.

An unknown turn radius is a blocking input for a planner that requires a bounded-curvature trajectory. Do not substitute a generic radius based only on aircraft category, speed, yaw rate, or marketing class.

Published maximum values are not automatically suitable planning values. Use an operational limit with margin, the actual flight mode, loaded configuration, firmware/controller parameters, airspeed and environmental conditions.

## 2. Manufacturer-published aircraft records

### 2.1 DJI Matrice 350 RTK — multirotor reference

| Parameter | Value | Classification | Notes |
|---|---:|---|---|
| Maximum horizontal speed | 23 m/s | OFFICIAL_SPEC | Maximum, not recommended mapping speed |
| Maximum pitch angle | 30 deg | OFFICIAL_SPEC | N mode with forward vision enabled: 25 deg |
| Maximum yaw angular velocity | 100 deg/s | OFFICIAL_SPEC | Body yaw capability; not horizontal path curvature |
| Maximum pitch angular velocity | 300 deg/s | OFFICIAL_SPEC | Attitude rate, not turn rate |
| Maximum wind resistance | 12 m/s | OFFICIAL_SPEC | Does not define cross-track turn performance |
| Minimum horizontal turn radius | UNKNOWN | UNKNOWN | DJI specification reviewed does not publish it |
| Sustained lateral acceleration limit | UNKNOWN | UNKNOWN | Not established by the reviewed manufacturer specification |

Source: DJI, Matrice 350 RTK Specifications, https://enterprise.dji.com/matrice-350-rtk/specs

**Planning use:** Populate the published speed/attitude/yaw limits with source provenance. Do not derive a path radius from 100 deg/s yaw rate: multirotor heading can rotate independently of its horizontal velocity vector. A path-curvature model requires a validated lateral acceleration/velocity envelope or flight-test data for the exact configuration and mode.

### 2.2 WingtraOne GEN II — tailsitter VTOL / fixed-wing cruise

| Parameter | Value | Classification | Notes |
|---|---:|---|---|
| Aircraft type | Tailsitter VTOL | OFFICIAL_SPEC | Separate vertical and wing-borne flight phases |
| Published cruise/flight speed | 16 m/s | OFFICIAL_SPEC | Manufacturer product specification |
| Maximum sustained wind | 12 m/s | OFFICIAL_SPEC | Environmental limit, not turn capability |
| Maximum gust wind | 18 m/s | OFFICIAL_SPEC | Environmental limit |
| Minimum fixed-wing turn radius | UNKNOWN | UNKNOWN | Not published in reviewed specification |
| Maximum bank angle for mission turns | UNKNOWN | UNKNOWN | Not published in reviewed specification |
| VTOL transition maneuver envelope | UNKNOWN | UNKNOWN | Requires mode-specific manufacturer/controller data |

Source: Wingtra, WingtraOne GEN II, https://wingtra.com/wingtraone-gen-ii-drone/

**Planning use:** Treat vertical take-off/landing and fixed-wing cruise as distinct motion modes. The 16 m/s value is a speed reference only; it does not establish a turn radius. Do not use multirotor yaw-in-place or fixed-wing Dubins assumptions across the VTOL transition.

### 2.3 Quantum-Systems Trinity Pro — eVTOL fixed-wing mapping platform

| Parameter | Value | Classification | Notes |
|---|---:|---|---|
| Aircraft type | eVTOL fixed-wing | OFFICIAL_SPEC | Vertical launch/recovery, wing-borne cruise |
| Optimal cruise speed | 17 m/s | OFFICIAL_SPEC | Published optimal cruise speed |
| Maximum flight time | 90 min | OFFICIAL_SPEC | Subject to export/default restrictions stated by manufacturer |
| Wind tolerance, hover phase | 11 m/s | OFFICIAL_SPEC | Hover/transition phase |
| Wind tolerance, cruise phase | 14 m/s | OFFICIAL_SPEC | Cruise phase; default regional limit may be lower |
| Minimum fixed-wing turn radius | UNKNOWN | UNKNOWN | Not published in reviewed tech sheet |
| Maximum bank angle for mission turns | UNKNOWN | UNKNOWN | Not published in reviewed tech sheet |
| Transition maneuver envelope | UNKNOWN | UNKNOWN | Requires mode-specific data |

Source: Quantum-Systems, Trinity Pro Technical Specifications, https://lp.quantum-systems.com/hubfs/Downloadables/QS_INT_Trinity_Pro_Techsheet_241015_Screen.pdf

**Planning use:** Keep hover/transition/cruise envelopes separate. 17 m/s is not a basis for inventing turn radius.

## 3. Autopilot configuration references — not aircraft reference values

ArduPilot documentation exposes controller parameters relevant to a configured vehicle:
- Plane `ROLL_LIMIT_DEG`: configured maximum commanded bank angle; documented range 0–90 deg.
- Plane `AIRSPEED_MIN`, `AIRSPEED_MAX`, `AIRSPEED_CRUISE`: configured airspeed limits/target.
- Copter `ATC_ANGLE_MAX`: configured maximum lean angle; documentation describes a default of 30 deg.
- Copter `AROT_XY_ACC_MAX`: body-frame XY acceleration limit in the speed controller, documented range 0.5–8.0 m/s².

Sources:
- ArduPilot Plane parameters: https://docs.ardupilot.org/plane/docs/parameters.html
- ArduPilot Copter parameters: https://docs.ardupilot.org/copter/docs/parameters.html
- ArduPilot Copter Stabilize mode: https://docs.ardupilot.org/copter/docs/stabilize-mode.html

These are controller configuration parameters and documented parameter ranges. They are not evidence that every aircraft supports the upper limit, nor are they manufacturer-certified physical limits. BlueSky may ingest actual read-back values from a specific vehicle configuration, but must preserve firmware/version, parameter source, operating mode and validation status.

## 4. Valid derivation models

### 4.1 Fixed-wing coordinated level turn

For a coordinated, approximately level turn:

`R = V_{TAS}^2 / (g \tan(\phi))`

where:
- `R` is air-relative turn radius in metres;
- `V_TAS` is true airspeed in m/s;
- `g` is gravitational acceleration (9.80665 m/s²);
- `phi` is bank angle.

This derivation is admissible only when the selected airspeed and bank limit are valid for the exact aircraft configuration, flight mode, mass/CG and operating condition. Stall margin, structural limits, controller tracking, gusts and wind must be checked separately. Ground-track curvature is wind-dependent and is not equal to the air-relative radius in general.

### 4.2 Multirotor constant-speed horizontal turn

For an idealized constant-speed horizontal turn:

`R = V^2 / a_{lat}`

where `a_lat` is a validated sustained lateral acceleration available in the relevant flight mode and configuration.

Do not substitute maximum yaw rate for `a_lat`. Do not substitute a generic autopilot parameter range or maximum tilt angle without evidence that the aircraft can sustain the corresponding lateral acceleration at the selected speed, payload and conditions.

### 4.3 VTOL aircraft

VTOL aircraft require mode-specific envelopes:
- vertical/hover;
- transition;
- wing-borne cruise;
- transition to recovery/landing.

A single universal turn radius across all phases is invalid unless the manufacturer explicitly defines and validates such a model.



## 4.4 Preliminary calculated maneuverability scenarios — NOT VALIDATED

Where the manufacturer does not publish turn performance, BlueSky may calculate **scenario estimates** from published speed and explicitly assumed dynamic limits. These estimates are useful for engineering comparison, early route-space sizing and identifying missing data. They are **not aircraft limits**, are not flight-test evidence and must not independently pass a safety hard gate.

### DJI Matrice 350 RTK — multirotor scenario

The manufacturer publishes maximum horizontal speed of 23 m/s but no sustained lateral-acceleration envelope. The following table evaluates the idealized relation `R = V²/a_lat` at 23 m/s using selected hypothetical sustained lateral-acceleration cases:

| Assumed sustained lateral acceleration | Calculated air-relative radius |
|---:|---:|
| 1.0 m/s² | 529.0 m |
| 2.0 m/s² | 264.5 m |
| 3.0 m/s² | 176.3 m |
| 4.0 m/s² | 132.2 m |

**Classification:** `DERIVED_SCENARIO`; confidence low; not validated for M350 RTK. The acceleration values above are scenario inputs, not DJI-published values and not inferred from yaw rate, pitch angle or ArduPilot parameter ranges. At lower selected speeds the idealized radius reduces quadratically for the same assumed acceleration. Actual path response depends on controller limits, payload/mass/CG, wind, speed, mode and transient response. Do not select one row as the aircraft's operational turn radius without configuration read-back and flight validation.

### WingtraOne GEN II — fixed-wing cruise scenario

Using the published 16 m/s flight-speed value as a provisional TAS proxy, the coordinated-level-turn equation gives:

| Assumed bank angle | Calculated air-relative radius |
|---:|---:|
| 15 deg | 97.4 m |
| 20 deg | 71.7 m |
| 25 deg | 56.0 m |
| 30 deg | 45.2 m |

**Classification:** `DERIVED_SCENARIO`; confidence low; not validated for WingtraOne GEN II. The bank angles are hypothetical analysis points, not Wingtra-published operational limits. The 16 m/s published speed is not explicitly identified as TAS in the reviewed product specification; treating it as TAS is an assumption. The values apply only to idealized wing-borne, coordinated, level turns, not hover or transition.

### Quantum-Systems Trinity Pro — wing-borne cruise scenario

Using the published 17 m/s optimal cruise-speed value as a provisional TAS proxy:

| Assumed bank angle | Calculated air-relative radius |
|---:|---:|
| 15 deg | 110.0 m |
| 20 deg | 81.0 m |
| 25 deg | 63.2 m |
| 30 deg | 51.0 m |

**Classification:** `DERIVED_SCENARIO`; confidence low; not validated for Trinity Pro. The bank angles are hypothetical analysis points, not manufacturer-published operational limits. The published optimal cruise speed is not explicitly identified as TAS in the reviewed technical sheet; treating it as TAS is an assumption. Values apply only to wing-borne cruise, not hover or transition.

### Required handling in BlueSky PRO

1. Store these values as scenario curves/ranges with all assumptions, not as a single `minimum_turn_radius` field.
2. Preserve source speed and its reference ambiguity (TAS/IAS/GS unknown unless the source explicitly states it).
3. Mark assumed bank angle or lateral acceleration as `UNVALIDATED_ASSUMPTION`; never promote it to `OFFICIAL_SPEC`.
4. AUTO may use scenario values for preliminary candidate generation and sensitivity analysis only. A candidate relying on them remains `MOTION_MODEL_UNVERIFIED` and cannot pass the vehicle-motion hard gate or become flight-ready.
5. MANUAL may display the estimate as advisory information, while retaining the same explicit unverified status.
6. Once actual configuration values and representative flight-test evidence are available, create a separately versioned validated motion model. Do not silently overwrite the original assumptions.
7. Ground-track turns must be recomputed with wind; the tables above are air-relative idealized radii.

## 5. BlueSky PRO ingestion contract

For each concrete UAV configuration, store:
- manufacturer, model, variant, airframe category;
- flight mode / motion phase;
- minimum and maximum airspeed, cruise airspeed and speed reference (IAS/CAS/TAS/ground speed);
- maximum operational bank angle or validated lateral acceleration;
- maximum roll/yaw rates and angular accelerations, each with axis and units;
- minimum turn radius only when published or derived from validated inputs;
- source URL, document title/revision/date, retrieval date;
- parameter/configuration read-back identity and firmware version;
- payload/mass/CG applicability;
- validation state and operational margin;
- source class and derivation record.

Unknown fields remain null/UNKNOWN. AUTO planning must reject a maneuver that requires an unknown hard bound. MANUAL planning may preserve operator-entered geometry, but its candidate assessment must report the missing motion validation and must not mark the route flight-ready.

## 6. Initial population decision

| BlueSky generic type | Concrete reference | Published maneuver data usable now | Turn radius |
|---|---|---|---|
| UAV-TYPE-001 Multirotor | DJI Matrice 350 RTK | max horizontal speed, pitch angle, yaw rate | UNKNOWN |
| UAV-TYPE-002 Fixed-Wing / UAV-TYPE-003 VTOL | WingtraOne GEN II | cruise speed, wind limits, aircraft type | UNKNOWN |
| UAV-TYPE-003 VTOL / fixed-wing cruise | Quantum-Systems Trinity Pro | optimal cruise speed, phase-specific wind limits | UNKNOWN |

These are reference examples, not the complete operational fleet. Do not copy values from one concrete model to another generic type or variant.

## 7. Acceptance rules

1. No model-specific turn radius is emitted from these three manufacturer records.
2. M350 yaw rate is not treated as path curvature.
3. WingtraOne and Trinity Pro cruise speed is not treated as sufficient to derive bank-turn radius.
4. ArduPilot parameter ranges/defaults are not treated as manufacturer aircraft limits.
5. Derived values must retain input values, units, equation, assumptions, source and model revision.
6. Unknown or stale motion data blocks the relevant AUTO trajectory candidate.
7. A motion-feasible candidate still requires constrained-environment, terrain, wind, energy, NOTAM/airspace and independent integrity validation. It is not flight authorization.
