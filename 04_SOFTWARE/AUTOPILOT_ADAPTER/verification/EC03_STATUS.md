# EC-03 Navigation Adapter

**Status:** IMPLEMENTATION BASELINE — P0

EC-03 establishes the common navigation adapter contract for `SRC-06 GNSS_RTK_NTRIP` and a deterministic prototype/bench implementation.

Implemented:
- navigation source identity and protocol metadata;
- position, velocity and attitude sample model;
- fix type and navigation quality;
- RTK/NTRIP correction state;
- source/adapter timestamps and validity/staleness flags;
- baseline sample validation and storage;
- automated CTest coverage;
- CMake integration.

This is not physical navigation verification. The baseline accepts controlled prototype/replay/bench samples only. Real GNSS/RTK/NTRIP source integration, controlled execution evidence, domain processing/result linkage, integrity-controlled evidence package and physical/operational verification remain open.
