# BlueSky PRO Performance Monitor

## Purpose

Collect synchronized browser/map timing and native Windows graphics/process counters while reproducing map stutter. The monitor is diagnostic-only: it does not change map style, pitch, terrain settings, camera behavior, or tile configuration.

## Build integration

The in-map monitor is part of `qml/MapTilerWebMap.qml`. It adds a **PERFORMANCE MONITOR** control to the map controls. The existing **PERF SNAPSHOT** remains available.

The native Windows collector is:

`tools/collect-windows-performance.ps1`

## Test procedure

1. Build and launch the same application build that will be evaluated.
2. Open the map and wait until the style and visible tiles have settled.
3. Open **PERFORMANCE MONITOR** and click **START RECORDING**.
4. In a separate PowerShell window, run:

   ```powershell
   Set-ExecutionPolicy -Scope Process Bypass
   .\tools\collect-windows-performance.ps1 -IntervalSeconds 1 -OutputPath .\BlueSkyPRO-windows-performance.csv
   ```

   Run the command from the repository root. If the script is copied elsewhere, adjust the path.
5. Reproduce the stutter with several continuous map drags (10–20 seconds each), without changing zoom, pitch, map style, or terrain state during a run.
6. Click **STOP**, then **EXPORT JSON** in the in-map monitor.
7. Stop the PowerShell collector with Ctrl+C. It writes the Windows counters to CSV.
8. Keep the JSON and CSV from the same test run together.

## Data collected

### In-map JSON

- RAF intervals overall and during map movement.
- MapLibre render-event intervals overall and during movement. These are intervals between events, not GPU execution times.
- Pointer samples and drag distance from the existing diagnostics.
- Long Tasks API events.
- WebGL context type, driver renderer/vendor strings when exposed, and context attributes.
- JavaScript heap when Chromium exposes `performance.memory`.
- Resource Timing entries, including request/response timing and sizes when the browser exposes them.
- Current style/source/layer and terrain state.
- Measurement overhead of the monitor's one-second sampling routine.
- Environment information such as user agent, logical processor count, device pixel ratio, and reported device memory.

### Windows CSV

- GPU Engine utilization counters, including per-engine instances where supported.
- GPU Process Memory dedicated/shared usage counters where supported.
- CPU and private working-set/private-bytes counters for BlueSky PRO and Qt WebEngine process names where those instances are exposed.

## Important measurement limits

Page JavaScript cannot reliably read GPU utilization, dedicated VRAM use, Chromium compositor frame execution time, or Qt WebEngine GPU-process timing. Those values are therefore explicitly marked unavailable in the JSON and are collected separately through Windows performance counters.

MapLibre's `render` event interval is not a measurement of GPU frame execution time. Resource Timing duration includes request/wait activity and must not be interpreted as decode or GPU-upload duration. Some cross-origin resource sizes may be zero because of browser timing restrictions.

The Windows counter names and availability depend on the Windows build and installed graphics driver. If a counter set is unavailable, the collector prints a warning; missing counters must not be interpreted as zero utilization.

## Test discipline

For comparable runs, keep constant:

- same executable and machine;
- same map style, center, zoom, bearing and pitch;
- same terrain ON/OFF state;
- same route and NOTAM data state;
- same approximate drag path and duration.

Run at least:
- baseline with the current normal map configuration;
- terrain OFF, with all other conditions unchanged;
- satellite raster disabled only if a dedicated controlled test configuration is available.

Do not change several variables in one run. Compare the timestamps of visible stutters against RAF/render intervals, long tasks, resource requests, GPU engine utilization, and GPU memory counters.

## Current scope

This first implementation provides a synchronized capture framework, not an automatic root-cause verdict. It cannot yet isolate DEM decode time or compositor execution time directly. Those require deeper native Chromium/Qt instrumentation or GPU tracing. The collected data is intended to narrow the subsystem and guide the next controlled experiment.
