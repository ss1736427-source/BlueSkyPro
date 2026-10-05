# IF-EQUIPMENT — Equipment Interface

## Purpose
Define the integration boundary between BlueSky/HUB and mission equipment systems.

## Equipment classes
Camera, EO/IR, thermal, LiDAR, gimbal, other mission equipment.

## Integration boundary

```text
BlueSky / HUB
     ↓
Adapter Registry
     ↓
Equipment Adapter
     ↓
Equipment Controller / Device API
     ↓
Equipment
```

The Adapter Registry performs lookup and compatibility resolution only. It does not authorize operations or bypass safety controls.

Canonical equipment semantics are defined by:

```text
BLUESKY-CANONICAL-VEHICLE-EQUIPMENT-SCHEMA-001
BLUESKY-UNIVERSAL-ADAPTER-CONTRACT-001
BLUESKY-ADAPTER-REGISTRY-BOUNDARY-001
```

## Command path
BlueSky → Authority/Safety Gate → Adapter Registry → Equipment Adapter → Equipment Controller → Equipment.

## Data path
Equipment → Equipment Adapter/HUB → Mission Data Store → Flight Record / processing.

## Required capabilities
- device discovery
- capability identification
- status/health
- command execution and acknowledgement
- trigger/scheduling
- recording state
- timestamp and mission correlation
- metadata association
- failure/degraded-state reporting

## Key requirement
Equipment integration must be adapter-based so UAV/autopilot integration is independent of a particular equipment manufacturer.

External vendor terminology such as `payload`, where present in a protocol, is normalized to canonical `Equipment` at the adapter boundary.
