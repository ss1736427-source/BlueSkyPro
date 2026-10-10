# ARCH-OPS-006 — Authority / Automation Resolution

**Status:** IMPLEMENTED — verification pending

## Implemented

The Operational Orchestrator now has a deterministic resolution step between an actionable action and execution authority.

The implementation:

- preserves the existing `AutomationLevel` contract;
- applies safety, regulatory, energy and action-authorization gates first;
- resolves automatic execution against system authority;
- resolves human-guided actions against role authority;
- keeps explicit human decisions pending until granted;
- fails closed when mandatory authority or gates are absent.

Implementation: `04_SOFTWARE/OPERATIONS/core/authority_automation_resolution.hpp`

Verification: `04_SOFTWARE/OPERATIONS/core/authority_automation_resolution_test.cpp`

## Architectural position

```text
READINESS / ACTION GRAPH
        ↓
ACTIONABLE ACTION
        ↓
AUTHORITY / AUTOMATION RESOLUTION
        ↓
NEXT ACTION / EXECUTION PATH
```

This component does not execute UAV commands and does not replace authentication, role management, safety, regulatory, energy, runtime or FCS contracts.

## Verification gate

Completion requires successful CMake build and CTest execution in the PR CI for the exact implementation revision.
