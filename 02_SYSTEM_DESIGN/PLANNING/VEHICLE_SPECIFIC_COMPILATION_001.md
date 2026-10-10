# Vehicle-Specific Mission Compilation

**ID:** PLAN-COMP-001
**Status:** BASELINED

Vehicle-specific compilation transforms an already validated Flight Profile into a UAV/configuration-specific planning artifact.

Inputs:
- selected Flight Profile;
- UAV identity;
- configuration version;
- performance model/version;
- vehicle altitude and speed limits.

The compiler validates compatibility and preserves profile geometry/altitudes. It does not recalculate terrain, airspace, wind, route candidates, multi-UAV conflicts or authorization.

A changed UAV configuration or performance dependency invalidates this compilation and downstream results only. The canonical route/profile remains reusable when its dependencies are unchanged.

The compiled artifact is not an authorization or execution command and does not establish READY.
