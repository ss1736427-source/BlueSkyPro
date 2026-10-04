# BlueSky PRO — Qt 6 / QML HMI Bootstrap

This directory is the first executable HMI implementation path for BlueSky PRO.

## Scope

- Qt 6 + QML / Qt Quick frontend.
- 1920x1080 PC cockpit baseline.
- Header / Flight Chart / Left Mission Panel / Right Operational Panel / UAV strip.
- Semantic HMI palette derived from the current BlueSky PRO baseline.
- No mission, safety, or UAV execution authority is implemented here.

## Architecture boundary

QML is presentation and controlled interaction only.

Critical execution path remains:

QML -> Core -> Safety / Validation -> Adapter

QML must not independently authorize or execute safety-critical UAV actions.

## Open in Qt Design Studio

Open the CMakeLists.txt as the project entry point. The QML files under qml/ are intended to be the editable visual layer.

## Next controlled steps

1. Verify the project opens and builds in the selected Qt 6 installation.
2. Replace placeholder Flight Chart with the map adapter surface.
3. Split the monolithic screen into independent HMI components.
4. Add controlled state models for Mission / Readiness / UAV.
5. Connect QML only to approved Core interfaces.
6. Add HMI verification and traceability records before treating the screen as implementation-closed.
