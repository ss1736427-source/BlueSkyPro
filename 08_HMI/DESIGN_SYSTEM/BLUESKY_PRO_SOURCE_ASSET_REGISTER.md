# BlueSky PRO — Source Asset Register

Status: CURRENT WORKING SET

## User-supplied visual references

### Main HMI
- Current BlueSky PRO Flight Chart / mission-planning screen sketches.
- Multiple supplied logo variants.

### UI component references
- Mission completion/progress indicator.
- Map toolbar and map surface.
- Warnings / Corrections panel.
- Button states.
- UAV Telemetry (Live) card.
- Color palette reference.
- Mission Actions panel.
- Switches / checkboxes / radio controls.
- Input fields / select / search.
- UAV table.

### Existing icon assets
- hub_analytics
- hub_archive
- hub_data
- hub_map
- hub_missions
- hub_monitoring
- hub_security
- pilot_status

## Working rule

All supplied visual references are authoritative source material for reconstruction. Preserve the approved visual language while improving ergonomics and information hierarchy.

Do not silently replace supplied assets with generic UI libraries.

## Binary asset status

The GitHub file-writing connector available in this session can create/update UTF-8 text files, including editable SVG source. Original schematic UAV SVGs have therefore been created and committed as text assets under `qt/BlueSkyPRO-HMI/qml/assets/`. Supplied binary PNG/JPG assets still need to be committed from the local working copy/desktop Git client; their licensing and provenance must be recorded before product distribution.


## BlueSky PRO UAV panel — authored local illustrations

- `qt/BlueSkyPRO-HMI/qml/assets/uav_multirotor.svg` — BS-001, multirotor.
- `qt/BlueSkyPRO-HMI/qml/assets/uav_fixed_wing.svg` — BS-002, fixed-wing.
- `qt/BlueSkyPRO-HMI/qml/assets/uav_heavy_multirotor.svg` — BS-003, heavy multirotor.
- `qt/BlueSkyPRO-HMI/qml/assets/uav_vtol.svg` — BS-004, VTOL.

These are original schematic illustrations with transparent backgrounds, not downloaded product photographs. They are bundled locally for offline preview. Commercial product imagery remains a separate asset/licensing task.
