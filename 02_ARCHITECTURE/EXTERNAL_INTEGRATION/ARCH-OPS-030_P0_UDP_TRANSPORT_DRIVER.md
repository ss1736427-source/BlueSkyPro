# ARCH-OPS-030 — P0 MAVLink UDP Transport Driver

## Objective
Provide the first real OS/network transport implementation behind the established transport-channel boundary.

## Contract
UDP socket -> raw MAVLink frame -> MavlinkSessionRuntime -> MavlinkRawDecoderBoundary

## Responsibilities
- socket open/close;
- IPv4 endpoint binding;
- non-blocking receive;
- datagram transmit;
- peer endpoint observation;
- transport-level statistics;
- platform boundary isolation.

## Invariants
1. The driver never interprets MAVLink payload semantics.
2. UDP failure is distinct from MAVLink decode failure.
3. Receive returns raw datagrams unchanged.
4. A closed driver cannot transmit or receive.
5. The driver does not create or modify MAVLink session state.
6. Platform-specific socket APIs remain inside the driver.

## Evidence boundary
SIL_OS_UDP_LOOPBACK

Real OS loopback socket evidence only. No field-network, radio, cellular, SITL/HIL, physical-flight or certification evidence.

## Verification
The test covers two real loopback UDP sockets, dynamic ports, transmit, receive, peer preservation, invalid destination rejection and close behaviour.

## Next
Bind this driver into the stable transport-channel runtime.
