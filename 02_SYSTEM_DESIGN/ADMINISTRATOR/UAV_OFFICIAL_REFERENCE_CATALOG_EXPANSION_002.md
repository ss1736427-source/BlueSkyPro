# UAV OFFICIAL REFERENCE CATALOG EXPANSION 002

## Purpose

Expand the BlueSky PRO reference UAV catalogue using manufacturer-published specifications. This catalogue is a source layer for the Administrator configuration database; it is not itself an approval for operation or certification.

## Source classification

- `OFFICIAL_SPEC` — value published by the manufacturer in an official specification/manual.
- `OFFICIAL_COMPATIBILITY` — compatibility explicitly stated by the manufacturer.
- `DERIVED` — value calculated from official values; must retain the derivation.
- `UNKNOWN` — not established by the reviewed official source.

Unknown values shall not be replaced by assumptions.

## 1. DJI Matrice 350 RTK

| Parameter | Official value | Source class |
|---|---:|---|
| Unfolded dimensions, without propellers | 810 × 670 × 430 mm | OFFICIAL_SPEC |
| Weight, without batteries | approx. 3.77 kg | OFFICIAL_SPEC |
| Weight, with two TB65 batteries | approx. 6.47 kg | OFFICIAL_SPEC |
| Single gimbal damper max payload | 960 g | OFFICIAL_SPEC |
| Max takeoff weight | 9.2 kg | OFFICIAL_SPEC |
| Battery | TB65 | OFFICIAL_SPEC / OFFICIAL_COMPATIBILITY |
| Operating temperature | -20 to 50 °C | OFFICIAL_SPEC |
| GNSS | GPS + Galileo + BeiDou + GLONASS* | OFFICIAL_SPEC |
| RTK positioning accuracy | 1 cm + 1 ppm horizontal; 1.5 cm + 1 ppm vertical | OFFICIAL_SPEC |

`*` GLONASS support is stated by DJI as available when the RTK module is enabled.

### Configuration rule

Two TB65 batteries are the documented battery configuration. TB60 + TB65 mixed operation shall be treated as incompatible unless a newer manufacturer document explicitly changes this rule.

## 2. DJI Matrice 30 Series

| Parameter | Official value | Source class |
|---|---:|---|
| Dimensions, unfolded, excl. propellers | 470 × 585 × 215 mm | OFFICIAL_SPEC |
| Dimensions, folded | 365 × 215 × 195 mm | OFFICIAL_SPEC |
| Weight incl. two batteries | 3770 ± 10 g | OFFICIAL_SPEC |
| Max takeoff weight | 4069 g | OFFICIAL_SPEC |
| EU C2 max takeoff weight | 3998 g | OFFICIAL_SPEC |
| Max horizontal speed | 23 m/s | OFFICIAL_SPEC |
| Max service ceiling, 1671 propellers | 5000 m | OFFICIAL_SPEC |
| Max service ceiling, 1676 propellers | 7000 m | OFFICIAL_SPEC |
| Max wind resistance | 12 m/s | OFFICIAL_SPEC |
| Max flight time | 41 min | OFFICIAL_SPEC |
| Max hover time | 36 min | OFFICIAL_SPEC |
| Motor | 3511 | OFFICIAL_SPEC |
| Propellers | 1671 / 1676 high-altitude | OFFICIAL_SPEC |
| IP rating | IP55 | OFFICIAL_SPEC |
| Operating temperature | -20 to 50 °C | OFFICIAL_SPEC |
| GNSS | GPS + Galileo + BeiDou + GLONASS* | OFFICIAL_SPEC |
| RTK accuracy | 1 cm + 1 ppm horizontal; 1.5 cm + 1 ppm vertical | OFFICIAL_SPEC |

The Matrice 30 family includes M30 and M30T variants. Their sensor configurations shall be represented as distinct model/variant records rather than inferred interchangeable payloads.

## 3. DJI Matrice 4 Series

### Common aircraft data

| Parameter | Official value | Source class |
|---|---:|---|
| Standard takeoff weight | 1219 g | OFFICIAL_SPEC |
| Takeoff weight with low-noise propellers | 1229 g | OFFICIAL_SPEC |
| Max takeoff weight, standard propellers | 1420 g | OFFICIAL_SPEC |
| Max takeoff weight, low-noise propellers | 1430 g | OFFICIAL_SPEC |
| Max flight distance, standard propellers | 35 km | OFFICIAL_SPEC |
| Max flight distance, low-noise propellers | 32 km | OFFICIAL_SPEC |
| Max wind resistance | 12 m/s* | OFFICIAL_SPEC |
| GNSS | GPS + Galileo + BeiDou + GLONASS* | OFFICIAL_SPEC |
| RTK accuracy | 1 cm + 1 ppm horizontal; 1.5 cm + 1 ppm vertical | OFFICIAL_SPEC |
| Standard propeller | 1157F | OFFICIAL_SPEC |
| Low-noise propeller | 1154F | OFFICIAL_SPEC |
| E-Port | 1; official accessories and third-party PSDK devices | OFFICIAL_SPEC |
| E-Port Lite | 1; USB/tuning and some third-party PSDK devices | OFFICIAL_SPEC |
| Operating temperature | -10 to 40 °C | OFFICIAL_SPEC |

`*` DJI specifies wind resistance during takeoff and landing; GLONASS is stated as supported when RTK is enabled.

### Matrice 4E

| Parameter | Official value | Source class |
|---|---|---|
| Wide camera sensor | 4/3 CMOS, 20 MP | OFFICIAL_SPEC |
| Wide camera equivalent focal length | 24 mm | OFFICIAL_SPEC |
| Wide aperture | f/2.8–f/11 | OFFICIAL_SPEC |
| Medium tele sensor | 1/1.3-inch CMOS, 48 MP | OFFICIAL_SPEC |
| Tele sensor | 1/1.5-inch CMOS, 48 MP | OFFICIAL_SPEC |

### Matrice 4T

| Parameter | Official value | Source class |
|---|---|---|
| Wide camera sensor | 1/1.3-inch CMOS, 48 MP | OFFICIAL_SPEC |
| Medium tele sensor | 1/1.3-inch CMOS, 48 MP | OFFICIAL_SPEC |
| Tele sensor | 1/1.5-inch CMOS, 48 MP | OFFICIAL_SPEC |
| Thermal imager | integrated thermal camera | OFFICIAL_SPEC |

### Configuration rule

The Matrice 4E and 4T shall be represented as separate variants with their documented integrated sensor sets. They shall not be modelled as arbitrary gimbal/payload hosts.

## 4. DJI Mavic 3 Enterprise Series

| Parameter | Mavic 3E | Mavic 3T/3TA | Source class |
|---|---:|---:|---|
| Weight, aircraft with propellers, without accessories | 915 g | 920 g | OFFICIAL_SPEC |
| Max takeoff weight | 1050 g | 1050 g | OFFICIAL_SPEC |
| Max ascent speed | 6 m/s normal; 8 m/s sport | same | OFFICIAL_SPEC |
| Max flight speed, normal | 15 m/s | 15 m/s | OFFICIAL_SPEC |
| Max wind resistance | 12 m/s | 12 m/s | OFFICIAL_SPEC |
| Max takeoff altitude | 6000 m without payload | 6000 m without payload | OFFICIAL_SPEC |
| Max flight time | 45 min* | 45 min* | OFFICIAL_SPEC |
| GNSS | GPS + Galileo + BeiDou + GLONASS** | same | OFFICIAL_SPEC |
| Operating temperature | -10 to 40 °C | -10 to 40 °C | OFFICIAL_SPEC |
| Class | C2 EU | C2 EU | OFFICIAL_SPEC |

`*` DJI also documents lower values when the EU C1-version battery and low-noise propellers are used.

`**` DJI states GLONASS is supported only when the RTK module is enabled.

### Mavic 3E sensor

4/3 CMOS, effective 20 MP, 24 mm equivalent lens, f/2.8–f/11, with mechanical/electronic shutter options documented by DJI.

### Mavic 3T / 3TA sensor

Integrated wide, telephoto and thermal imaging system. Mavic 3T and 3TA thermal camera specifications differ and therefore shall be separate variants in the catalogue.

### Battery compatibility rule

The DJI Mavic 3 Enterprise Series uses its documented Mavic 3 Enterprise batteries. Battery variants and EU C1 configuration shall be modelled explicitly because DJI documents different weight, endurance and regulatory characteristics.

## 5. DJI Matrice 3D / 3TD

These aircraft are associated by DJI with DJI Dock 2 and have integrated sensor architectures.

| Parameter | Matrice 3D | Matrice 3TD | Source class |
|---|---|---|---|
| Wide camera | 4/3 CMOS, 20 MP | 1/1.32-inch CMOS, 48 MP | OFFICIAL_SPEC |
| Wide equivalent focal length | 24 mm | 24 mm | OFFICIAL_SPEC |
| Wide aperture | f/2.8–f/11 | f/1.7 | OFFICIAL_SPEC |
| Tele camera | 1/2-inch CMOS, 12 MP | 1/2-inch CMOS, 12 MP | OFFICIAL_SPEC |
| Tele equivalent focal length | 162 mm | 162 mm | OFFICIAL_SPEC |
| Thermal camera | not applicable to 3D | integrated thermal camera | OFFICIAL_SPEC |
| Protection | IP54 | IP54 | OFFICIAL_SPEC |

These models shall not be presented as interchangeable with Matrice 4 or Mavic 3 Enterprise payloads.

## 6. Autel EVO Max Series — external reference platform

The following data are from Autel Robotics official documentation and are retained as a separate manufacturer family.

### EVO Max 4T

| Parameter | Official value | Source class |
|---|---:|---|
| Weight | 1645 g incl. ABX40, Fusion 4T and propellers | OFFICIAL_SPEC |
| Max takeoff mass | 1999 g; 1890 g for EU C2 | OFFICIAL_SPEC |
| Propeller | 1136 / 1158 depending on documented configuration | OFFICIAL_SPEC |
| Max ascent speed | up to 8 m/s, mode dependent | OFFICIAL_SPEC |
| Max horizontal speed | up to 23 m/s, mode dependent | OFFICIAL_SPEC |
| Max takeoff altitude | 4000 m with ABX40; 3000 m with ABX41 | OFFICIAL_SPEC |
| Max flight time | 42 min | OFFICIAL_SPEC |
| Max hover time | 38 min | OFFICIAL_SPEC |
| Max wind resistance | 12 m/s | OFFICIAL_SPEC |
| GNSS | GPS + GLONASS + Galileo + BDS | OFFICIAL_SPEC |
| IP rating | IP43 | OFFICIAL_SPEC |

### Battery

Autel documents ABX40 and ABX41 batteries for the EVO Max family, including their voltage, energy and weight. The V2 family uses ABX41-D according to the reviewed manufacturer documentation.

Battery identity shall therefore be a first-class configuration parameter.

### Payload/interface rule

Autel documentation identifies P-Port and P-Port Lite interfaces and supports third-party PSDK devices on the EVO Max platform. BlueSky PRO shall nevertheless require an explicit compatibility record for each third-party device before exposing it to an operator.

## 7. Catalogue inclusion policy

A UAV family may enter the operational reference catalogue when the reviewed manufacturer documentation establishes sufficient identity and core performance data. Individual payload combinations require separate compatibility evidence.

The following shall never be inferred merely from physical similarity:

- payload compatibility;
- battery interchangeability;
- electrical compatibility;
- mounting compatibility;
- software/SDK compatibility;
- regulatory class applicability;
- certification suitability.

## 8. Operational UI rule

The Administrator database may contain all researched records. Flight Planning shall expose only records satisfying the active compatibility and configuration rules.

```text
CATALOGUE
   ↓
SOURCE VALIDATION
   ↓
COMPATIBILITY RULES
   ↓
APPROVED CONFIGURATION SET
   ↓
MISSION REQUIREMENTS
   ↓
AUTO-CONFIGURATION
   ↓
OPERATOR SELECTION
```

## 9. Official sources reviewed

- DJI Enterprise — Matrice 350 RTK Specifications.
- DJI Enterprise — Matrice 30 Series Specifications.
- DJI Enterprise — Matrice 4 Series Specifications.
- DJI Enterprise — Mavic 3 Enterprise Specifications.
- DJI Enterprise — DJI Dock 2 / Matrice 3D and 3TD specifications.
- Autel Robotics — EVO Max Series official manuals and product specifications.

## 10. Status

`REFERENCE_CATALOGUE_EXPANSION`

This document is a controlled research/reference input. It does not by itself establish airworthiness, operational approval, Russian certification compliance, or acceptance of any particular mission configuration.
