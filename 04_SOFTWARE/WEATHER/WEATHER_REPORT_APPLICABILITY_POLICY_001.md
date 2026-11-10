---
id: WEATHER-REPORT-APPLICABILITY-POLICY-001
type: weather_report_applicability_policy
status: APPROVED_SYSTEM_RULE_PENDING_PROVIDER_INTEGRATION
---

# BlueSky PRO — Weather Report Applicability and Reuse Policy 001

## 1. Purpose and scope

This policy defines how BlueSky PRO selects and reuses scheduled aviation weather reports and numerical forecast runs for a mission. All timestamps and intervals are UTC.

The policy separates:
- observed aerodrome weather: METAR and SPECI;
- aerodrome forecast: TAF, including AMD/COR/CNL amendments;
- route-area numerical forecast: NOAA GFS.

These products are not interchangeable. In particular, an aerodrome observation is not a route-wide forecast, and a GFS model cycle is not a METAR issue time.

The operational three-hour and fifteen-minute rules below are BlueSky planning rules. They do not replace a product's regulatory issue schedule or encoded validity period.

## 2. Regulatory issue schedules used as input

### 2.1 METAR

Under Russian FAP approved by Ministry of Transport Order No. 49 dated 05 February 2026, regular aerodrome weather reports are issued:
- at 00 and 30 minutes of each hour while flights are being conducted;
- at 00 minutes of each hour when no flights are being conducted, subject to the prescribed coordination.

For aerodromes with non-round-the-clock operating hours, observations begin two hours before the start of operations and continue during the operating period.

METAR is an observation report. Its issue cadence must not be interpreted as a three-hour regulatory validity period.

### 2.2 SPECI

SPECI is an event-driven special observation report issued when the agreed operational criteria are reached. It is not a periodic replacement for METAR.

- Criteria reached outside the regular 00/30-minute slots can cause SPECI issuance.
- If the criteria are reached exactly at a regular 00/30-minute slot, the regular METAR is issued instead.
- SPECI reporting deterioration is disseminated immediately after observation.
- An improvement report is issued only if the improvement persists for ten minutes, subject to the applicable coding instruction.

A received SPECI is a new observation event and invalidates the assumption that the previous METAR is still the latest observation. It triggers an immediate weather re-evaluation; it does not by itself prove that the route must be changed.

### 2.3 TAF

Under Russian FAP No. 49:
- regular TAF is issued every three hours;
- issue lead time is less than one hour and not less than 30 minutes before the start of its validity period;
- regular TAF validity is 6, 9, 24 or 30 hours;
- the exact validity start and end are encoded in the TAF validity group; the system must not infer a fixed start hour merely from the three-hour issue cadence;
- a newly issued TAF cancels an earlier TAF for the same aerodrome and the same validity period.

TAF selection is based on its encoded validity interval, not on a generic age limit. A newer AMD/COR product supersedes the applicable prior forecast; CNL cancels the forecast it identifies.

## 3. BlueSky operational applicability rule for METAR/SPECI

ETD is the planned takeoff time. The operational observation window is defined relative to ETD:

`[ETD - 3 hours, ETD + 3 hours]`

For pre-flight selection:
1. Only reports already received by the system may be selected; a future observation is never used.
2. Select the newest available METAR or SPECI whose observation time is not earlier than ETD minus three hours.
3. A SPECI received after the selected METAR immediately becomes the latest observation for re-evaluation.
4. The three-hour window is a planning reuse criterion, not a declaration that METAR is legally valid for three hours.

During mission execution, each newly received METAR/SPECI is evaluated as a new observation event. The system must retain report type, observation time, receipt time, station identifier and source provenance.

An aerodrome METAR/SPECI does not provide route-wide weather coverage. It must not be used as a substitute for route-area wind forecast data.

## 4. TAF coverage rule

For a planned mission interval, select the newest confirmed current TAF for the relevant aerodrome whose encoded validity interval covers the required aerodrome operation interval.

The system must not:
- extend TAF validity by applying the METAR three-hour rule;
- infer validity from issue time alone;
- use a cancelled or superseded TAF;
- claim route-wide coverage from an aerodrome TAF.

If no single current TAF covers the required interval, return a visible insufficient-coverage state rather than silently extrapolating.

## 5. Fifteen-minute departure tolerance

A takeoff is considered on time when actual takeoff occurs no later than 15 minutes after ETD.

For a delay of 15 minutes or less:
- retain the selected weather inputs without a new download if their applicable coverage still includes the actual takeoff and planned flight interval;
- do not trigger a recalculation solely because the clock crossed ETD;
- do trigger re-evaluation if a SPECI or other material weather update arrives.

For a delay greater than 15 minutes:
- re-evaluate report selection and forecast coverage against the revised planned takeoff and flight interval;
- reuse verified cached products when they remain applicable;
- download only missing or newly required products.

The 15-minute tolerance is a BlueSky operational rule, not a regulatory grace period.

## 6. GFS numerical forecast cycle rule

NOAA/NCEP GFS 0.25-degree runs are initialized at 00, 06, 12 and 18 UTC.

GFS selection remains governed by the GFS Forecast Planner Contract:
- consider only provider-confirmed or verified-cache cycles;
- choose the newest single run that brackets the complete mission execution interval;
- do not combine different model cycles into one time series;
- do not treat the 600-second availability-listing cache TTL as forecast validity;
- reuse already verified GRIB products while they still cover the required mission interval.

The three-hour METAR observation window is not applied as a GFS validity rule. A GFS run's applicability is determined by its forecast valid times and coverage of the full mission interval.

The existing lead schedule remains hourly from F000 through F120 and every three hours from F123 through F384.

## 7. Weather-driven recalculation sequence

1. The primary optimizer produces the initial route.
2. The system obtains the planned mission interval from the approved mission/route schedule.
3. The weather planner selects the applicable TAF and GFS inputs; the observation service selects the latest applicable METAR/SPECI where an aerodrome observation is relevant.
4. The weather calculation evaluates the initial route.
5. If the weather assessment changes feasibility or performance, the optimizer performs the weather-corrected calculation.
6. A new ordinary report does not cause repeated downloads or route calculations when the existing inputs remain applicable and the dependency identity has not changed.
7. SPECI triggers an immediate re-evaluation. A route recalculation is performed only when the new weather data changes the relevant performance, safety or feasibility result.

Weather retrieval must be scoped to the mission area, route, execution interval and required altitude levels. Map pan/zoom alone must not trigger a new weather acquisition.

## 8. Implementation status and integration boundary

The repository currently implements the NOAA GFS acquisition, availability, cache, GRIB2 decoding and route-wind calculation path.

The reviewed weather implementation does not yet provide an end-to-end METAR/SPECI/TAF acquisition and parsing path. The FAP also identifies GAMET/AIRMET, SIGMET, SIGWX and upper-wind/temperature products for relevant UAV operations; those products are not yet implemented in the reviewed weather engine. GFS wind data does not replace those regulated aviation products. Therefore:
- Sections 2–5 define the required aviation-report policy but are not evidence that METAR/SPECI/TAF integration is already operational.
- GFS cycle selection is implemented separately by GFS Forecast Planner.
- Automatic orchestration from the primary optimizer through weather assessment and weather-corrected optimization must be verified independently; this policy does not claim that the full sequence is already wired.

The HMI now derives a preliminary GFS interval from operator-entered ETD and the primary optimizer route's segment distance/nominal-speed values. This is only an acquisition coverage estimate; the weather trajectory engine recalculates segment traversal times using decoded wind. If the wind-corrected traversal exceeds the downloaded forecast coverage, the evaluation must remain blocked rather than extrapolate.

The 15-minute departure rule is implemented as a deterministic policy function, but the reviewed telemetry boundary still does not expose a confirmed actual-liftoff event. Until an adapter-specific liftoff event is mapped and verified, the HMI must not infer actual TOT from START/command acknowledgement and cannot automatically invoke the delay decision at runtime.

The current weather assessment returns feasibility and performance results, but an end-to-end command that submits the weather-corrected inputs back to AlgorithmOrchestrator and replaces the route candidate has not yet been verified. The automatic GFS acquisition is therefore integrated after validated primary-route publication; weather-driven re-optimization remains an integration gap.

The GFS planner continues to accept an explicit mission execution interval. Deriving that interval from ETD and route duration requires a finalized mission schedule contract and must not be guessed from route distance or nominal speed.

## 9. Regulatory references

1. Ministry of Transport of the Russian Federation, Order No. 49 dated 05 February 2026, Federal Aviation Rules "Rules for Providing Meteorological Information for Aircraft Operations", registered 07 April 2026, No. 85933. Official publication: https://publication.pravo.gov.ru/document/0001202604080030
2. Roshydromet, Order No. 244 dated 20 August 2026, Instruction on METAR, SPECI and TAF codes.
3. FAP No. 49, paragraph 145, explicitly lists METAR/SPECI, TAF, GAMET/AIRMET, SIGMET, SIGWX and upper-wind/temperature information to be provided to UAS owners/operators and external pilots.
4. NOAA/NCEP, GFS product inventory: https://www.nco.ncep.noaa.gov/pmb/products/gfs/nomads/

The applicability of Russian civil-aviation rules to a specific UAV operation must be confirmed against the aircraft category, airspace regime and applicable operating approvals.
