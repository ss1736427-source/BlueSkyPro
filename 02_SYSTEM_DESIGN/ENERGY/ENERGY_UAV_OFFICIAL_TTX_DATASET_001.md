# BlueSky PRO — Official UAV TTX Dataset 001

**Document ID:** ENERGY-UAV-TTX-DATASET-001  
**Status:** REFERENCE / CANDIDATE DATASET  
**Purpose:** систематизация опубликованных производителями ТТХ БВС и батарей для последующего построения Energy Model.  
**Rule:** значения ниже являются официально заявленными характеристиками конкретных изделий и конфигураций. Они не являются автоматически принятыми параметрами BlueSky PRO и не заменяют измеренные эксплуатационные данные.

## 1. Source hierarchy

1. Manufacturer official specification page.
2. Manufacturer official user / maintenance manual.
3. Manufacturer official battery documentation.
4. Other technical sources only as secondary references.

## 2. Reference platforms

Для первой численной базы выбраны четыре серийные платформы DJI разных размерных классов:

- Matrice 350 RTK + TB65
- Matrice 300 RTK + TB60
- Matrice 30 Series + TB30
- Mavic 3 Enterprise Series + Mavic 3 Series Intelligent Flight Battery / EU C1 battery

Выбор не означает, что BlueSky PRO ограничивается DJI. Платформы используются как документированные reference configurations для проверки структуры модели.

---

# 3. DJI Matrice 350 RTK + TB65

## 3.1 Aircraft

| Parameter | Official value | Unit / condition |
|---|---:|---|
| Unfolded dimensions, propellers excluded | 810 × 670 × 430 | mm |
| Folded dimensions, propellers included | 430 × 420 × 430 | mm |
| Diagonal wheelbase | 895 | mm |
| Weight, single downward gimbal, without batteries | approx. 3.77 | kg |
| Weight, with two TB65 batteries | approx. 6.47 | kg |
| Single gimbal damper max payload | 960 | g |
| Maximum takeoff weight | 9.2 | kg |
| Max angular velocity, pitch | 300 | °/s |
| Max angular velocity, yaw | 100 | °/s |
| Max pitch angle | 30 | ° |
| Max pitch angle, N mode + forward vision | 25 | ° |
| Max ascent speed | 6 | m/s |
| Max descent speed, vertical | 5 | m/s |
| Max tilted descent speed | 7 | m/s |
| Max horizontal speed | 23 | m/s |
| Max flight altitude | 5000 | m; 2110s props, takeoff weight ≤7.4 kg |
| Max flight altitude | 7000 | m; 2112 high-altitude props, takeoff weight ≤7.2 kg |
| Max wind resistance | 12 | m/s |
| Max flight time | 55 | min; approx. 8 m/s, no payload, windless, battery to 0% |
| Ingress protection | IP55 | declared rating |
| GNSS | GPS + GLONASS + BeiDou + Galileo | — |
| Operating temperature | -20…+50 | °C |
| EU class | C3 | — |
| RTK fixed accuracy, horizontal | 1 cm + 1 ppm | — |
| RTK fixed accuracy, vertical | 1.5 cm + 1 ppm | — |
| Hovering accuracy, RTK | ±0.1 | m |
| Hovering accuracy, GNSS | ±1.5 horizontal / ±0.5 vertical | m |
| Hovering accuracy, vision | ±0.3 horizontal / ±0.1 vertical | m |

## 3.2 Propulsion / payload interfaces

| Parameter | Official value |
|---|---|
| Supported gimbals | H30, H30T, H20, H20T, H20N, L2, L1, P1, S1, V1 |
| Gimbal configurations | single downward; single upward; dual downward; downward + upward; dual downward + single upward |
| Third-party payload | certified payloads developed based on DJI Payload SDK |

## 3.3 TB65 battery

| Parameter | Official value | Unit |
|---|---:|---|
| Model | TB65 | — |
| Capacity | 5880 | mAh |
| Voltage | 44.76 | V |
| Type | Li-ion | — |
| Energy | 263.2 | Wh |
| Weight | approx. 1.35 | kg |
| Operating temperature | -20…+50 | °C |
| Ideal storage temperature | 22…30 | °C |
| Charging temperature | -20…+40 | °C |
| Low-temperature heating | below 5 °C | auto-heating stated by manufacturer |
| Recommended charging temperature | 15…35 | °C |
| Charging time, 220 V, two batteries 0→100% | approx. 60 | min |
| Charging time, 220 V, two batteries 20→90% | approx. 30 | min |
| Charging time, 110 V, two batteries 0→100% | approx. 70 | min |
| Charging time, 110 V, two batteries 20→90% | approx. 40 | min |

## 3.4 Navigation / sensing / communications values useful to system modeling

| Parameter | Official value |
|---|---|
| Video transmission | DJI O3 Enterprise |
| Max transmission distance, unobstructed | 20 km FCC / 8 km CE/SRRC/MIC |
| Vision obstacle sensing | forward/back/left/right 0.7–40 m; up/down 0.6–30 m |
| Infrared sensing | 0.1–8 m |
| FPV camera | 1080p, 142° FOV, 30 fps |
| Operating frequency | 2.4000–2.4835 GHz; 5.150–5.250 GHz; 5.725–5.850 GHz, subject to regional rules |

**Source:** DJI Matrice 350 RTK official specifications and official download center.

---

# 4. DJI Matrice 300 RTK + TB60

## 4.1 Aircraft

| Parameter | Official value | Unit / condition |
|---|---:|---|
| Unfolded dimensions, propellers excluded | 810 × 670 × 430 | mm |
| Folded dimensions, propellers included | 430 × 420 × 430 | mm |
| Diagonal wheelbase | 895 | mm |
| Weight, single downward gimbal, without batteries | approx. 3.6 | kg |
| Weight, with two TB60 batteries | approx. 6.3 | kg |
| Single gimbal damper max payload | 930 | g |
| Maximum takeoff weight | 9 | kg |
| Max angular velocity, pitch | 300 | °/s |
| Max angular velocity, yaw | 100 | °/s |
| Max pitch angle | 30 | ° |
| Max pitch angle, P mode + forward vision | 25 | ° |
| Max ascent speed, S mode | 6 | m/s |
| Max ascent speed, P mode | 5 | m/s |
| Max descent speed, vertical, S mode | 5 | m/s |
| Max descent speed, vertical, P mode | 4 | m/s |
| Max tilted descent speed, S mode | 7 | m/s |
| Max speed, S mode | 23 | m/s |
| Max speed, P mode | 17 | m/s |
| Service ceiling | 5000 | m; 2110 props, takeoff weight ≤7 kg |
| Service ceiling | 7000 | m; 2195 props, takeoff weight ≤7 kg |
| Max wind resistance | 12 | m/s |
| Max flight time | 55 | min |
| Ingress protection | IP45 | declared rating |
| GNSS | GPS + GLONASS + BeiDou + Galileo | — |
| Operating temperature | -20…+50 | °C |
| RTK fixed accuracy, horizontal | 1 cm + 1 ppm | — |
| RTK fixed accuracy, vertical | 1.5 cm + 1 ppm | — |
| Hovering accuracy, RTK | ±0.1 | m |
| Hovering accuracy, GPS | ±1.5 horizontal / ±0.5 vertical | m |
| Hovering accuracy, vision | ±0.3 horizontal / ±0.1 vertical | m |

## 4.2 Payload / camera support

| Parameter | Official value |
|---|---|
| Supported gimbals | XT2, XT, Z30, H20, H20T, P1, L1 |
| Gimbal configurations | single downward; dual downward; single upward; upward + downward; triple gimbals |
| FPV camera | 960p, 145° FOV, 30 fps |
| Infrared ToF sensing | 0.1–8 m; 30° FOV |

## 4.3 TB60 battery

| Parameter | Official value | Unit |
|---|---:|---|
| Model | TB60 | — |
| Capacity | 5935 | mAh |
| Voltage | 52.8 | V |
| Battery type | LiPo 12S | — |
| Energy | 274 | Wh |
| Net weight | approx. 1.35 | kg |
| Operating temperature | -20…+50 | °C |
| Ideal storage temperature | 22…30 | °C |
| Charging temperature | -20…+40 | °C |
| Low-temperature heating | below 5 °C | automatic heating stated |
| Charging time, 220 V, two batteries 0→100% | 60 | min |
| Charging time, 220 V, two batteries 20→90% | 30 | min |
| Charging time, 110 V, two batteries 0→100% | 70 | min |
| Charging time, 110 V, two batteries 20→90% | 40 | min |

**Source:** DJI Matrice 300 RTK official specifications and official download center.

---

# 5. DJI Matrice 30 Series + TB30

## 5.1 Aircraft

| Parameter | Official value | Unit / condition |
|---|---:|---|
| Unfolded dimensions | 470 × 585 × 215 | mm |
| Folded dimensions | 365 × 215 × 195 | mm |
| Diagonal wheelbase | 668 | mm |
| Weight, incl. two batteries | 3770 ± 10 | g |
| Max takeoff weight | 4069 | g |
| Max takeoff weight, EU C2 | 3998 | g |
| Max angular velocity, pitch | 150 | °/s |
| Max angular velocity, yaw | 100 | °/s |
| Max pitch angle | 35 | ° |
| Max pitch angle, N mode + forward vision | 25 | ° |
| Max ascent speed | 6 | m/s |
| Max descent speed | 5 | m/s |
| Max tilted descent speed | 7 | m/s |
| Max horizontal speed | 23 | m/s |
| Max service ceiling | 5000 | m; 1671 props |
| Max service ceiling | 7000 | m; 1676 props |
| Max wind resistance | 12 | m/s |
| Max hover time | 36 | min |
| Max flight time | 41 | min |
| Motor model | 3511 | — |
| Propeller models | 1671; 1676 High Altitude | — |
| Ingress protection | IP55 | declared rating |
| GNSS | GPS + Galileo + BeiDou + GLONASS | GLONASS with RTK module |
| Operating temperature | -20…+50 | °C |
| EU class | C2 | — |
| RTK fixed accuracy, horizontal | 1 cm + 1 ppm | — |
| RTK fixed accuracy, vertical | 1.5 cm + 1 ppm | — |
| Hovering accuracy, RTK | ±0.1 | m |
| Hovering accuracy, GPS | ±1.5 horizontal / ±0.5 vertical | m |
| Hovering accuracy, vision | ±0.3 horizontal / ±0.1 vertical | m |

## 5.2 TB30 battery

| Parameter | Official value | Unit |
|---|---:|---|
| Model | TB30 | — |
| Capacity | 5880 | mAh |
| Voltage | 26.1 | V |
| Battery type | Li-ion 6S | — |
| Energy | 131.6 | Wh |
| Net weight | approx. 685 | g |
| Operating temperature | -20…+50 | °C |
| Storage temperature | 20…30 | °C |
| Charging temperature | -20…+40 | °C |
| Low-temperature heating | below 10 °C | automatic heating stated |
| Chemical system | LiNiMnCoO2 | — |

## 5.3 Payload / sensing

| Parameter | Official value |
|---|---|
| Zoom camera sensor | 1/2-inch CMOS, 48 MP |
| Wide camera sensor | 1/2-inch CMOS, 12 MP |
| Thermal imager | uncooled VOx microbolometer |
| Thermal NETD | ≤50 mK @ F1.0 |
| Thermal normal resolution | 640 × 512 |
| Thermal super-resolution | 1280 × 1024 |
| FPV camera | 1920 × 1080, 161° DFOV, 30 fps |
| Laser module | 905 nm |
| Laser measurement range | 3–1200 m under specified reference target |
| Vision obstacle sensing | forward 0.6–38 m; other directions 0.5–33 m |
| Infrared sensing | 0.1–10 m |

**Source:** DJI Matrice 30 Series official specifications.

---

# 6. DJI Mavic 3 Enterprise Series

## 6.1 Aircraft

| Parameter | Official value | Unit / condition |
|---|---:|---|
| Weight Mavic 3E | 915 | g, with propellers, without accessories |
| Weight Mavic 3T/3TA | 920 | g, same condition |
| Max takeoff weight | 1050 | g |
| EU C1 battery configuration MTOW | 899 | g |
| Folded dimensions | 221 × 96.3 × 90.3 | mm |
| Unfolded dimensions | 347.5 × 283 × 107.7 | mm |
| Diagonal distance | 380.1 | mm |
| Max ascent, Normal | 6 | m/s |
| Max ascent, Sport | 8 | m/s |
| Max descent, Normal | 6 | m/s |
| Max descent, Sport | 6 | m/s |
| Max speed, Normal | 15 | m/s |
| Max speed, Sport, forward | 21 | m/s |
| Max speed, Sport, side | 20 | m/s |
| Max speed, Sport, backward | 19 | m/s |
| Max wind resistance | 12 | m/s |
| Max takeoff altitude | 6000 | m without payload |
| Max flight time | 45 | min, no wind, standard battery/props |
| Max flight time, EU C1 configuration | 36 | min |
| Max hover time | 38 | min, no wind |
| Max hover time, EU C1 configuration | 30 | min |
| Max flight distance | 32 | km, standard battery/props |
| Max flight distance, EU C1 configuration | 24 | km |
| Max pitch angle, Normal | 30 | ° |
| Max pitch angle, Sport | 35 | ° |
| Max angular velocity | 200 | °/s |
| GNSS | GPS + Galileo + BeiDou + GLONASS | GLONASS with RTK module |
| Operating temperature | -10…+40 | °C |
| Motor model | 2008 | — |
| Propeller model | 9453F Enterprise | — |
| EU class | C2 | — |

## 6.2 Standard battery

| Parameter | Official value | Unit |
|---|---:|---|
| Battery | DJI Mavic 3 Series Intelligent Flight Battery | — |
| Capacity | 5000 | mAh |
| Standard voltage | 15.4 | V |
| Max charging voltage | 17.6 | V |
| Type | LiPo 4S | — |
| Chemical system | LiCoO2 | — |
| Energy | 77 | Wh |
| Weight | 335.5 | g |
| Charging temperature | 5…40 | °C |

## 6.3 EU C1 battery

| Parameter | Official value | Unit |
|---|---:|---|
| Capacity | 4230 | mAh |
| Standard voltage | 15.4 | V |
| Max charging voltage | 17.6 | V |
| Type | LiPo 4S | — |
| Chemical system | LiCoO2 | — |
| Energy | 65.15 | Wh |
| Weight | 285 | g |

## 6.4 Mavic 3E / 3T / 3TA payload data

| Parameter | Official value |
|---|---|
| Mavic 3E wide camera | 4/3 CMOS, 20 MP |
| Mavic 3T/3TA wide camera | 1/2-inch CMOS, 48 MP |
| Tele camera | 1/2-inch CMOS, 12 MP; 162 mm equivalent focal length |
| Thermal camera, 3T | 640 × 512, 30 Hz, ≤50 mK NETD |
| Thermal temperature range | -20…150 °C high gain; 0…500 °C low gain |
| Thermal camera, 3TA | 8 μm pixel pitch, DFOV 41.2° |
| Laser rangefinder | not listed on the cited general aircraft specification as a primary parameter set |
| Video transmission | DJI O3 Enterprise |
| Max transmission distance | 15 km FCC / 8 km CE/SRRC/MIC |
| Transmission latency | approx. 200 ms, environment/device dependent |
| Obstacle sensing | forward/back/lateral/upward 0.5–38 m; downward 0.3–18 m under stated conditions |

**Source:** DJI Mavic 3 Enterprise official specifications.

---

# 7. Cross-platform normalized dataset

| Normalized parameter | M350/TB65 | M300/TB60 | M30/TB30 | Mavic 3E/std battery |
|---|---:|---:|---:|---:|
| Aircraft class / mass scale | heavy enterprise | heavy enterprise | medium enterprise | compact enterprise |
| Aircraft mass with stated batteries | 6.47 kg / 2 TB65 | 6.3 kg / 2 TB60 | 3.77 kg / 2 TB30 | 0.915 kg aircraft; battery separate in source definition |
| MTOW | 9.2 kg | 9 kg | 4.069 kg | 1.05 kg |
| Battery voltage | 44.76 V | 52.8 V | 26.1 V | 15.4 V |
| Battery capacity | 5880 mAh | 5935 mAh | 5880 mAh | 5000 mAh |
| Battery energy | 263.2 Wh | 274 Wh | 131.6 Wh | 77 Wh |
| Battery count in stated aircraft configuration | 2 | 2 | 2 | 1 |
| Max flight time | 55 min | 55 min | 41 min | 45 min |
| Max wind resistance | 12 m/s | 12 m/s | 12 m/s | 12 m/s |
| Max horizontal speed | 23 m/s | 23 m/s | 23 m/s | 15 m/s Normal; 21 m/s forward Sport |
| Operating temperature | -20…+50 °C | -20…+50 °C | -20…+50 °C | -10…+40 °C |
| IP rating | IP55 | IP45 | IP55 | not used as a core energy-model input |

---

# 8. Derived reference values — explicitly NOT baseline values

The following can be mathematically derived from manufacturer-declared values, but they must not be treated as physical consumption coefficients.

## 8.1 Battery energy cross-check

For M350 TB65:

`5.880 Ah × 44.76 V = 263.19 Wh ≈ 263.2 Wh`

For M300 TB60:

`5.935 Ah × 52.8 V = 313.37 Wh`

**Important:** this arithmetic result conflicts with DJI's published TB60 energy value of 274 Wh. Therefore the published 274 Wh must remain the authoritative manufacturer-declared value for the reference dataset; the discrepancy itself shall be recorded as a source-data reconciliation item rather than silently corrected.

For M30 TB30:

`5.880 Ah × 26.1 V = 153.47 Wh`

This also differs from DJI's published 131.6 Wh. Again, use the manufacturer's declared energy value as the reference value and record the arithmetic discrepancy.

For Mavic 3 standard battery:

`5.000 Ah × 15.4 V = 77.0 Wh`

This matches the published 77 Wh.

## 8.2 Energy-per-minute reference quotient

The quotient `declared battery energy / declared maximum flight time` is permitted only as a diagnostic reference, not as a consumption model:

- M350/TB65: 263.2 / 55 ≈ 4.79 Wh/min per battery, using one TB65 energy value as published.
- M300/TB60: 274 / 55 ≈ 4.98 Wh/min per battery.
- M30/TB30: 131.6 / 41 ≈ 3.21 Wh/min per battery.
- Mavic 3E: 77 / 45 ≈ 1.71 Wh/min.

These quotients are not directly comparable because flight-time tests, payload, propulsion state, battery configuration and environmental conditions differ.

---

# 9. Parameters directly useful for BlueSky PRO Energy Model

## 9.1 Candidate static parameters

- MTOW.
- Aircraft empty / stated configuration mass.
- Battery mass.
- Battery count.
- Nominal capacity.
- Manufacturer-declared energy.
- Nominal voltage.
- Battery chemistry/type.
- Operating temperature range.
- Propeller model.
- Motor model where published.
- Published maximum horizontal speed.
- Published maximum ascent/descent speeds.
- Published wind resistance.
- Published maximum flight time and its exact test condition.
- Payload / gimbal configuration limits.

## 9.2 Parameters requiring operational telemetry or dedicated engineering data

The official TTX collected here do **not** provide a complete validated consumption function for arbitrary mission conditions. The following remain data-acquisition/model-identification items:

- instantaneous electrical power;
- voltage/current under load;
- SOC-to-energy relationship under load;
- usable energy versus temperature;
- degradation / SOH model;
- energy consumption versus airspeed;
- energy consumption versus groundspeed;
- wind-induced power increment;
- payload-induced power increment;
- altitude dependence;
- climb/descent energy model;
- hover power under defined mass/environment conditions;
- propulsion efficiency map;
- reserve / uncertainty model;
- return-feasibility acceptance thresholds.

---

# 10. Source-data rules for BlueSky PRO

1. Preserve manufacturer values exactly with units and test conditions.
2. Never convert a maximum flight time into a generic endurance coefficient without its test conditions.
3. Never infer usable energy from nominal energy alone.
4. Never infer degradation coefficients from cycle-life statements alone.
5. Keep aircraft, battery, payload, propeller and firmware/configuration as separate configuration dimensions.
6. Record source revision/date when a parameter is baselined.
7. Any conflict between manufacturer-declared values and arithmetic reconstruction is a controlled reconciliation item.
8. The reference dataset is evidence for model construction, not evidence that BlueSky PRO's resulting model is validated.

---

# 11. Current status

`ENERGY-UAV-TTX-DATASET-001` = REFERENCE DATASET COMPLETE FOR CURRENT SELECTED PLATFORMS.

The dataset is sufficient to define the static input schema and to identify the missing dynamic parameters. It is **not** sufficient by itself to close `NAV-TV-009`.

## Primary official sources

- DJI Matrice 350 RTK — official specifications.
- DJI Matrice 350 RTK — official Download Center / User Manual.
- DJI Matrice 300 RTK — official specifications.
- DJI Matrice 300 RTK — official Download Center / User Manual / Maintenance Manual.
- DJI Matrice 30 Series — official specifications.
- DJI Mavic 3 Enterprise Series — official specifications.
