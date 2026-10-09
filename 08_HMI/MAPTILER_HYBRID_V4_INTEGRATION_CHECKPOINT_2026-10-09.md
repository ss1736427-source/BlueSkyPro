# MapTiler Hybrid v4 Integration — Technical Checkpoint 2026-10-09

## Status

**IN PROGRESS — SDK integration not implemented.** The existing map view is a custom Qt Quick raster-tile renderer. It cannot provide the MapTiler SDK's vector rendering, camera pitch, 3D terrain, or extruded building layers merely by changing the tile URL.

## Current baseline

- Working branch: `fix/hmi-map-tile-loading-2026-10-09`.
- Baseline commit: `50f8765994289b610807864513ce2b635b79d700` (`feat(map): add compact map zoom controls`).
- `qt/BlueSkyPRO-HMI/qml/GoogleMapView.qml` uses a custom `Image` tile repeater, QML coordinate-to-world calculations, `Canvas` route rendering, `DragHandler`, and `WheelHandler`.
- MapTiler currently supplies raster `dataviz-dark` tiles. Provider selection and tile caching are handled in QML/C++.
- `qt/BlueSkyPRO-HMI/CMakeLists.txt` currently finds Qt Quick and Network and does not declare a Qt WebEngine dependency.
- `qt/BlueSkyPRO-HMI/src/main.cpp` supplies API keys to QML through environment variables. The key must remain outside source control.
- The existing route overlay must continue to consume only authoritative WGS84 geometry; no preview or synthetic coordinates may be introduced.

## External technical findings

- MapTiler documents Hybrid v4 style variants `hybrid-v4` and `hybrid-v4-dark`.
- MapTiler SDK JS extends MapLibre GL JS and supports vector styles, terrain elevation, pitch/bearing, and 3D layer APIs.
- 3D terrain requires the SDK terrain/elevation capability and elevation data. 3D buildings require a compatible vector source/layer and building-height attributes; a Hybrid style URL alone is not proof that buildings are extruded.
- The SDK's documented integration model is browser JavaScript/CSS. To use it inside this Qt Quick desktop app, a supported embedded browser/runtime is needed unless the project chooses a different native vector renderer.

## Architecture decision for the next implementation step

Use a dedicated map-engine adapter behind the existing map-view contract, with MapTiler SDK JS hosted in an embedded Qt WebEngine view if the supported Qt packages are available in both the developer environment and CI. Keep the existing raster renderer as a fallback until the new renderer is verified. Do not replace the current map in-place before the SDK adapter can initialize and report failure safely.

The adapter must preserve:
1. Map center and zoom, plus map drag and zoom interaction.
2. Existing side-panel wheel isolation.
3. Route overlay fed from the current authoritative WGS84 route binding.
4. API key supplied at runtime from `BLUESKY_MAPTILER_API_KEY`; never embed a live key in HTML, QML, or repository files.
5. Map attribution and explicit loading/error states.
6. A clear distinction between 2D Hybrid, 3D terrain, and 3D buildings. Each capability must be independently enabled and tested.

## Dependency gate

Before writing the WebEngine QML adapter:
1. Inspect the active HMI workflow on the current branch and identify its Qt version, modules, and deployment packaging.
2. Confirm that the required Qt WebEngine module is available for the supported Windows Qt kit and CI kit.
3. Update the build/deployment workflow and CMake dependency in the same logical change; do not leave the application dependent on a locally installed module absent from CI.
4. Add a minimal embedded-page smoke test before adding terrain/building overlays.
5. Pin a supported MapTiler SDK JS version and load it in a way consistent with the project's online/offline and security requirements.
6. Verify current commit SHA, diff, CI for that exact SHA, then run the Windows app and test the live map.

## Verification state

- Existing raster map and zoom controls: source present in baseline commit; runtime behavior for commit `50f8765994289b610807864513ce2b635b79d700` is not confirmed by this checkpoint.
- MapTiler SDK JS embedded in Qt: **NOT IMPLEMENTED / NOT VERIFIED**.
- Hybrid v4: **NOT IMPLEMENTED**.
- 3D terrain: **NOT IMPLEMENTED**.
- 3D buildings: **NOT IMPLEMENTED**.
- Windows packaging and runtime: **UNVERIFIED**.

## Next deterministic work item

Inspect the exact current-branch HMI CI workflow and Qt deployment/package process. Resolve the WebEngine availability/deployment gate before adding any SDK UI code. Keep the change isolated to map integration and its tests; do not touch planning route authority or synthesize route geometry.
