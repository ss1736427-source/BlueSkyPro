# MT-01 — camera capture quality and in-motion exposure model

## Operating principle

MT-01 photogrammetry captures images while the aircraft follows the coverage leg. A camera exposure is an observation with a finite shutter-open interval; it is not a waypoint dwell, hover, or task-duration pause. Do not add one waypoint duration for every photograph.

The coverage planner places exposure stations by ground distance. The nominal interval is:

`trigger_interval_s = forward_trigger_spacing_m / planned_ground_speed_mps`

Along-track image spacing is derived from image footprint and required forward overlap. Side overlap determines line spacing. The wind-corrected trajectory must re-evaluate trigger interval and motion blur using actual segment ground speed.

## Image quality constraints

For ground sampling distance `GSD_m_px`, ground speed `Vg_mps`, and selected exposure time `t_exposure_s`:

`motion_blur_px = Vg_mps * t_exposure_s / GSD_m_px`

The planned ground speed must satisfy both:
- `trigger_interval_s >= camera.minimum_trigger_interval_s`;
- `motion_blur_px <= camera.maximum_motion_blur_px`.

The planner derives the maximum camera ground speed as the stricter of the trigger-rate and motion-blur limits. The selected exposure time and blur tolerance are configuration inputs; they must not be guessed from the camera's maximum shutter capability.

## DJI Mavic 3E manufacturer reference

Official DJI specifications for the wide camera list:
- 4/3 CMOS, 20 MP;
- 5280 × 3956 maximum image size;
- mechanical shutter range up to 1/2000 s;
- electronic shutter range up to 1/8000 s;
- timed JPEG capture intervals including 0.7 s.

Sources:
- https://enterprise.dji.com/mavic-3-enterprise/specs
- https://enterprise.dji.com/mavic-3-enterprise

The 0.7 s figure is the camera's supported minimum timed-capture interval, not a recommended project trigger interval. Actual trigger interval is determined by GSD, footprint, overlap and ground speed. Actual exposure time must be selected for illumination and motion conditions.

## Mission-type overlap defaults

PIX4D acquisition guidance gives typical starting points, not universal guarantees:
- general mapping: at least 75% forward / 60% side;
- forest and dense vegetation: at least 85% / 85%;
- homogeneous agricultural fields: at least 80% / 80%;
- corridor mapping: at least 85% forward / 60% side;
- urban 3D reconstruction: about 85% forward / 70% side, with oblique imagery and a double grid where facade reconstruction is required.

Source: https://support.pix4d.com/hc/best-practices-for-image-acquisition-and-photogrammetry

## Energy accounting

Camera and payload electrical load is continuous while powered, not a per-image waypoint dwell. `TrajectoryPerformanceInput.payload_power_w` adds:

`payload_energy_wh = payload_power_w * airborne_duration_s / 3600`

This is separate from propulsion energy and is included in total energy and reserve assessment. A verified payload power value must come from the assigned equipment profile or measured data. Zero means no payload-power contribution was configured; it does not prove that the camera consumes no energy.

## Current implementation boundary

- The 2D coverage planner computes capture spacing from image footprint and forward overlap.
- It checks the configured camera trigger interval and motion-blur tolerance at nominal ground speed.
- The wind trajectory re-evaluates trigger interval and motion blur on each explicitly identified coverage segment using wind-corrected ground speed. Connector, turn and transit segments are not treated as image-capture legs.
- Camera validation is fail-closed: configuration identity, GSD, trigger spacing, minimum interval, exposure, blur tolerance and coverage-segment identities must be supplied together. A trigger-rate or blur violation makes the trajectory infeasible.
- The wind trajectory adds continuous payload power over each flown segment.
- MT-01 rejects the legacy per-capture waypoint-dwell representation. Camera captures must not be represented as `task_execution_duration_s` / `task_execution_energy_wh`.
- Wind-corrected trigger timing and blur still require a trajectory-stage check using actual segment ground speed. The current static per-segment wind model does not yet provide time-indexed forecast interpolation.
- Camera latency, rolling-shutter correction, terrain-following GSD variation, image geotag/RTK quality, calibration evidence, image sharpness checks and post-flight coverage validation remain separate readiness requirements.
