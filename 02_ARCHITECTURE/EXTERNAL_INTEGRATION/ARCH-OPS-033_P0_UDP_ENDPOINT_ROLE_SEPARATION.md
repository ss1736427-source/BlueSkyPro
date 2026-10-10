# ARCH-OPS-033 — P0 UDP Endpoint Role Separation

## Purpose
Remove the temporary coupling introduced by ARCH-OPS-032 between the UDP ingress admission peer and the outbound destination.

## Boundary
UDP ingress socket -> source endpoint -> ingress peer admission -> MAVLink session identity/sequence -> raw MAVLink2 decoder

Outbound path: channel runtime -> configured UDP remote destination -> UDP OS socket.

## Contract
MavlinkTransportChannelConfig now has two independent optional UDP roles:
- udp_ingress_peer — expected source endpoint for inbound frames.
- udp_remote — destination endpoint for outbound frames.

Runtime methods:
- setUdpIngressPeer() controls ingress admission.
- setUdpRemote() controls egress destination.

Neither endpoint is interpreted by the MAVLink decoder or normalized telemetry layer.

## Acceptance
- An inbound frame is admitted only when its source matches udp_ingress_peer, when configured.
- Changing udp_remote does not change inbound admission.
- Outbound transmission uses only udp_remote.
- Reconnect retains both endpoint roles.
- Existing deterministic-memory behavior remains unchanged.
- Real UDP loopback test exercises independent ingress and egress endpoints.

## Rationale
MAVLink supports systems with multiple communication channels and uses system/component IDs for routing. Packet sequence numbers are associated with the sending channel, so channel identity must remain explicit rather than being inferred from a current transmit destination.

## Evidence limitation
SIL_OS_UDP_ENDPOINT_ROLE_SEPARATION only. No radio, cellular, serial, field-network, SITL/HIL, physical-flight, or certification evidence.
