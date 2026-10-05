# IF-VIDEO — Video Interface

## Purpose
Define live and recorded video integration independently from flight-control telemetry.

## Flow
Camera → Encoder/Stream Source → Transport → HUB → Pilot/Operator.

Recorded media → Storage → HUB → Mission Data Store.

## Required functions
- stream discovery
- codec/profile information
- start/stop recording
- live stream state
- stream health and latency
- timestamp synchronization
- mission/UAV association
- recording recovery/state after link interruption

## Key requirement
Video transport must not be coupled to MAVLink command/telemetry transport; the two channels may share physical infrastructure but have independent logical contracts.
