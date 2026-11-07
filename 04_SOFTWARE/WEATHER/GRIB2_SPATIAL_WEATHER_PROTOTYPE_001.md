# BlueSky PRO weather data integration — NOAA GFS

**Status:** NOAA source selected; asynchronous Qt acquisition service added on branch `feat/weather-data-engine-noaa-gfs-001`. Live download and GRIB scientific decoding are not yet verified in the development runtime. Downloaded files are explicitly marked `LIVE_MODEL_DATA_UNDECODED` and are blocked from optimizer use until decoding and quality control are implemented.

## 1. Source decision

Primary source for the first operational integration: **NOAA/NCEP GFS 0.25-degree GRIB2 via NOMADS GRIB Filter**.

Reason for selecting NOAA ahead of ECMWF for the initial low-altitude UAV use case:

- The official GFS 0.25-degree filter currently lists direct above-ground wind levels at 10, 20, 30, 40, 50, 80, 100 and 1000 m.
- This gives direct 50 m and 100 m AGL values, which are useful for the core 50–500 m mission envelope.
- GFS supports geographic subsetting and selection of U/V wind components.
- The same NOMADS interface exposes hybrid levels and pressure levels for later vertical-profile work.

Official inventory: https://nomads.ncep.noaa.gov/gribfilter.php?ds=gfs_0p25  
Official variable/level and forecast-hour mapping: https://nomads.ncep.noaa.gov/info.php?page=opendap_grib_migration  
GFS product inventory: https://www.nco.ncep.noaa.gov/pmb/products/gfs/nomads/

**Important limitation:** 100–1000 m is a large vertical gap. A 150–500 m value must not be treated as a direct model level. The first implementation only acquires model fields; interpolation quality must be exposed by the decoder/query service. GFS 0.25-degree horizontal grid is tens of kilometres and cannot resolve building wakes, small islands, local coastal jets or terrain-scale flow.

ECMWF IFS 0.1-degree GRIB2 is retained as the next independent model adapter. Its open-data licence permits commercial use and redistribution with CC-BY-4.0 attribution. The 0.1-degree product is published about two hours after the 0.25-degree product and is split into surface and pressure-level files. Official documentation: https://www.ecmwf.int/en/forecasts/datasets/open-data

## 2. Runtime integration added

Files:
- `qt/BlueSkyPRO-HMI/src/WeatherDataService.h`
- `qt/BlueSkyPRO-HMI/src/WeatherDataService.cpp`
- `qt/BlueSkyPRO-HMI/tests/WeatherDataServiceTest.cpp`

The service is registered in the application QML context as `weatherDataService`.

Capabilities:
1. Builds a bounded NOMADS GRIB Filter URL for GFS 0.25.
2. Requests only UGRD/VGRD and explicitly selected AGL levels.
3. Uses Qt Network asynchronously; it does not block the UI thread.
4. Limits a single response to 256 MiB.
5. Writes to a `.part` file and publishes the completed GRIB2 atomically.
6. Validates GRIB edition 2 signature and terminal `7777` marker.
7. Stores a JSON manifest containing model cycle, forecast hour, bounding box, levels, retrieval timestamp, size and SHA-256.
8. Reuses a previously downloaded matching file from the local cache.
9. Rejects unsupported heights and invalid forecast-hour steps rather than silently substituting data.
10. Exposes progress, ready, error and cancellation signals.

Example QML call:

```qml
weatherDataService.requestWindSubset(
    "20261107", "12", 6,
    96.0, 5.0, 100.0, 10.0,
    ["10", "50", "100"]
)
```

The date and cycle above are an example only. Production scheduling must select a currently published model cycle and requested forecast leads dynamically.

## 3. Current data contract and flight-use gate

The service currently performs **acquisition and container validation only**. It does not decode the GRIB fields into meteorological values. Manifest fields:
- `classification = LIVE_MODEL_DATA_UNDECODED`
- `decode_status = NOT_PERFORMED`
- `optimizer_use_status = BLOCKED_UNTIL_DECODED_AND_QC_PASSED`

Therefore, the downloaded file is not yet passed into `WindPerformanceTrajectory`. This is intentional: the existing optimizer consumes per-segment wind samples and must not receive synthetic, stale, missing or undecoded data.

## 4. Next required integration steps

1. Add ecCodes GRIB2 decoding and verify actual `UGRD`/`VGRD` messages, levels, grid coordinates, units and valid times.
2. Implement spatial and vertical interpolation with explicit quality flags; preserve the model-level identity and vertical datum.
3. Build a mission query from route bounds, altitude range and estimated segment-time window.
4. Convert decoded wind fields into per-segment `WindSample` values for `WindPerformanceTrajectory`.
5. Persist immutable forecast snapshot IDs so mission calculations are reproducible.
6. Add a background refresh scheduler and a multi-lead request queue. Existing files must be reused; HMI and optimizer must not wait on network I/O.
7. Add ECMWF IFS as an independent provider using the same normalized data contract.

## 5. Verification

Offline contract test:

```sh
cmake -S qt/BlueSkyPRO-HMI -B build-weather
cmake --build build-weather --target weather_data_service_test
ctest --test-dir build-weather -R weather_data_service_contract --output-on-failure
```

The offline test checks URL construction, supported levels, forecast-step validation and GRIB2 container guards. It does not prove live NOAA availability or scientific decoding.

A live download should be performed from a network-enabled Windows or CI runner using a currently published date/cycle and a small mission-area bounding box. After download, run `grib_ls` or equivalent ecCodes inspection and confirm both U/V fields at each requested AGL level before enabling optimizer consumption.
