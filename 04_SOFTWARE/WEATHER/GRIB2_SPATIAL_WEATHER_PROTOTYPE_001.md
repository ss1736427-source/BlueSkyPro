# NOAA GFS GRIB2 spatial weather prototype

**Status:** acquisition probe committed; live download and scientific decode remain **UNVERIFIED**.  
**Canonical branch:** `main`  
**Purpose:** verify the first link in the spatial-weather pipeline before integrating a GRIB decoder into the Qt application.

## 1. Selected first source

NOAA/NCEP NOMADS GFS 0.25-degree GRIB filter:

- Endpoint: https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_0p25.pl
- Official variable/level mapping: https://nomads.ncep.noaa.gov/info.php?page=opendap_grib_migration
- The filter supports selection by GFS initialization date/cycle, forecast hour, variable, above-ground level and geographic bounding box.
- Requested wind components are `UGRD` and `VGRD`; requested heights are metres above ground (AGL), not barometric altitude.

The exact availability of a requested level/file is determined by the selected product and cycle. An unavailable file or unsupported field must be reported as an error, not replaced silently.

## 2. Prototype tool

File: `tools/weather/noaa_gfs_grib_probe.py`

The script uses only the Python standard library. It:

1. constructs a bounded HTTPS request for one forecast hour and a selected geographic rectangle;
2. requests only U/V wind components and selected above-ground levels;
3. sets a descriptive User-Agent and request timeout;
4. enforces a configurable maximum download size;
5. writes to a temporary `.part` file, checks the GRIB2 edition marker and final `7777` marker, then atomically renames the payload;
6. emits a JSON manifest with cycle, forecast hour, bounding box, requested variables/levels, retrieval time, byte count and SHA-256;
7. labels the result `LIVE_MODEL_DATA_UNDECODED`, with `decode_status=NOT_PERFORMED` and `flight_use_status=NOT_APPROVED`.

The probe does **not** decode meteorological values, interpolate a route, validate forecast quality, or authorize a flight.

### Offline contract test

Run from the repository root:

```sh
python tools/weather/noaa_gfs_grib_probe.py --self-test
```

Expected output:

```text
SELF-TEST PASS: URL construction, selected levels/variables, bbox and GRIB2 guards.
```

This self-test makes no network request and is not evidence of live NOAA availability.

### Live acquisition example

Choose a GFS cycle that NOMADS currently lists as available. Example for a small Helsinki-area rectangle:

```sh
python tools/weather/noaa_gfs_grib_probe.py \
  --date YYYYMMDD --cycle 12 --forecast-hour 6 \
  --bbox 23 59 26 61 --levels 10,80,100 \
  --output artifacts/weather/gfs-helsinki-f006.grib2
```

Replace `YYYYMMDD` with the date of the selected cycle in UTC. A failed request can mean the cycle is not yet published, the forecast hour is not available, or a selected level is absent. Check the current official NOMADS catalogue before retrying. Narrow the bounding box or request fewer levels if the size limit is reached.

Successful acquisition should produce both:

- `gfs-helsinki-f006.grib2`
- `gfs-helsinki-f006.grib2.manifest.json`

The manifest's retrieval timestamp is not the model initialization time. Preserve both.

## 3. Required next verification

1. Run the offline self-test with the supported Python version.
2. Run a live download for a currently available cycle and save the resulting manifest.
3. Inspect the payload with ECMWF ecCodes (for example, `grib_ls`) and verify that both `UGRD` and `VGRD` are present at each requested height.
4. Check the decoded grid bounds, grid spacing, units, valid time, missing values and whether each height is truly above ground.
5. Add deterministic tests using a small, redistributable GRIB2 fixture or a mocked transport; CI must not depend on a live NOAA endpoint.
6. Only after those checks, design the C++ provider/decoder boundary and route-time-height interpolation contract.

## 4. Safety and scientific limits

- A GRIB2 signature check proves only container plausibility, not meteorological correctness.
- GFS 0.25-degree grid spacing is on the order of tens of kilometres; it does not resolve building wakes, wires, tree lines, terrain-induced local flow or small-scale gusts.
- AGL levels in the weather model must not be confused with the BlueSky route's barometric altitude or altitude above a terrain datum without explicit vertical-datum conversion.
- Missing, stale, incomplete or undecoded data must remain unavailable to operational route/energy calculations.
- Forecast data and synthetic Virtual Flight fixtures must remain separate and explicitly classified.
- NOAA's operational availability and the current cycle's exact field inventory must be checked at retrieval time. Do not hard-code a cycle as permanently available.

## 5. References

- NOAA/NCEP GFS 0.25-degree GRIB filter: https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_0p25.pl
- NOMADS field/level and forecast-hour mapping: https://nomads.ncep.noaa.gov/info.php?page=opendap_grib_migration
- NOAA GFS documentation/catalogue: https://www.nco.ncep.noaa.gov/pmb/products/gfs/
- ecCodes: https://confluence.ecmwf.int/display/ECC/
