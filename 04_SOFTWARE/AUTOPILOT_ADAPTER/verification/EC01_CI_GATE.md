# EC-01 — CI Gate

The EC-01 branch shall pass the existing Autopilot Adapter workflow before merge.

Required checks:

1. adapter-chain validation;
2. Python verification tests;
3. CMake configure;
4. CMake build;
5. CTest execution including `c2_interface_baseline_test`.

A passing CI result validates the prototype build/test baseline only. It does not constitute physical C2 verification.
