# ARCH-OPS-033 Status

Status: IMPLEMENTED — CI pending

## Scope
Separate UDP ingress peer admission from the outbound UDP destination.

## Implemented
- Added independent udp_ingress_peer channel configuration.
- Added setUdpIngressPeer() runtime control.
- UDP ingress admission now checks only udp_ingress_peer.
- UDP transmit continues to use udp_remote.
- Reconnect preserves both independent endpoint roles.
- Real UDP loopback test verifies that outbound destination changes do not alter ingress admission.
- Deterministic-memory transport remains unchanged.

## Evidence
Target evidence class: SIL_OS_UDP_ENDPOINT_ROLE_SEPARATION.

This remains local OS loopback evidence. It does not establish radio, cellular, serial, field-network, SITL/HIL, physical-flight, or certification evidence.

## Architectural rationale
Transport endpoint identity and MAVLink session identity remain separate. The ingress peer is an admission constraint, while the remote endpoint is an egress destination. This avoids coupling receive authorization to the current transmit target and prepares the channel abstraction for multi-link routing.

MAVLink routing supports systems with multiple channels, and packet sequence numbers are channel-scoped for packet-loss interpretation.
