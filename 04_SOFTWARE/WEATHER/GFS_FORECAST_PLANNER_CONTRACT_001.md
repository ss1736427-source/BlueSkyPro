---
id: GFS-FORECAST-PLANNER-CONTRACT-001
type: weather_forecast_planner_contract
status: IMPLEMENTED_CONTRACT
---

# BlueSky PRO — GFS Forecast Planner Contract 001

## 1. Purpose

Select one confirmed-available NOAA GFS 0.25-degree forecast run and the forecast lead files needed to bracket an explicitly supplied mission execution window.

The planner is a deterministic core component. It does not perform network requests, infer mission timing, decode GRIB2, interpolate weather in time, or certify meteorological quality.

## 2. Inputs

The caller supplies:

- Mission start and end as strict UTC timestamps: `YYYY-MM-DDTHH:MM:SSZ`.
- A list of candidate GFS cycle timestamps.
- An explicit availability flag for each candidate. A flag may be true only after the acquisition layer confirms that the cycle is usable from the provider or verified cache.

The planner does not treat a scheduled cycle as available merely because the cycle should exist.

## 3. GFS forecast lead schedule

The implemented schedule is:

- F000 through F120: hourly.
- F123 through F384: every three hours.

This schedule is encoded explicitly and tested at the transition from hourly to three-hourly leads.

## 4. Selection rule

1. Validate the mission window and every supplied cycle timestamp.
2. Accept only synoptic cycles at 00Z, 06Z, 12Z or 18Z.
3. Consider only candidates explicitly marked available.
4. Sort candidates by cycle time and examine newest first.
5. For each candidate, convert the mission start/end to forecast lead offsets.
6. Select the newest single run whose lead schedule brackets both ends of the complete mission window.
7. Return the ordered set of lead hours from the lower bracket of mission start through the upper bracket of mission end.

A single run is required in this contract. It does not silently combine different model cycles into one time series.

## 5. Fail-closed outcomes

- `READY`: one confirmed cycle brackets the complete requested window.
- `INVALID_REQUEST`: malformed or reversed mission time interval.
- `INVALID_CYCLE_CANDIDATE`: malformed timestamp or non-synoptic candidate.
- `NO_AVAILABLE_CYCLE`: no candidate was supplied or no candidate is confirmed available.
- `NO_SINGLE_RUN_COVERAGE`: no confirmed run brackets the full window within F000-F384.

No partial plan is returned as ready.

## 6. Mission time integration boundary

The canonical Mission model currently has no structured execution schedule and Route waypoints have no planned passage timestamps. The existing Mission Time Model 001 is marked `DRAFT_FOR_AGREEMENT`.

Therefore, this planner accepts an explicit mission execution window from its caller. It does not derive it from ETD, TOT, TRIP, ETA, route distance, nominal speed, task text, or waypoint order. Those values have different semantics and are not interchangeable.

A future schedule contract must explicitly define planned movement start, takeoff, task execution intervals, route passage times, landing and recovery semantics. Until then, callers must provide a reviewed UTC window.

## 7. Deliberate limitations

- Provider availability is an input; this component does not probe NOMADS.
- The planner does not decide how long a forecast cycle remains operationally fresh.
- It does not claim that a GRIB2 file has downloaded, decoded, passed QC, or covers every required grid cell.
- It does not implement temporal interpolation. It only returns the lead set that brackets the requested interval.
- It does not combine forecast runs when one run cannot cover the full interval.
- It does not modify mission, route, HMI, map, DEM or trajectory physics.

## 8. Verification

The standalone contract test covers newest eligible cycle selection, forecast cadence across F120/F123, exact-time windows, unavailable cycles, invalid cycle times, invalid calendar dates and forecast-horizon rejection.
