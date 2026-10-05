#!/usr/bin/env python3

from __future__ import annotations

import sys

from zone_sizing import CapabilityWeight, ZoneSizingError, weighted_area_targets


def main() -> int:
    targets = weighted_area_targets(
        1000.0,
        [CapabilityWeight("UAV-01", 1.0), CapabilityWeight("UAV-02", 3.0)],
    )
    assert abs(targets["UAV-01"] - 250.0) < 1e-9
    assert abs(targets["UAV-02"] - 750.0) < 1e-9
    assert abs(sum(targets.values()) - 1000.0) < 1e-9

    try:
        weighted_area_targets(1000.0, [CapabilityWeight("UAV-01", 0.0)])
    except ZoneSizingError:
        pass
    else:
        raise AssertionError("zero capability weight must fail")

    print("ZONE SIZING TESTS: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
