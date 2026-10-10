---
id: UAV-CONFIGURATION-REFERENCE-CATALOG-001
type: uav_configuration_reference_catalog
status: reference_only
---

# BlueSky PRO — UAV Configuration Reference Catalog 001

## 1. Purpose

Provide the first controlled reference population for the Administrator configuration model using manufacturer-documented data. These records are reference inputs for the configuration engine and are not BlueSky PRO certification approval by themselves.

## 2. Selection rule

Only explicit manufacturer-supported relationships shall be marked `COMPATIBLE`.

`UNKNOWN` shall remain unavailable to the operator and shall not be used by automatic configuration.

## 3. Reference UAV — DJI Matrice 350 RTK

| Parameter | Manufacturer-declared value | Provenance |
|---|---|---|
| Model | Matrice 350 RTK | DJI official specifications |
| Weight, with single downward gimbal, without batteries | approx. 3.77 kg | DJI official specifications |
| Weight, with two TB65 batteries | approx. 6.47 kg | DJI official specifications |
| Max takeoff weight | 9.2 kg | DJI official specifications |
| Max horizontal speed | 23 m/s | DJI official specifications |
| Max wind resistance | 12 m/s | DJI official specifications |
| Max flight time | 55 min, reference test condition | DJI official specifications |
| IP rating | IP55 | DJI official specifications |
| Operating temperature | -20 °C to +50 °C | DJI official specifications |
| GNSS | GPS + GLONASS + BeiDou + Galileo | DJI official specifications |
| RTK accuracy, horizontal | 1 cm + 1 ppm | DJI official specifications |
| RTK accuracy, vertical | 1.5 cm + 1 ppm | DJI official specifications |
| Single gimbal damper max payload | 960 g | DJI official specifications |

Official source: DJI Matrice 350 RTK Specifications.

## 4. M350 battery compatibility

### TB65

`M350 RTK ↔ TB65 = COMPATIBLE`

TB65 manufacturer-declared reference values:

- capacity: 5880 mAh;
- voltage: 44.76 V;
- energy: 263.2 Wh;
- chemistry: Li-ion;
- mass: approx. 1.35 kg;
- operating temperature: -20 °C to +50 °C.

DJI also states that M350 RTK can use two TB60 batteries simultaneously, but TB60 and TB65 must not be mixed during use.

Therefore the configuration engine shall model the battery set as a constrained set, not as independent battery selections.

## 5. M350 payload compatibility

The current DJI specification identifies support for:

- Zenmuse H30;
- Zenmuse H30T;
- Zenmuse H20;
- Zenmuse H20T;
- Zenmuse H20N;
- Zenmuse L2;
- Zenmuse L1;
- Zenmuse P1;
- Zenmuse S1;
- Zenmuse V1;
- certified third-party payloads developed based on the DJI Payload SDK.

The specification also defines supported gimbal arrangements including single downward, single upward, dual downward, downward + upward, and dual downward + upward configurations.

The configuration engine shall therefore represent payload placement/configuration separately from simple payload identity.

## 6. Reference UAV — DJI Matrice 300 RTK

| Parameter | Manufacturer-declared value | Provenance |
|---|---|---|
| Model | Matrice 300 RTK | DJI official specifications |
| Weight, with single downward gimbal, without batteries | approx. 3.6 kg | DJI official specifications |
| Weight with two TB60 batteries | approx. 6.3 kg | DJI official specifications |
| Max takeoff weight | 9 kg | DJI official specifications |
| Max horizontal speed | 23 m/s | DJI official specifications |
| Max wind resistance | 12 m/s | DJI official specifications |
| Max flight time | 55 min, reference test condition | DJI official specifications |
| IP rating | IP45 | DJI official specifications |
| Operating temperature | -20 °C to +50 °C | DJI official specifications |
| GNSS | GPS + GLONASS + BeiDou + Galileo | DJI official specifications |
| Single gimbal damper max payload | 930 g | DJI official specifications |

## 7. M300 battery compatibility

### TB60

`M300 RTK ↔ TB60 = COMPATIBLE`

DJI explicitly states that M300 RTK does not support TB50/TB55 and that TB60 is the applicable Intelligent Flight Battery.

The configuration engine shall not expose unsupported battery models for M300 RTK.

## 8. Reference UAV — DJI Matrice 30 Series

The DJI Matrice 30 Series is treated as a platform family with integrated mission sensors rather than as a generic interchangeable-gimbal platform.

| Parameter | Manufacturer-declared value | Provenance |
|---|---|---|
| Series | Matrice 30 Series | DJI official specifications |
| Dimensions, unfolded, excluding propellers | 470 × 585 × 215 mm | DJI official specifications |
| Dimensions, folded | 365 × 215 × 195 mm | DJI official specifications |
| Diagonal wheelbase | 668 mm | DJI official specifications |
| Weight, incl. two batteries | 3770 ± 10 g | DJI official specifications |
| Max takeoff weight | 4069 g | DJI official specifications |
| Max horizontal speed | 23 m/s | DJI official specifications |
| Max service ceiling, without other payload | 5000 m with 1671 propellers; 7000 m with 1676 propellers | DJI official specifications |
| Max wind resistance | 12 m/s | DJI official specifications |
| Max flight time | 41 min, reference test condition | DJI official specifications |
| Max hover time | 36 min | DJI official specifications |
| IP rating | IP55 | DJI official specifications |
| Operating temperature | -20 °C to +50 °C | DJI official specifications |
| GNSS | GPS + Galileo + BeiDou; GLONASS with RTK module | DJI official specifications |
| RTK accuracy | 1 cm + 1 ppm horizontal; 1.5 cm + 1 ppm vertical | DJI official specifications |
| Motor | 3511 | DJI official specifications |
| Propellers | 1671; 1676 High Altitude | DJI official specifications |

The M30 Series has an integrated sensor architecture; its mission capability shall therefore be represented as part of the aircraft variant rather than assumed to be interchangeable with M350/M300 gimbals.

## 9. M30 Series integrated equipment variants

### Matrice 30

The visible-light mission capability is represented by the manufacturer-defined aircraft configuration.

### Matrice 30T

The M30T adds the integrated thermal/visual/laser sensing capability documented by DJI. DJI identifies the M30T zoom camera as a 1/2-inch CMOS sensor with 48 MP effective pixels and describes the platform as having integrated infrared and laser rangefinding capability.

For the configuration engine:

`M30T → integrated visual + thermal + laser capability = FIXED/INTEGRATED`

It shall not be presented to the operator as an arbitrary external payload choice.

## 10. M30 battery compatibility

### TB30

`M30 Series ↔ TB30 = COMPATIBLE`

TB30 manufacturer-declared reference values:

- capacity: 5880 mAh;
- voltage: 26.1 V;
- cell configuration: Li-ion 6S;
- energy: 131.6 Wh;
- mass: approx. 685 g;
- operating temperature: -20 °C to +50 °C.

The aircraft uses two batteries. The configuration model shall treat the battery pair as a constrained aircraft power configuration.

## 11. Reference UAV — DJI Matrice 4 Series

DJI defines two aircraft variants in the Matrice 4 Series: Matrice 4T and Matrice 4E. Their primary mission sensors are integrated into the aircraft rather than represented as interchangeable external gimbals.

| Parameter | Manufacturer-declared value | Provenance |
|---|---|---|
| Variants | Matrice 4T / Matrice 4E | DJI official specifications and FAQ |
| Standard takeoff weight, incl. battery, propellers and microSD | 1219 g | DJI official specifications |
| Takeoff weight with low-noise propellers | 1229 g | DJI official specifications |
| Max takeoff weight, standard propellers | 1420 g | DJI official specifications |
| Max takeoff weight, low-noise propellers | 1430 g | DJI official specifications |
| Max payload | 200 g | DJI official specifications |
| Propeller size | 10.8 in | DJI official specifications |
| Diagonal wheelbase | 438.8 mm | DJI official specifications |
| Max horizontal speed | 21 m/s | DJI official specifications |
| Max altitude | 6000 m | DJI official specifications |
| Max operating altitude with payload | 4000 m | DJI official specifications |
| Max flight time, standard propellers | 49 min, reference test condition | DJI official specifications |
| Max flight time, low-noise propellers | 46 min, reference test condition | DJI official specifications |
| Max hover time, standard propellers | 42 min | DJI official specifications |
| Max hover time, low-noise propellers | 39 min | DJI official specifications |
| Max flight distance, standard propellers | 35 km, no wind reference condition | DJI official specifications |
| Max wind resistance | 12 m/s during takeoff and landing | DJI official specifications |
| Operating temperature | -10 °C to +40 °C | DJI official specifications |
| GNSS | GPS + Galileo + BeiDou + GLONASS with RTK module | DJI official specifications |
| RTK accuracy | 1 cm + 1 ppm horizontal; 1.5 cm + 1 ppm vertical | DJI official specifications |
| Transmission | O4 Enterprise | DJI official specifications |
| Max transmission distance | 25 km FCC; 12 km CE/SRRC/MIC, unobstructed reference condition | DJI official specifications |

## 12. Matrice 4E / 4T integrated mission equipment

### Matrice 4E

Manufacturer-declared integrated imaging capability:

- wide camera: 4/3-inch CMOS, 20 MP;
- medium tele camera: 1/1.3-inch CMOS, 48 MP;
- telephoto camera: 1/1.5-inch CMOS, 48 MP;
- laser rangefinding module;
- mechanical shutter on the wide camera;
- minimum photo interval: 0.5 s.

DJI states that Matrice 4E supports precise mapping after enabling RTK. Therefore the automatic configurator may classify M4E as a mapping-capable reference platform when the mission requirements and applicable operational conditions are satisfied.

### Matrice 4T

Manufacturer-declared integrated imaging capability:

- wide camera: 1/1.3-inch CMOS, 48 MP;
- medium tele camera: 1/1.3-inch CMOS, 48 MP;
- telephoto camera: 1/1.5-inch CMOS, 48 MP;
- thermal camera: 640 × 512, VOx;
- NIR auxiliary light;
- laser rangefinding module;
- minimum photo interval: 0.7 s.

DJI specifies thermal measurement capability and laser rangefinding. The M4T shall therefore be represented as a fixed integrated thermal/visual/laser configuration rather than as a platform to which arbitrary payloads are attached.

## 13. Matrice 4 battery

`Matrice 4E/4T ↔ Matrice 4 Series Intelligent Flight Battery = COMPATIBLE`

Manufacturer-declared reference values:

- capacity: 6741 mAh;
- standard voltage: 14.76 V;
- maximum charging voltage: 17.0 V;
- cell type: Li-ion 4S;
- energy: 99.5 Wh;
- weight: 401 g;
- discharge rate: 4C;
- cycle count: 200;
- operating/recharging temperature restrictions as specified by DJI.

DJI states that Matrice 4 Series batteries can only be charged using the Matrice 4 Series Charging Hub. The battery therefore belongs to the dedicated Matrice 4 power family.

## 14. Matrice 4 accessories

DJI identifies the following official aircraft accessories as compatible with Matrice 4 Series:

- DJI AL1 Spotlight;
- DJI AS1 Speaker.

Manufacturer-declared reference values relevant to configuration/energy assessment:

| Accessory | Mass | Max power | Relationship |
|---|---:|---:|---|
| AL1 Spotlight | 99 g incl. bracket | 32 W | COMPATIBLE with Matrice 4 Series |
| AS1 Speaker | 92.5 g incl. bracket | 15 W | COMPATIBLE with Matrice 4 Series |

When both accessories are installed, the configurator shall evaluate the resulting configuration rather than assuming the base-aircraft endurance remains unchanged. DJI specifically recommends reducing spotlight brightness as needed to minimize impact on flight time.

## 15. Cross-platform equipment relationships

The catalog shall distinguish aircraft-integrated equipment from interchangeable payloads.

| UAV | Component / capability | Relationship | Rule |
|---|---|---|---|
| M350 RTK | TB65 | COMPATIBLE | two-battery aircraft configuration |
| M350 RTK | TB60 | COMPATIBLE | two TB60; do not mix TB60/TB65 |
| M350 RTK | TB65 + TB60 mixed set | INCOMPATIBLE | explicitly prohibited |
| M300 RTK | TB60 | COMPATIBLE | supported battery |
| M300 RTK | TB50 | INCOMPATIBLE | manufacturer states not supported |
| M300 RTK | TB55 | INCOMPATIBLE | manufacturer states not supported |
| M30 Series | TB30 | COMPATIBLE | two-battery aircraft configuration |
| M4E | Matrice 4 Series Intelligent Flight Battery | COMPATIBLE | dedicated battery family |
| M4T | Matrice 4 Series Intelligent Flight Battery | COMPATIBLE | dedicated battery family |
| M350 RTK | listed DJI gimbals | COMPATIBLE | subject to supported mounting arrangement |
| M30 Series | M350/M300 external gimbal payload | NOT ASSUMED | integrated-sensor platform; requires explicit manufacturer compatibility evidence |
| M4E/M4T | arbitrary external gimbal payload | NOT ASSUMED | requires explicit manufacturer compatibility evidence |
| M4E/M4T | AL1 Spotlight | COMPATIBLE | official Matrice 4 accessory |
| M4E/M4T | AS1 Speaker | COMPATIBLE | official Matrice 4 accessory |
| M350 RTK | non-certified third-party payload | UNKNOWN / EXCLUDED | only certified Payload SDK payloads may qualify |

## 16. Configuration-engine rule

The reference catalog is not a list from which the operator may freely combine records.

The engine shall evaluate the relationship graph first:

```text
UAV MODEL / VARIANT
       ↓
INTEGRATED OR EXTERNAL EQUIPMENT
       ↓
BATTERY COMPATIBILITY
       ↓
PAYLOAD / ACCESSORY COMPATIBILITY
       ↓
MOUNT / INTERFACE
       ↓
MASS / MTOW
       ↓
POWER / ENERGY
       ↓
NAVIGATION / COMMUNICATION
       ↓
MISSION REQUIREMENTS
       ↓
VALID CONFIGURATION SET
```

Only records surviving all mandatory checks enter the operator-facing candidate set.

## 17. Automatic mission selection

The operational interface shall preferably request mission requirements rather than individual hardware choices.

The engine shall:

1. identify UAV models/variants capable of the mission;
2. obtain only compatible equipment candidates;
3. construct only valid configurations;
4. reject configurations violating limits;
5. evaluate configuration-specific energy behaviour;
6. rank valid configurations against mission objectives;
7. return the recommended configuration and valid alternatives.

## 18. Data provenance

Each populated parameter must retain:

- source organization;
- source document;
- source revision/date;
- exact applicability to UAV/component;
- BlueSky record revision;
- qualification state.

The official manufacturer specifications and compatibility documentation are external source evidence; they are not silently converted into BlueSky PRO acceptance criteria.

## 19. Sources

- DJI Matrice 350 RTK official specifications: https://enterprise.dji.com/matrice-350-rtk/specs
- DJI Matrice 350 RTK official FAQ: https://enterprise.dji.com/matrice-350-rtk/faq
- DJI Matrice 350 RTK Download Center, including accessory compatibility documentation: https://www.dji.com/downloads/products/matrice-350-rtk
- DJI Matrice 300 RTK official specifications/support: https://www.dji.com/support/product/matrice-300
- DJI Matrice 300 RTK Download Center, including accessory compatibility documentation: https://www.dji.com/downloads/products/matrice-300
- DJI Matrice 30 Series official specifications: https://enterprise.dji.com/matrice-30/specs
- DJI Matrice 4 Series official specifications: https://enterprise.dji.com/matrice-4-series/specs
- DJI Matrice 4 Series official FAQ: https://enterprise.dji.com/matrice-4-series/faq

## 20. Status

**REFERENCE_ONLY — expanded official-data seed.**

This catalog now provides a broader reference population for the Administrator configuration model. Additional UAV families and component records shall be added only when their manufacturer data and compatibility relationships are controlled. No record becomes an approved operational configuration solely by appearing in this reference catalog.
