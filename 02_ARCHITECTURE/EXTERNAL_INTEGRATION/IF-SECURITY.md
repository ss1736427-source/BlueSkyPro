# IF-SECURITY — External Security Interface

## Purpose
Define security controls at the boundary between BlueSky, communications, vehicles and external services.

## Required domains
- operator identity and authentication;
- authorization and role enforcement;
- vehicle identity;
- C2 message authenticity/integrity;
- key/certificate lifecycle;
- secure external-service authentication;
- audit trail;
- configuration and software provenance.

## Principle
Security controls MUST be separable from mission logic and MUST produce explicit states and events that can be recorded in the Flight Record.

## Minimum operational states
AUTHENTICATED, UNAUTHENTICATED, KEY_INVALID, LINK_UNTRUSTED, SERVICE_UNAVAILABLE, SECURITY_EVENT.
