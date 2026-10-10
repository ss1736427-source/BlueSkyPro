# EC-06 Equipment Adapter

Status: IMPLEMENTATION BASELINE

The Equipment Adapter baseline provides a controlled contract/prototype for equipment identity, profile and configuration references, capabilities, state, telemetry, actions, and data outputs.

Implemented:
- concrete `EquipmentAdapterBaseline`;
- capability availability handling;
- state and telemetry readback;
- action translation with explicit error codes;
- data-output declaration;
- automated CTest coverage.

This baseline is not physical equipment verification and does not claim certification evidence. Physical equipment integration, approved source adapters, controlled execution evidence, evidence-chain linkage, integrity-controlled export, and operational verification remain subsequent stages.
