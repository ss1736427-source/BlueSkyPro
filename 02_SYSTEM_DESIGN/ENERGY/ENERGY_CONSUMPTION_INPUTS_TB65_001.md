---
id: ENERGY-CONSUMPTION-INPUTS-TB65-001
type: engineering_parameter_input_baseline
status: candidate_for_engineering_review
parent: ENERGY-CANDIDATE-DATASET-TB65-001
---

# BlueSky PRO — Energy Consumption Model Inputs — TB65 Candidate Set 001

## 1. Purpose

Define the first concrete input set required to connect the Matrice 350 RTK + TB65 candidate dataset to the Energy Consumption Model without introducing unsupported universal coefficients.

## 2. Controlled configuration

| Item | Candidate configuration | Status |
|---|---|---|
| UAV | DJI Matrice 350 RTK | Reference configuration |
| Battery | TB65 Intelligent Flight Battery | Reference configuration |
| Battery architecture | Two TB65 batteries used simultaneously by aircraft | Manufacturer configuration |
| Propeller | 2110s standard / 2112 high-altitude-low-noise | Configuration-dependent |
| Payload | Mission-dependent | TBD per controlled configuration |

DJI specifies a maximum flight time of 55 minutes for M350 RTK at approximately 8 m/s, without payload, in windless conditions, until battery level reached 0%; DJI explicitly states actual usage varies with flight mode, accessories and environment. This value is therefore a manufacturer reference point, not a BlueSky PRO consumption coefficient. citeturn0search0turn0search1

## 3. Input classification

### 3.1 Direct manufacturer/reference inputs

| Input | Unit | Candidate source | Status |
|---|---:|---|---|
| Battery nominal capacity | mAh | DJI TB65 specification | SOURCE |
| Battery nominal voltage | V | DJI TB65 specification | SOURCE |
| Battery declared energy | Wh | DJI TB65 specification | SOURCE |
| Battery mass | kg | DJI TB65 specification | SOURCE |
| Aircraft mass without batteries | kg | DJI specification | SOURCE |
| Aircraft mass with two TB65 batteries | kg | DJI specification | SOURCE |
| Maximum takeoff mass | kg | DJI specification | SOURCE |
| Maximum horizontal speed | m/s | DJI specification | SOURCE |
| Maximum wind resistance | m/s | DJI specification | SOURCE |
| Manufacturer reference flight-time condition | m/s, payload, wind | DJI specification | SOURCE |

The current official DJI specification lists TB65 as 5880 mAh, 44.76 V and 263.2 Wh, with approximately 1.35 kg mass; the aircraft is listed at approximately 3.77 kg without batteries and approximately 6.47 kg with two TB65 batteries. citeturn0search0

### 3.2 Runtime / telemetry inputs

These are required by the BlueSky PRO runtime model but are not treated as universal constants:

- current battery state / SOC;
- battery temperature;
- battery health / cycle information where available;
- voltage and current telemetry where available;
- validity and freshness of telemetry;
- active flight regime;
- actual airspeed, if available;
- actual groundspeed;
- wind vector / wind estimate;
- aircraft mass/configuration;
- payload/equipment state.

## 4. Derived quantities

The following shall be calculated by the model rather than copied from a nameplate:

```text
Available Energy
Usable Energy
Instantaneous / interval power demand
Energy consumed over route segment
Predicted energy for return route
Energy reserve / margin
Return feasibility
```

## 5. Consumption-model structure

For each route segment the implementation shall preserve a separation between:

```text
Aircraft configuration
+ Payload
+ Flight regime
+ Airspeed / Groundspeed
+ Wind
+ Altitude / environmental state
+ Battery state
        ↓
Power / energy demand model
        ↓
Segment energy
        ↓
Mission energy accumulation
```

No universal Wh/km, Wh/min, power coefficient, wind correction coefficient, temperature derating coefficient or reserve percentage shall be inserted at this stage.

## 6. Calibration dataset required

To turn the candidate input set into an engineering baseline, the following measurements/data are required for the controlled configuration:

1. Battery state before flight.
2. Battery state after defined flight segments.
3. Voltage/current or equivalent energy telemetry at controlled sampling interval.
4. Flight time per segment.
5. Airspeed and groundspeed.
6. Wind direction and speed.
7. Altitude/profile.
8. Payload and configuration.
9. Battery temperature.
10. Battery cycle/health information.
11. Firmware/software configuration relevant to telemetry interpretation.
12. Environmental conditions and test identification.

## 7. Acceptance of external reference flight time

The 55-minute DJI figure may be used only as a **reference consistency check** for the candidate configuration because its stated test condition is specific: approximately 8 m/s, no payload, no wind, and discharge to 0%. It shall not be used as an operational reserve or as evidence that a BlueSky PRO return is feasible. citeturn0search0

## 8. Required next engineering action

The next controlled artifact shall contain a parameterized calculation example using measured or manufacturer-supported inputs and shall explicitly identify every remaining unknown coefficient.

Until that dataset exists:

```text
Consumption Model = DEFINED
Numerical calibration = NOT COMPLETE
Return Energy Model = STRUCTURALLY DEFINED
NAV-TV-009 = NOT READY FOR EXECUTION
```
