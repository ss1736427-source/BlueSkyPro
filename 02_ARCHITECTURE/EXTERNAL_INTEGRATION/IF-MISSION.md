# IF-MISSION — BlueSky Mission Interface

**Status:** STRUCTURAL DRAFT  
**Purpose:** Define the boundary between the internal BlueSky Mission Model and an external flight controller/autopilot mission representation.

## 1. Principle

BlueSky's internal mission model is authoritative for planning and coordination. An external autopilot mission is an execution representation. They MUST NOT be treated as the same data model.

## 2. Flow

```text
BlueSky Mission Model
        |
        v
Mission Validation
        |
        v
Vehicle Capability Check
        |
        v
Autopilot Mission Translator
        |
        v
External Mission Representation
        |
        v
Mission Transfer Adapter
        |
        v
Autopilot
```

## 3. Required operations

- create execution mission;
- translate supported mission items;
- upload mission;
- download mission;
- verify uploaded mission;
- compare planned vs onboard mission;
- acknowledge transfer result;
- detect unsupported commands/items;
- handle partial transfer/failure;
- version and identify the mission instance.

## 4. Safety boundary

BlueSky MUST validate the mission against the selected vehicle capabilities and execution constraints before transfer. Unsupported or ambiguous mission semantics MUST NOT be silently converted.

## 5. Initial protocol target

MAVLink Mission Protocol is the initial interoperability target for ArduPilot/PX4-class flight controllers. Protocol-specific details belong in `IF-MAVLINK.md` and implementation-level ICDs.

## 6. Open items

- Exact supported MAVLink mission-item subset.
- Mapping of BlueSky Mission Graph actions to autopilot mission commands.
- Handling of commands that require companion-computer or payload execution.
- Atomicity/rollback policy for failed uploads.
- Mission integrity/signature requirements.
- Multi-UAV mission-instance identification and synchronization.
