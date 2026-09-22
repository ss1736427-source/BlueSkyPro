# BlueSky PRO — Controlled Technical Overview

BlueSky PRO is a desktop-oriented UAS flight planning and mission management system designed around one operational principle: **the map is the primary workspace, while planning, aircraft state, communications, compliance, AI assistance and documentation operate as one coordinated system.**

This review package presents the product concept, system logic and development direction. It intentionally excludes source code, proprietary implementation details, internal coefficients, model parameters, credentials and other material required to reproduce the system.

## Start here

For the technical team, begin with [`00_PROJECT_OVERVIEW.md`](00_PROJECT_OVERVIEW.md). It gives the system-level picture and separates implemented, defined, planned and experimental areas.

Then use [`10_TECHNICAL_REVIEW_GUIDE.md`](10_TECHNICAL_REVIEW_GUIDE.md) for the review sequence and [`11_ACCESS_CONTROL.md`](11_ACCESS_CONTROL.md) for repository-control rules.

## What is being demonstrated

- End-to-end flight planning and mission lifecycle
- Flight Chart as the central operational workspace
- Pilot, technician, administrator and integration roles
- UAV/fleet capability representation
- C2/connectivity and telemetry integration
- Energy-aware route planning
- Weather/wind-aware planning and correction
- Multi-UAV mission coordination
- AI learning from operational corrections
- Regulatory/ATM integration
- Verification, traceability and flight documentation
- A path toward automated aircraft-level risk/insurance assessment

## Product direction

BlueSky PRO is intended to reduce the amount of routine work performed by people while increasing traceability and preserving human control over operational decisions.

Review status: concept and architecture development in progress.
Repository evidence and historical development remain maintained separately from this review branch.
