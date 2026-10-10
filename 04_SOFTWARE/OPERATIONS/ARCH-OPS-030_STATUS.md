# ARCH-OPS-030 STATUS

**Status:** IMPLEMENTATION BASELINE — MAVLink UDP TRANSPORT DRIVER

## Implemented
- real OS UDP socket lifecycle;
- IPv4 bind with dynamic port support;
- non-blocking receive;
- datagram send;
- source endpoint preservation;
- transport statistics;
- portable Windows Winsock2 and POSIX socket boundary;
- deterministic loopback integration test.

## Evidence
**SIL_OS_UDP_LOOPBACK**

The test exercises a real operating-system UDP socket on the local loopback interface. It is not evidence of radio, cellular, serial, field-network, SITL/HIL or physical-flight behaviour.

## Architectural boundary
UDP Driver -> Transport Channel Runtime -> MAVLink Session Runtime -> Raw Decoder

The driver contains only transport concerns. MAVLink parsing remains in the existing raw decoder.

## Next mandatory slice
Bind the UDP driver into the transport channel runtime so a configured UDP channel can use the real socket for receive/send while retaining the deterministic channel seam for tests.
