# Archive 3 source fingerprint — 2026-10-16

Reference archive: `BlueSkyPRO-Windows-Map-Review.zip`
Archive location in the current analysis environment: `/mnt/data/bspro_archives/BlueSkyPRO-Windows-Map-Review.zip`
Archive ZIP SHA-256 and executable hash were not recorded in this manifest.

## Files measured directly from Archive 3

Git blob SHA-1 is computed using Git's canonical blob header `blob <byte-length>\0` followed by the exact file bytes.

| Archive path | Bytes | Git blob SHA-1 | SHA-256 |
|---|---:|---|---|
| `BlueSky/PRO/qml/MapTilerWebMap.qml` | 21123 | `8d482963428a9e2a7474af6c9d1f452391575bae` | `f90a2fcc41a2fb464427da2584e5adcb8a8e705c85057e7c9277c37533f9d1e6` |
| `BlueSky/PRO/qml/FlightChart.ui.qml` | 12715 | `dfedc21787a5e47d8005aace8bff3b4ea7a1c92c` | `e44c096facf0c027312b6186eaac3ed11260e71bcd6a4cd4efab47dc15f0aced` |

## Comparison status

The files currently stored in the GitHub baseline branch do **not** byte-match the archive copies:
- Repository `MapTilerWebMap.qml` was 20779 bytes when measured; Archive 3 is 21123 bytes.
- Repository `FlightChart.ui.qml` was 12290 bytes when measured; Archive 3 is 12715 bytes.

The HTML base URL in the archive is `https://unpkg.com/`. Archive 3's style-change handler uses a full style reload. These individual settings are not sufficient to establish whole-file equivalence.

## Required next action

Synchronize the two source files from Archive 3 into a dedicated baseline commit, then build and run that exact commit on Windows. Do not treat this fingerprint-only record as a fully restored source baseline. Do not copy the compiled executable into Git as a substitute for source synchronization.
