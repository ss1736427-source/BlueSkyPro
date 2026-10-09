# MapTiler Hybrid v4 Integration — Technical Checkpoint 2026-10-09

## Status

**IN PROGRESS — selectable 2D raster basemaps implemented; MapTiler SDK vector rendering and 3D remain unimplemented.** The existing map view is a custom Qt Quick raster-tile renderer. It cannot provide vector rendering, camera pitch, 3D terrain, or extruded building layers merely by changing the tile URL.

## Current baseline

- Working branch: `fix/hmi-map-tile-loading-2026-10-09`.
- Baseline before provider integration: `50f8765994289b610807864513ce2b635b79d700` (`feat(map): add compact map zoom controls`).
- Provider integration commit: `432cca3dc75f8f35bf48842ace62727ef6e56946` (`feat(map): add selectable Yandex and MapTiler Hybrid basemaps`).
- Cache-format correction: `2b3219c3c8b2e360ed7a3f6d2d30f00af7b769ac` (`fix(map): align Hybrid raster format with tile cache`); MapTiler raster tiles use PNG to match the cache manager's `.png` files.
- `qt/BlueSkyPRO-HMI/qml/GoogleMapView.qml` uses a custom `Image` tile repeater, QML coordinate-to-world calculations, `Canvas` route rendering, `DragHandler`, and `WheelHandler`.
- `GoogleMapView.qml` now offers an on-map selector for `MapTiler Hybrid v4`, `MapTiler Hybrid v4 Dark`, and `Яндекс Карты`.
- When `BLUESKY_MAPTILER_API_KEY` exists, MapTiler Hybrid v4 is the initial selection. Without that key, Yandex is the initial selection.
- MapTiler Hybrid styles are currently requested through the documented raster XYZ endpoint; this is a 2D raster integration, not the MapTiler SDK vector renderer.
- Tile cache keys include the selected MapTiler style identity to prevent cross-style tile collisions. Yandex remains selectable and uses its existing endpoint.
- `qt/BlueSkyPRO-HMI/CMakeLists.txt` currently finds Qt Quick and Network and does not declare a Qt WebEngine dependency.
- `qt/BlueSkyPRO-HMI/src/main.cpp` supplies API keys to QML through environment variables. The key must remain outside source control.
- The existing route overlay must continue to consume only authoritative WGS84 geometry; no preview or synthetic coordinates may be introduced.

## External technical findings

- MapTiler documents Hybrid v4 style variants `hybrid-v4` and `hybrid-v4-dark`.
- MapTiler SDK JS extends MapLibre GL JS and supports vector styles, terrain elevation, pitch/bearing, and 3D layer APIs.
- 3D terrain requires the SDK terrain/elevation capability and elevation data. 3D buildings require a compatible vector source/layer and building-height attributes; a Hybrid style URL alone is not proof that buildings are extruded.
- The SDK's documented integration model is browser JavaScript/CSS. To use it inside this Qt Quick desktop app, a supported embedded browser/runtime is needed unless the project chooses a different native vector renderer.

## Architecture decision for the next implementation step

Use a dedicated map-engine adapter behind the existing map-view contract. Keep the existing raster renderer as a fallback until the new renderer is verified. Do not replace the current map in-place before the SDK adapter can initialize and report failure safely.

### Dependency-gate findings (2026-10-09)

- Current developer environment is Qt 6.11.2 with MinGW 13.1. Qt's current WebEngine platform notes state that Qt WebEngine does not compile with MinGW; therefore a Qt WebEngine-based implementation is incompatible with the established local toolchain unless the toolchain changes.
- Current HMI CI is `.github/workflows/bluesky-pro-hmi.yml`: Ubuntu 24.04, Qt 6.8.3 Linux GCC, and only `qtshadertools` as an additional module. It does not install WebEngine, test Windows, or package a deployable application.
- Qt's current Qt WebView configuration enables its native WebView2 plugin only when `WIN32 AND MSVC AND WebView2_FOUND`; this does not support assuming WebView2 is available in the existing Windows MinGW kit. Qt WebView also documents that overlapping WebView content with QML components is unsupported/unpredictable, which conflicts with BlueSky PRO's map-first UI and map overlays.
- Qt WebEngine is not a viable drop-in dependency for the established MinGW toolchain, and the current Linux CI does not install it or package its runtime.
- Therefore, **do not select Qt WebView/WebEngine as the map host under the current toolchain**. Do not change the project's compiler or add browser dependencies merely to enable the map SDK. Keep the raster renderer as the stable fallback while evaluating native vector rendering.
- Initial native candidate: **MapLibre Native for Qt** (`maplibre/maplibre-native-qt`). Its upstream project provides Qt 6 QML/Qt Location bindings, documents support for Qt 6.5+, and provides Windows and Linux build workflows. It uses an OpenGL backend for the QML/Qt Location path. This makes it a better architectural candidate than a browser host, but it is **not yet verified** against BlueSky PRO's exact Qt 6.11.2 MinGW kit, Qt modules, current CI, licensing/deployment needs, or map style/data source.
- Important compatibility details to verify in a proof of concept: the upstream project requires Qt Core/Gui/Network/Sql and Quick plus Location for the QML plugin; Qt 6.9+ builds also require Qt Location private components. Its current CMake project uses C++20 internally, so the external dependency must be built separately rather than forcing an unreviewed language-standard change onto BlueSky PRO's C++17 app.
- External sources: Qt WebEngine deployment documentation; Qt WebEngine platform notes; Qt WebView platform documentation; `jurplel/install-qt-action` module documentation.

The adapter must preserve:
1. Map center and zoom, plus map drag and zoom interaction.
2. Existing side-panel wheel isolation.
3. Route overlay fed from the current authoritative WGS84 route binding.
4. API key supplied at runtime from `BLUESKY_MAPTILER_API_KEY`; never embed a live key in HTML, QML, or repository files.
5. Map attribution and explicit loading/error states.
6. A clear distinction between 2D Hybrid, 3D terrain, and 3D buildings. Each capability must be independently enabled and tested.

## Dependency gate and selected next step

The Qt WebView/WebEngine route is **rejected for the current toolchain** unless future evidence demonstrates a supported, maintainable configuration without disrupting Windows MinGW and Linux CI.

Next deterministic step:
1. Preserve the existing raster map and Yandex fallback; do not replace or regress current pan/zoom, tile cache, panel wheel isolation, provider selection, attribution, or authoritative route overlay.
2. Evaluate MapLibre Native for Qt first. Before integrating it into the main app, establish a separate proof of concept against Qt 6.11.2 MinGW on Windows and the Linux CI kit. Confirm the dependency actually compiles, then validate licensing, style/data-source compatibility, deployable runtime, and plugin packaging.
3. The proof of concept must display a vector basemap, accept center/zoom updates, render only authoritative WGS84 route geometry, and coexist with QML panels without relying on unsupported browser overlays. The production raster renderer must remain selectable throughout this experiment.
4. Treat pitch, terrain, and 3D buildings as separate capabilities. Do not claim or implement them until the chosen engine and data source demonstrably support them.
5. Keep API keys runtime-supplied; pin dependencies and document online/offline limitations.
6. Verify the exact commit SHA, diff, CI, and Windows runtime before promoting the new engine beyond an experimental adapter.

## Verification state

- Existing raster map and zoom controls: source present in baseline commit; runtime behavior for commit `50f8765994289b610807864513ce2b635b79d700` is not confirmed by this checkpoint.
- Selectable Yandex + MapTiler Hybrid v4 raster basemaps: **IMPLEMENTED IN SOURCE / HMI CI PASS for commit `5aaa5ee9ffc5b56b6f07f201d88e2c260f2a5665` (workflow `BlueSky PRO HMI Qt Build`, run #188; Configure, Build, and HMI contract tests succeeded) / WINDOWS RUNTIME UNVERIFIED**.
- MapTiler SDK JS embedded in Qt: **NOT IMPLEMENTED / NOT VERIFIED**.
- Hybrid v4 vector rendering: **NOT IMPLEMENTED**.
- 3D terrain: **NOT IMPLEMENTED**.
- 3D buildings: **NOT IMPLEMENTED**.
- Windows packaging and runtime: **UNVERIFIED**.

## Next work item and decision gate

1. Existing raster provider implementation has HMI CI evidence from run #188 for source commit `5aaa5ee9ffc5b56b6f07f201d88e2c260f2a5665` (Configure, Build, and HMI contract tests passed). The subsequent commit `fb8e4cb122666625b4d984ff0506684689c2b672` changes only this checkpoint document; no HMI source changed. Windows runtime remains unverified; test all three provider choices, key-missing states, cached tile separation, and retained Yandex loading.
2. Dependency gate is now investigated: Qt WebView's WebView2 backend is configured for MSVC, QML overlays over WebView are unsupported/unpredictable, and Qt WebEngine does not fit the established MinGW workflow. Do not implement an SDK-in-WebView adapter under the current toolchain.
3. MapLibre Native for Qt is the first native candidate based on its official Qt 6 QML bindings and Windows/Linux build workflows, but remains an unverified candidate rather than an approved production dependency. The next implementation gate is a separate build/render proof of concept on the exact Windows MinGW kit and compatible Linux CI. Keep the current raster/Yandex renderer as the stable fallback; terrain and building extrusion remain later, separately verified capabilities.
4. Keep all route overlays sourced from authoritative WGS84 route geometry; do not touch route authority or synthesize coordinates.


### Native candidate compatibility check — 2026-10-09

The official MapLibre Native for Qt Windows CI currently builds Qt 6.11.2 with MSVC 2022 (x64/ARM64); it does not establish support for the project's Qt 6.11.2 MinGW 13.1 kit. Therefore MinGW compatibility remains **UNVERIFIED** and the library must not be integrated before an isolated compiler check. The QML integration also requires Qt Location/Positioning and Qt Location private components on Qt 6.9+, while the current HMI CMake project declares only Qt Quick and Network. The upstream core is BSD-2-Clause, but its QML bindings have separate GPL/LGPL licensing options; deployment/compliance needs review before shipping.

Next gate: verify MinGW build compatibility without changing the application's compiler or C++17 setting. If this cannot be established, keep the current raster/Yandex renderer and evaluate another explicitly compatible native approach.

### MinGW compatibility proof-of-concept — result 2026-10-09

**RESULT: BUILD PASS for the upstream MapLibre Native for Qt repository on Windows x64 with Qt 6.11.2 MinGW 13.1.0.** This closes the narrow compiler-compatibility question that was previously unverified; it does not approve production integration.

Evidence:
- BlueSky PRO workflow run (push): https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37937745619 — job completed successfully.
- Pull-request workflow run: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37937752269 — job completed successfully.
- Tested Qt root: `D:\\a\\BlueSkyPro\\Qt\\6.11.2\\mingw_64`.
- Compiler: `g++.exe (x86_64-posix-seh-rev1, Built by MinGW-Builds project) 13.1.0`.
- CMake configuration and generation succeeded; native library build and install steps succeeded.
- Experimental installed artifact: `maplibre-native-qt-6.11.2-mingw-poc`, artifact ID `11621352510`, SHA-256 `d68bbe172f9d69e8571ce531bbc4dc0be28ae110c72ab3f58491b09be970f8c9`, expires 2026-10-16.
- Tested source is upstream `maplibre/maplibre-native-qt` branch `main` as checked out by the workflow. The workflow currently does not pin an upstream commit; reproducibility requires pinning before relying on the artifact for production.

Scope limitations:
- This is a dependency build/install proof only. No BlueSky PRO application integration, plugin load, map rendering, center/zoom interaction, authoritative route overlay, Windows packaging, or runtime test was performed.
- It does not resolve QML bindings licensing, attribution/data-source suitability, style compatibility, deployment footprint, offline behavior, or Linux CI integration.
- Keep the existing raster MapTiler/Yandex renderer unchanged as the production fallback. Do not mark vector/3D capabilities implemented or verified.

Next deterministic step: inspect the exact upstream commit used by the successful run and pin it in the experimental workflow; then verify that the installed artifact contains the expected CMake packages, libraries, and QML plugin before attempting a minimal isolated render proof of concept. Production integration remains blocked until the render contract and licensing/deployment gates are addressed.

### Installed MinGW artifact inventory — 2026-10-09

The published artifact `maplibre-native-qt-6.11.2-mingw-poc` (artifact ID `11621352510`) was downloaded and inspected as a ZIP. The archive contains 522 entries and includes:
- `bin/QMapLibre.dll`, `bin/QMapLibreLocation.dll`, and `bin/QMapLibreQuickPrivate.dll`.
- `lib/cmake/QMapLibre/QMapLibreConfig.cmake` and related CMake target files.
- `plugins/geoservices/qtgeoservices_maplibre.dll`.
- `qml/MapLibre/declarative_maplibre.dll` and `qml/MapLibre/Location/declarative_maplibre_locationplugin.dll`.

This confirms the **presence of expected package/plugin files in the installed artifact**, not that Windows can load the plugins or render a map. The official standalone QML example was configured and built successfully in workflow run `37970467018` (https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37970467018), associated with BlueSky PRO commit `e464f318248ffb6c2bba1fbc05a0bfb92d94e8fd`. All workflow steps passed, including `Configure official QML standalone example` and `Build official QML standalone example`. Artifact `maplibre-native-qt-6.11.2-mingw-poc`, ID `11636674464`, size 6,215,965 bytes, SHA-256 `632d2515821ad0ed5108deaa8c02e52bcb380e614b0ef134fdb82f772fc6d30a`, expires 2026-10-16. This closes the isolated compile/configure gate for the upstream library and official example with Qt 6.11.2 MinGW. It does not prove that the example launches, loads QML plugins at runtime, fetches style/tiles, or displays a map. Runtime rendering, route overlay integration, Windows deployment, licensing, and data attribution remain unverified.

### MapLibre QML startup smoke-test — result 2026-10-09

**RESULT: CI PASS for native library build, install, official standalone QML example build, and limited process-liveness smoke test. Actual map rendering remains UNVERIFIED.**

Tested BlueSky PRO commit: `2fb71f6bc8665e40a549d726b81715597246361c` (workflow change only).

Evidence:
- Push workflow run: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37979000144 — completed successfully on 2026-10-09; job `113984425140`.
- Pull-request workflow run: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37979007701 — completed successfully on 2026-10-09; job `113984451094`.
- Both runs passed: native library configure/build/install, official QML standalone example configure/build, `Runtime smoke test — QML startup`, compiler/Qt version recording, and artifact upload.
- Experimental install artifact from push run: `maplibre-native-qt-6.11.2-mingw-poc`, artifact ID `11639848814`, size 6,215,968 bytes.
- Experimental install artifact from pull-request run: `maplibre-native-qt-6.11.2-mingw-poc`, artifact ID `11641123350`, size 6,215,966 bytes.
- Both artifacts were available when checked; retention is governed by the workflow's 7-day policy.

Interpretation and limitations:
- The smoke test checks that the GUI process remains alive for 15 seconds. The upstream example exits with an error if QML root objects are empty, so process liveness is a useful limited startup signal.
- The smoke test does **not** verify visible map pixels, successful style or tile network requests, QML plugin load diagnostics beyond process liveness, center/zoom interaction, or route overlays.
- This closes the isolated MinGW build/install and official-example build gate and passes the configured startup smoke test. It does **not** approve production integration and does not change the existing MapTiler/Yandex raster renderer.
- Still required before production integration: controlled visual/runtime evidence that the vector basemap actually renders, style/data-source and attribution review, QML bindings licensing review, deployable runtime/package validation, Linux CI compatibility, and a test adapter preserving authoritative WGS84 route geometry and existing UI interactions.

Next deterministic step: establish a stronger render-level proof of concept that records an objective signal of successful style/map rendering (prefer an automated screenshot/pixel or explicit renderer/style-loaded signal) without changing the production renderer. Keep API credentials outside source control and do not treat this smoke test as evidence of map rendering.

### Render-level smoke-test — result 2026-10-09

**RESULT: FAIL / actual MapLibre rendering remains UNVERIFIED.** The first automated render-probe attempt exposed a native process crash, not a pixel-threshold failure.

Evidence:
- Push workflow run: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37982494344 — failed at `Runtime smoke test — rendered map pixels`, job `113996712681`.
- Pull-request workflow run: https://github.com/ss1736427-source/BlueSkyPro/actions/runs/37982503603 — failed at the same step, job `113996753095`.
- Both runs passed the native dependency build/install and official standalone QML example build before the new probe step.
- The diagnostic process exited with code `-1073741819` (`0xC0000005`, Windows access violation) before a screenshot could be validated.
- Available logs do not establish whether the crash originates in graphics-context setup, MapLibre rendering, QML loading, or the screenshot-capture approach. Do not label any one of these as the root cause without additional evidence.

Next diagnostic step:
1. Keep the production raster/Yandex map unchanged.
2. Improve crash observability in the isolated proof of concept (capture QML/Qt diagnostics and, if feasible, a Windows crash dump) and separate “custom QML startup” from “map/style initialization” and “screenshot capture” so each can be tested independently.
3. Re-run the minimal probe on the same pinned MapLibre revision and Qt 6.11.2 MinGW toolchain.
4. Only after identifying and correcting the failure, re-enable a render assertion. Do not weaken the test to a process-liveness-only check and do not claim visual rendering verified.
