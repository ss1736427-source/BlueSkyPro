# BlueSkyPro Performance Audit — Baseline

**Status:** initial source inspection; not a runtime profile.  
**Scope:** Qt 6 HMI, map tile pipeline, PlanningBridge.  
**Branch:** `fix/map-zoom-loading-provider-selector-2026-10-10`.

## Objective

Identify measurable bottlenecks before introducing broad concurrency or rewriting the Planning Core. Keep the HMI responsive, preserve deterministic/safety-relevant planning results, avoid unnecessary network traffic, and do not alter the authoritative main worktree during experiments.

## Source observations

| Component | Verified source behavior | Risk to measure | Next action |
|---|---|---|---|
| `src/PlanningBridge.cpp` | `startProcess()` calls `waitForStarted(3000)`; `sendRequest()` calls `waitForBytesWritten(1000)`; `stopProcess()` calls `waitForFinished(1000)` before killing the process. | These calls can block the calling thread while waiting. Impact depends on where the invokables are called. | Find all callers and confirm thread context before changing semantics. Prefer asynchronous state transitions where callers can handle them. |
| `src/TileCacheManager.cpp` | Uses `QNetworkAccessManager`, up to 6 active requests, and 36 ms spacing between request starts. It drops queued requests during view changes and stores tile bytes plus expiry metadata. | Queue delay, stale work, disk operations, server throttling, malformed response bodies. | Add timing/counter instrumentation; do not raise concurrency before measuring provider behavior. |
| `src/TileCacheManager.cpp` | A successful HTTP status and non-empty body are sufficient to proceed to cache/write or `tileReady`; image payload validity is not checked here. | Error pages or invalid image bytes can reach the QML image loader/cache. | Validate content conservatively before persistence; add tests for invalid responses before enabling this in production. |
| `qml/GoogleMapView.qml` | Rebuilds tile list on viewport/zoom changes; tracks visible tiles separately from overscan for loading status; uses fallback/stable layers. | Delegate readiness, tile coverage, visual fallback alignment, repeated network requests, QML frame-time stalls. | Instrument cache-hit vs network, visible/overscan readiness, failed images, and rebuild duration. Reproduce zoom defects after each isolated change. |
| `src/main.cpp` | Constructs `PlanningBridge` and `TileCacheManager` in the GUI application's main thread and exposes them to QML. | Blocking invokable calls can stall the UI; QObject affinity matters if moving work. | Preserve QObject thread-affinity rules; do not move `QNetworkAccessManager` across threads casually. |

## Measurement plan

Capture baseline and post-change measurements on the same machine, provider, geographic extent, zoom sequence, and cache state.

- Map: tile cache hit/miss counts; request queue wait; network duration/status/bytes; cache read/write duration; image decode/readiness duration; visible tile count; failed image count; viewport rebuild duration.
- UI: GUI frame time or frame drops during drag/zoom; main-thread stalls; memory peak.
- Planning: request enqueue/start/finish times, execution duration, cancellation count, input/output sizes, and result equivalence.
- Storage: cache file count/size, cache lookup time, cleanup duration, and cache expiry decisions.

Report percentiles (p50/p95/p99) for timings where sample size permits; report counts and failure rates as well. Avoid logging API keys, full credential-bearing URLs, mission-sensitive coordinates, or payloads.

## Implementation order

1. Add lightweight, opt-in diagnostics with a single consistent event format.
2. Reproduce map blank regions and long tile replacement while capturing measurements.
3. Fix confirmed causes one at a time, retaining the previous visual layer until replacements are usable.
4. Audit all `PlanningBridge` callers before changing synchronous API behavior.
5. Introduce bounded workers only for measured CPU-bound work; use asynchronous I/O for network and process communication.
6. Add regression tests and compare the same scenarios before/after.

## Architecture rules

- The GUI thread must not perform long CPU work, synchronous waits, or bulk disk operations.
- Every queue must have explicit priority, bounded resource use, cancellation semantics, and observable wait time.
- Superseded viewport work should not delay current visible tiles.
- Cache entries must be provider/style/zoom/x/y-specific and obey provider terms and cache headers.
- Background prefetch is bounded and opt-in/configurable; do not bulk-download provider datasets without authorization.
- Parallel execution must not change safety-relevant planning outputs without explicit verification.
- Do not merge performance changes solely because CI compiles: require runtime measurements and regression tests.

## Current limitations

- No runtime profiler trace has been collected for this audit.
- The current source review does not prove which component causes the observed latency or black map regions.
- No throughput, latency, memory, or frame-rate improvement is claimed yet.
