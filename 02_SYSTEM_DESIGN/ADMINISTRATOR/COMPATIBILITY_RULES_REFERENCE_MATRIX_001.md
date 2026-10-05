# COMPATIBILITY RULES REFERENCE MATRIX 001

## 1. Status

This matrix is a reference layer for the Administrator compatibility engine. Only relationships explicitly supported by manufacturer documentation are marked `CONFIRMED`.

`UNKNOWN` is not treated as compatible for automatic operational selection.

## 2. DJI Matrice 350 RTK

Official DJI documentation states that M350 RTK supports Zenmuse H30, H30T, L2, H20N, L1, P1, H20, H20T, S1 and V1, plus certified third-party Payload SDK payloads. It supports up to three payloads using the available gimbal arrangement. DJI also states that two TB60 batteries may be used simultaneously, but TB60 and TB65 must not be mixed. citeturn0search0turn0search1

| Relation | Result | Basis |
|---|---|---|
| M350 RTK + 2×TB65 | CONFIRMED | DJI FAQ/specification |
| M350 RTK + 2×TB60 | CONFIRMED | DJI FAQ |
| M350 RTK + TB60 + TB65 | INCOMPATIBLE | DJI FAQ explicitly prohibits mixing |
| M350 RTK + Zenmuse H30 | CONFIRMED | DJI official specs |
| M350 RTK + Zenmuse H30T | CONFIRMED | DJI official specs |
| M350 RTK + Zenmuse L2 | CONFIRMED | DJI official specs |
| M350 RTK + Zenmuse L1 | CONFIRMED | DJI official specs |
| M350 RTK + Zenmuse P1 | CONFIRMED | DJI official specs |
| M350 RTK + Zenmuse H20/H20T/H20N | CONFIRMED | DJI official specs |
| M350 RTK + S1/V1 | CONFIRMED | DJI official specs |
| M350 RTK + certified PSDK payload | CONDITIONAL | Certification/SDK conditions apply |
| M350 RTK + arbitrary third-party payload | UNKNOWN | No blanket compatibility claim |

## 3. DJI Matrice 300 RTK

M300 RTK relationships shall be populated only from the applicable DJI compatibility documentation. The M350 and M300 accessory ecosystems shall not be treated as interchangeable merely because an interface appears similar.

| Relation | Result | Basis |
|---|---|---|
| M300 RTK + component documented for M300 | CONFIRMED | Applicable DJI compatibility documentation required |
| M300 RTK + M350-only component | UNKNOWN/INCOMPATIBLE | Must follow component-specific documentation |
| M300 RTK + M350 RC Plus | CONFIRMED where DJI FAQ applies | DJI official FAQ |

## 4. DJI Matrice 30 / M30T

The M30 family uses an integrated multi-sensor payload architecture. M30 and M30T differ by the thermal sensor: M30 has wide-angle, zoom and laser rangefinder; M30T adds thermal imaging. The platform uses two batteries and has a stated MTOW of 4069 g. citeturn1search1turn1search6

| Relation | Result | Basis |
|---|---|---|
| M30 + its integrated payload | CONFIRMED | DJI official specifications |
| M30T + its integrated payload | CONFIRMED | DJI official specifications/FAQ |
| M30/M30T + arbitrary external gimbal | UNKNOWN | No blanket external-gimbal compatibility established |
| M30/M30T + Dock version requirement | CONDITIONAL | DJI Dock-specific aircraft version applies |

## 5. DJI Matrice 4E / 4T

DJI identifies M4E and M4T as separate aircraft variants with integrated sensor payloads. The series exposes E-Port and E-Port Lite interfaces; DJI states that official accessories and third-party PSDK devices can use the E-Port, subject to the stated installation restrictions. citeturn0search2turn0search4turn0search7

| Relation | Result | Basis |
|---|---|---|
| M4E + integrated M4E sensor package | CONFIRMED | DJI official specs/FAQ |
| M4T + integrated M4T sensor package | CONFIRMED | DJI official specs/FAQ |
| M4E + arbitrary payload | UNKNOWN | Must be explicitly supported |
| M4T + arbitrary payload | UNKNOWN | Must be explicitly supported |
| M4 Series + official/PSDK accessory through E-Port | CONDITIONAL | Interface and installation conditions apply |
| M4 Series + accessory installed after power-on | INCOMPATIBLE | DJI states accessories/expansion modules must be installed before powering on |

## 6. DJI Matrice 400

DJI officially specifies a single TB100 battery for Matrice 400 and a maximum payload of 6 kg at the third gimbal connector. DJI lists L3, H30, H30T, L2, P1 and specified functional accessories including AP100, S1, V1, Manifold 3 and cellular/RTK ecosystem components. DJI explicitly states that Matrice 400 accessories are not compatible with Matrice 350 accessories. citeturn1search0turn1search2

| Relation | Result | Basis |
|---|---|---|
| M400 + TB100 | CONFIRMED | DJI official specs |
| M400 + dual batteries | INCOMPATIBLE | DJI FAQ: single battery |
| M400 + Zenmuse L3 | CONFIRMED | DJI official specs/FAQ |
| M400 + Zenmuse H30 | CONFIRMED | DJI official specs/FAQ |
| M400 + Zenmuse H30T | CONFIRMED | DJI official specs/FAQ |
| M400 + Zenmuse L2 | CONFIRMED | DJI official specs/FAQ |
| M400 + Zenmuse P1 | CONFIRMED | DJI official specs/FAQ |
| M400 + AP100 Parachute | CONFIRMED | DJI official FAQ |
| M400 + Manifold 3 | CONFIRMED | DJI official FAQ |
| M400 + M350 accessory | INCOMPATIBLE | DJI FAQ explicitly states ecosystems are not compatible |
| M400 + non-DJI payload | CONDITIONAL | Total mass and manufacturer/test requirements apply |

## 7. Autel EVO Max 4T

Autel official battery information identifies ABX40 and ABX41 battery variants, with different voltage, energy and mass. These battery records shall remain separate catalogue items until the aircraft documentation establishes their exact permitted configurations. citeturn0search9

| Relation | Result | Basis |
|---|---|---|
| EVO Max 4T + ABX40 | CONFIRMED | Autel official battery documentation |
| EVO Max 4T + ABX41 | CONFIRMED | Autel official battery documentation |
| EVO Max 4T + arbitrary external payload | UNKNOWN | Explicit payload compatibility required |

## 8. Rule interpretation

The matrix is deliberately conservative:

```text
CONFIRMED
    ↓
eligible candidate

CONDITIONAL
    ↓
eligible only after condition evaluation

UNKNOWN
    ↓
not eligible for automatic operational selection

INCOMPATIBLE
    ↓
reject + hide from normal selection
```

## 9. Configuration-level evaluation

Pairwise compatibility is insufficient for final approval. The engine shall evaluate the complete configuration for:

- total mass;
- MTOW;
- payload position and permitted mounting point;
- battery count and topology;
- power constraints;
- environmental limits;
- propulsion constraints;
- communication and navigation requirements;
- energy feasibility;
- operational approval state.

## 10. Source hierarchy

1. Current manufacturer technical specification;
2. Current manufacturer user/service/compatibility documentation;
3. Manufacturer SDK or integration documentation;
4. Controlled verified engineering data;
5. Other reference material — never sufficient by itself to establish operational compatibility.

## 11. Scope boundary

This matrix does not certify a configuration. It establishes the compatibility-data layer from which the Administrator and automatic configurator can build candidate configurations. Certification, operational approval and verification remain separate controlled states.
