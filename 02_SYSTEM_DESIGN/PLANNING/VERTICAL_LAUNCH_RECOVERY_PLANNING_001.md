# Vertical Launch and Recovery Planning

**ID:** PLAN-VLR-001
**Status:** BASELINED

## Purpose

Define the deterministic vertical launch/recovery segment used when a mission starts or ends inside an otherwise authorized regulatory restriction.

## Calculation

The safe transition altitude is the maximum of:
- terrain elevation plus required clearance;
- applicable regulatory minimum altitude;
- UAV/configuration minimum transition altitude;
- obstacle clearance altitude plus required clearance.

If the resulting altitude exceeds the applicable UAV maximum altitude, the vertical profile is infeasible.

## Boundary

Authorization can qualify a regulatory restriction, but it does not remove physical obstacles or clearance requirements. This calculation does not authorize flight, modify readiness, or execute the vehicle.

The result is a versioned planning dependency and must be consumed by route generation/validation.