# BlueSky PRO — Software Configuration Baseline

**Status:** IMPLEMENTATION BASELINE — P0
**Domain:** EC-18 — Software Configuration / Version Control

## 1. Purpose

This document defines the controlled software configuration identity used by the BlueSky PRO verification evidence chain.

A verification result shall be traceable to the exact source revision and build configuration used to produce it.

## 2. Controlled configuration identity

| Field | Controlled value |
|---|---|
| Product | BlueSky PRO |
| Repository | `ss1736427-source/BlueSky-PRO-Knowledge` |
| Controlled branch | `main` |
| Baseline revision | `65a1567b` |
| Software release state | `UNRELEASED-P0` |
| Build system | CMake |
| C++ standard | C++20 |
| Primary build configuration | `Release` |
| Verification domain | `EC-18` |

The baseline revision identifies the repository state from which the current P0 configuration is derived. A later controlled revision supersedes this baseline only through an explicit configuration update.

## 3. Configuration record

The machine-readable baseline is maintained in:

`software_configuration_baseline.json`

The record is intended to be referenced by verification execution records and evidence packages rather than copied manually into individual test results.

## 4. Required configuration controls

For each controlled verification execution, the configuration record shall identify, as applicable:

- repository;
- branch/tag or equivalent controlled reference;
- exact source revision;
- software release/build state;
- build system and relevant build configuration;
- compiler/toolchain identity when available;
- build artifacts or artifact references;
- configuration-specific parameters;
- applicable hardware/equipment configuration;
- configuration baseline identifier.

## 5. Change control

A source or build change that can affect verification behaviour shall create a new controlled configuration identity or update the applicable controlled baseline through the project version-control history.

The previous configuration identity shall remain traceable. Historical evidence shall not be silently reassigned to a later configuration.

## 6. Evidence linkage

The EC-18 configuration identity shall be available to the common evidence lifecycle:

`Requirement → Test Method → Test Case → Configuration → Execution → Source Data → Raw Data → Processed Data → Result → Evidence Package → Certification Reference`

The configuration identifier is therefore a required joining key between software revision control and verification evidence.

## 7. Current implementation boundary

This baseline establishes the controlled configuration identity and machine-readable record for the P0 prototype.

It does not by itself constitute physical verification evidence, nor does it prove that every future build artifact has been independently reproduced and verified. Those controls remain part of the subsequent verification execution and release process.
