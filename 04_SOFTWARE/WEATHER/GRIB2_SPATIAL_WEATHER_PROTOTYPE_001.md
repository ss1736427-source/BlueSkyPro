# BlueSky PRO — spatial weather and GRIB2 wind engine

**Current implementation status:** NOAA GFS GRIB2 acquisition is merged into `main` (PR #59). A separate weather-engine implementation is under review in PR #60. The decoder and route adapter are not yet approved for operational flight use.

## 1. Data source

Primary acquisition source: NOAA/NCEP GFS 0.25-degree GRIB2 through the NOMADS GRIB Filter.

- Official product inventory: https://nomads.ncep.noaa.gov/gribfilter.php?ds=gfs_0p25
- Official variable/level mapping: https://nomads.ncep.noaa.gov/info.php?page=opendap_grib_migration
- GFS product inventory: https://www.nco.ncep.noaa.gov/pmb/products/gfs/nomads/

The selected direct AGL wind levels are 10, 20, 30, 40, 50, 80, 100 and 1000 m. The 100–1000 m gap is not suitable for ordinary linear interpolation. The weather engine therefore rejects vertical interpolation across gaps greater than 100 m by default. It does not silently create 150–500 m observations.

GFS 0.25-degree horizontal resolution is on the order of tens of kilometres. It does not resolve building wakes, small islands, local coastal jets, wires or terrain-scale flow. It is a regional forecast input, not a local obstacle/wind sensor.

ECMWF IFS 0.1-degree Open Data remains the planned independent second provider: https://www.ecmwf.int/en/forecasts/datasets/open-data

## 2. Acquisition layer — merged to main

Files:
- `qt/BlueSkyPRO-HMI/src/WeatherDataService.h`
- `qt/BlueSkyPRO-HMI/src/WeatherDataService.cpp`
- `qt/BlueSkyPRO-HMI/tests/WeatherDataServiceTest.cpp`

The Qt service requests UGRD/VGRD asynchronously, enforces a bounded request and 256 MiB download limit, writes a temporary file, checks GRIB2 edition/container markers, publishes a cache file and SHA-256 manifest, supports cache reuse and cancellation, and exposes progress/ready/error signals.

This is acquisition and container validation. The GRIB marker check does not prove that the file contains valid meteorological fields.

## 3. Decoder and normalized wind field — PR #60

New files:
- `weather_field.hpp`: normalized regular latitude/longitude grid and QC result contract.
- `grib2_decoder.hpp/.cpp`: optional ecCodes C API decoder.
- `wind_interpolator.cpp`: bilinear horizontal interpolation and bounded vertical interpolation.
- `route_wind_adapter.hpp/.cpp`: route-to-`WindSample` conversion.
- `weather_trajectory_calculator.hpp/.cpp`: fail-closed preparation followed by the existing `WindPerformanceTrajectory::calculate`.
- `grib2_weather_pipeline.hpp/.cpp`: end-to-end GRIB2 file → decoder → interpolation/QC → route samples → trajectory calculation.

Decoder acceptance rules:
- GRIB edition 2.
- `shortName=u` and `shortName=v`, units in m/s.
- `typeOfLevel=heightAboveGround`, positive AGL level.
- regular latitude/longitude grid with complete Ni × Nj coordinate coverage.
- U and V must have identical AGL level, valid date/time and grid coordinates.
- unsupported grids, incomplete pairs, duplicate fields and malformed grids are rejected.
- missing values are preserved as NaN and cause an interpolation query to fail if they affect its bilinear cell.

The decoder currently handles one regular_ll grid geometry. Reduced Gaussian, projected and rotated grids are deliberately unsupported rather than silently reinterpreted.

## 4. Interpolation and route adapter

Horizontal interpolation is bilinear and does not extrapolate outside the decoded grid. It does not substitute nearest-neighbour values for missing cells.

Vertical interpolation:
- exact model level is used directly;
- interpolation is allowed only between bracketing AGL levels;
- maximum allowed vertical gap defaults to 100 m;
- extrapolation is disabled;
- mixed forecast valid times are rejected.

The route adapter:
- requires explicit AGL altitude references for all route waypoints;
- samples each segment at its geographic and altitude midpoint (first implementation approximation);
- requires one common forecast valid time across the field set;
- builds all samples before modifying the trajectory input;
- writes `WindSample.north_mps = VGRD`, `east_mps = UGRD`;
- sets `up_mps = 0`, because this product request does not provide vertical wind;
- sets the wind snapshot ID/version only when every segment sample passes.

The `Grib2WeatherPipeline` is a callable core API that connects a GRIB2 file to `WindPerformanceTrajectory`. The new sources are linked into the `bluesky_planning` CMake library. The Qt HMI acquisition service still does not automatically invoke this pipeline; application-level orchestration and ecCodes runtime packaging remain follow-up work. Temporal interpolation and per-segment forecast-time sampling are not implemented. The existing `WindPerformanceTrajectory` physics and wind-tolerance logic have not been changed.

## 5. Build and tests

Standalone Linux build with ecCodes:

```bash
sudo apt-get install -y libeccodes-dev
cmake -S 04_SOFTWARE/WEATHER -B build-weather -DBLUESKY_ENABLE_ECCODES=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-weather --parallel 2
ctest --test-dir build-weather --output-on-failure
```

The new workflow `.github/workflows/weather-grib2-engine.yml` performs this build on Ubuntu and runs deterministic synthetic-grid and fail-closed contract tests.

**Verification still required before operational use:**
1. Confirm CI build/test result for PR #60.
2. Decode a real current NOAA GFS GRIB2 download and inspect its complete message inventory with `grib_ls` / `grib_dump`.
3. Decode a current NOAA GFS GRIB2 payload and validate U/V AGL message pairs, scanning modes, missing bitmap values, date/time and level pairing against `grib_ls` / `grib_dump`. The CI fixture is generated with ecCodes and is not a substitute for an operational NOAA sample.
4. Validate decoder output against ecCodes command-line output for selected grid cells.
5. Add Windows/MSVC ecCodes packaging and runtime DLL deployment; the current weather-engine CI validates Linux only.
6. Add temporal interpolation, mission-time sampling and snapshot lineage integration.
7. Connect the adapter to the production planning invocation only after all above tests pass.

Until then, downloaded or decoded forecast data must not be treated as approved operational wind input. Synthetic Virtual Flight data and live forecast data remain separate classifications.
