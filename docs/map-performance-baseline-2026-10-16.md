# BlueSky PRO — tested map baseline (Archive #3)

- Baseline captured: 2026-10-16.
- Reference archive: `BlueSkyPRO-Windows-Map-Review.zip` (Archive #3; the 166 MB ZIP is not stored in Git).
- Project-owner validation: map is responsive; all expected layers are present; panning is smooth. Tiles may take time to load after zooming but eventually appear.
- Reported diagnostic snapshot: 164 style layers (70 line, 61 symbol, 31 fill, 1 background, 1 hillshade/3D-type); 0 map errors; 0 long tasks; style load about 246 ms; 243 tile/vector resources; maximum resource duration about 4.84 s.
- The source file `qt/BlueSkyPRO-HMI/qml/MapTilerWebMap.qml` is aligned with Archive #3 for the verified HTML base URL: `https://unpkg.com/`. The previous branch version used `https://api.maptiler.com/`.
- Renderer: MapLibre GL JS 6.14.0; MapTiler `hybrid-v4`; `maxTileCacheZoomLevels: 6`; labels enabled by default; style switching uses `diff: false` (full style replacement), matching Archive #3.
- API keys are not committed. Set `BLUESKY_MAPTILER_API_KEY` in the runtime environment.
- This branch records the tested settings and reported test record, but its QML source files do not yet byte-match Archive 3. See `docs/archive3-source-fingerprint-2026-10-16.md`. The original ZIP and compiled Windows executable are not embedded in Git.

## Validation boundary

Performance numbers are observations from the reported test snapshot, not guarantees for other machines or network conditions. No further performance tuning is included in this baseline commit.
