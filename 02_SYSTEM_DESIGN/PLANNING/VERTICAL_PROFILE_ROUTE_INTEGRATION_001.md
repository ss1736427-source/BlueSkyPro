# Vertical Profile Route Integration

**ID:** PLAN-GEN-002
**Status:** BASELINED

The launch and recovery vertical profiles are explicit route segments, not a UI-only visualization.

For a feasible route:
LAUNCH → VERTICAL CLIMB → CRUISE → VERTICAL DESCENT → RECOVERY.

The builder requires both launch and recovery transition profiles to be feasible and requires cruise altitude to satisfy both transition altitudes.

This contract does not replace constrained spatial validation. The resulting route must still pass route constraint validation and the authorization-qualified constrained environment before becoming a candidate for selection.

No authorization or execution authority is created by this builder.
