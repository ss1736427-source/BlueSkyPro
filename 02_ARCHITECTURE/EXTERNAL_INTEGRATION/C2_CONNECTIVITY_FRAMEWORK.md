# BlueSky PRO — C2 / Connectivity Framework

**Status:** WORKING BASELINE

## Objective

BlueSky shall operate over heterogeneous command-and-control/data-link environments without coupling mission logic to one radio, modem, network or transport.

## Connectivity model

```text
                     BlueSky C2 Manager
                            │
                  Link Abstraction Layer
                            │
          ┌─────────────────┼─────────────────┐
          │                 │                 │
       Primary           Backup            Local
       Link              Link              Link
          │                 │                 │
      RF/Radio            IP/Cell          USB/Local
          │                 │                 │
          └─────────────────┼─────────────────┘
                            │
                         UAV/FCS
```

## Required functions

- link discovery and registration;
- link health and quality measurement;
- primary/backup designation;
- automatic failover according to policy;
- recovery and controlled return to preferred link;
- connection-loss detection;
- command delivery state;
- telemetry continuity state;
- bandwidth/latency monitoring;
- queueing of non-time-critical data;
- prioritization of safety-critical traffic;
- authentication/integrity/security hooks;
- audit of link changes and outages.

## Link state machine

```text
DISCONNECTED
     ↓
CONNECTING
     ↓
CONNECTED
     ↓
DEGRADED
     ↓
LOST
     ↓
RECOVERING
     ↓
CONNECTED
```

## Failover principle

Loss of a link shall not automatically mean loss of the mission. BlueSky shall distinguish:

1. loss of primary C2;
2. loss of telemetry only;
3. loss of video only;
4. loss of Equipment control;
5. loss of all external communication.

The aircraft/autopilot contingency behavior remains authoritative for onboard autonomous safety actions. BlueSky supervises and coordinates according to the approved contingency policy.

## Multi-link policy

The system shall support, where hardware permits:

- RF + cellular/IP;
- redundant RF links;
- local wired service connection;
- relay/mesh/ground-network paths;
- future vendor-specific transports.

The same normalized C2 service shall be used above the transport layer.

## Security

Transport confidentiality, authentication, message integrity, key management and protocol-specific security shall be specified per link. MAVLink 2 signing, where applicable, is treated as message authentication/integrity and not as a replacement for encrypted transport.

## Operational evidence

Every loss, recovery, failover and degraded period shall be timestamped and associated with the affected vehicle and Flight Record.
