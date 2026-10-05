# ARCH-OPS-039 — P0 Fleet addressing

## Requirement

G2-10 requires multi-UAV system/component addressing, routing and collision prevention.

## Implemented slice

ARCH-OPS-039 introduces a deterministic fleet addressing runtime beneath the BlueSky operational layer. It binds an existing vehicle identity to a MAVLink system/component address and preserves source/session identity separately.

The registry prevents two registered vehicles from occupying the same system/component address and rejects conflicting re-registration of an existing vehicle. It provides deterministic lookup in both directions for downstream routing components.

## Architectural rule

The registry does not become the network transport. Physical routing, address allocation, broadcast discovery and network-level collision avoidance remain separate integration concerns. MAVLink address values are supplied by the configuration/integration layer rather than silently generated here.

## Lifecycle

CONFIGURATION / DISCOVERY
        ↓
FleetAddressBinding
        ↓
FleetAddressingRuntime
   ┌────┴────┐
address → UAV  UAV → address
        ↓
DOWNSTREAM ROUTING / SESSION

## Evidence

SIL_FLEET_ADDRESSING_RUNTIME

The evidence demonstrates deterministic local registration, collision rejection, resolution and release. It does not demonstrate physical network collision avoidance or field operation.
