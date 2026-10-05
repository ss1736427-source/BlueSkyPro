# ARCH-OPS-029 — P0 MAVLink Transport Channel Runtime

## Objective

Bind the MAVLink session runtime to a concrete channel abstraction that can represent connection state, send/receive, failure, reconnect, statistics and per-channel routing without coupling BlueSky core to a physical transport.

## Contract

`Channel -> MavlinkSessionRuntime -> MavlinkRawDecoderBoundary -> DecodedMavlinkMessage`

## Responsibilities

- channel identity and configuration;
- explicit connect/disconnect;
- deterministic send/receive;
- channel failure and recovery;
- packet receive/transmit statistics;
- routing to the configured MAVLink session;
- heartbeat timeout propagation;
- simultaneous channel isolation.

## Invariants

1. Offline channels cannot transmit or inject receive data.
2. Connected channels route received frames only to their configured session.
3. Channel statistics are isolated per channel.
4. Channel failure does not mutate another channel.
5. Reconnect creates a recovering session state and resets session sequence state.
6. Heartbeat timeout is reflected at channel level.
7. Disconnect returns the channel to OFFLINE.
8. Transport/channel state remains outside normalized telemetry.

## Evidence boundary

`SIL_DETERMINISTIC_TRANSPORT_CHANNEL`

This slice uses an in-process deterministic transport seam. It does not claim live serial/UDP/TCP/radio operation, SITL/HIL, physical flight or certification evidence.

## Verification

The deterministic test covers:

1. two simultaneous channels;
2. connection establishment;
3. send;
4. receive and routing;
5. per-channel statistics;
6. channel failure;
7. transmit rejection while failed;
8. reconnect and session reset;
9. heartbeat recovery;
10. session timeout propagated to channel;
11. independent disconnect of the second channel.

## Next

Bind the first real OS/network transport driver to this stable channel interface without changing the session or decoder contracts.
