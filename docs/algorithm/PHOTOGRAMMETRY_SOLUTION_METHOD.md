# BlueSky PRO — Photogrammetry / 3D Mapping Solution Method

**Status:** DESIGN BASELINE  
**Date:** 2026-10-04  
**Purpose:** Define the equipment, camera, acquisition, flight, illumination, positioning and quality parameters that the mission planner must consider for successful photogrammetric data acquisition.

## 1. Principle

Photogrammetry planning is an end-to-end data-acquisition problem. The planner MUST optimize for the quality and completeness of the final product, not merely for a flyable route.

The mission profile defines the required product, target GSD, accuracy, coverage, time window and available equipment. The planner then derives camera configuration, surface-following altitude, flight-line geometry, image triggering, illumination constraints, positioning/control strategy and quality gates.

## 2. Equipment model

The UAV equipment profile MUST expose, where applicable:

- camera/sensor type;
- sensor dimensions and pixel dimensions;
- focal length / 35 mm equivalent;
- fixed or variable focal length;
- lens distortion/calibration model;
- rolling-shutter or global/mechanical-shutter characteristics;
- shutter capability;
- aperture range;
- ISO range;
- focus mode;
- image format and resolution;
- image capture interval;
- gimbal capability and limits;
- camera orientation;
- RTK capability;
- PPK capability;
- GNSS/INS quality and event/geotag timing;
- battery/endurance characteristics;
- maximum safe mapping speed;
- terrain-follow capability;
- obstacle sensing/clearance capability;
- payload mass and UAV performance limits;
- storage capacity and expected data volume.

Equipment capability MUST constrain the mission planner. The planner MUST NOT select a required acquisition configuration that the installed payload cannot deliver.

## 3. Camera configuration

### 3.1 Lens

Prefer a stable fixed focal length for mapping when compatible with the mission. Variable zoom MUST be treated as a controlled parameter because changing focal length changes the camera geometry and acquisition footprint. Pix4D recommends fixed focal length for stable mapping and notes that longer focal length can improve spatial resolution at a given altitude while requiring a higher image rate to preserve overlap.

### 3.2 Focus

For aerial mapping, use locked/manual focus at infinity when supported by the camera and validated for the payload. Do not allow autofocus changes during a mapping block.

### 3.3 Stabilization

Electronic/mechanical image stabilization used by the camera itself SHOULD be disabled when it interferes with photogrammetric camera modelling. The gimbal may still stabilize the payload orientation.

### 3.4 Shutter

The planner MUST maintain a shutter speed sufficient to prevent motion blur at the planned ground speed, altitude and focal length.

As an initial photogrammetry rule, Pix4D gives 1/300–1/800 s as an indicative range and recommends increasing shutter speed if directional blur becomes significant. DJI mapping guidance may require faster settings; therefore the final value MUST be derived from the specific camera, speed and lighting rather than hard-coded globally.

### 3.5 ISO

Prefer the lowest ISO that provides adequate exposure. High ISO increases noise and can reduce reconstruction quality.

### 3.6 Aperture

Aperture MUST be selected to maintain sufficient sharpness/depth of field while avoiding under/overexposure. Automatic aperture can be acceptable when the exposure strategy is controlled; fully manual exposure may be preferable where lighting is stable and repeatability is required. The mission template MUST permit both modes.

### 3.7 Exposure

The acquisition controller MUST monitor exposure quality and reject/flag overexposed or underexposed images. Exposure should remain consistent throughout a mapping block where possible.

### 3.8 Image format

The payload profile MUST define the supported format and preserve the highest-quality source imagery appropriate to the processing chain. Still imagery is preferred over video for accurate mapping.

## 4. Rolling shutter / camera calibration

The equipment model MUST record whether the sensor uses rolling shutter.

For rolling-shutter cameras, the planning and processing chain MUST preserve the camera timing and motion information needed for rolling-shutter compensation. Rolling shutter can introduce geometric distortion because different image rows are exposed at different times while the UAV is moving.

Camera calibration metadata MUST include, where available:

- focal length;
- principal point;
- radial distortion;
- tangential distortion;
- sensor dimensions/pixel size;
- rolling-shutter model/readout characteristics.

The calibration model MUST be tied to the actual camera/lens configuration used for the mission.

## 5. GSD and surface distance

GSD is a mission requirement, not merely an output statistic.

The planner MUST derive the permissible camera-to-surface distance from:

- target GSD;
- sensor dimensions/resolution;
- focal length;
- camera orientation;
- surface geometry.

GSD depends on distance to the terrain/object and camera parameters.

## 6. Surface-relative flight

For inclined or irregular surfaces:

- generate or use a terrain/3D surface model;
- calculate local surface elevation;
- calculate surface normal/slope/aspect;
- maintain the required camera-to-surface distance;
- maintain safety clearance from terrain;
- adapt speed and flight-line spacing where geometry changes.

The planner MUST distinguish:

ALTITUDE_ABOVE_REFERENCE

from:

DISTANCE_TO_SURVEY_SURFACE.

The second is the controlling variable for photogrammetric quality.

## 7. Flight-line generation

The planner MUST generate candidate flight-line orientations from surface geometry rather than always applying a fixed north/south grid.

Candidate orientation scoring SHOULD include:

- surface principal direction / longest dimension;
- coverage efficiency;
- number and length of flight lines;
- number of turns;
- turn geometry;
- GSD consistency;
- overlap consistency;
- illumination;
- wind;
- energy;
- total mission time;
- safety clearance.

The longest surface direction is a valid candidate heuristic, but MUST NOT be an unconditional rule.

## 8. Image overlap and trigger rate

The planner MUST calculate image spacing from:

- desired front overlap;
- desired side overlap;
- camera footprint;
- GSD;
- UAV speed;
- sensor orientation.

General photogrammetry guidance commonly starts at approximately 75% front and 60% side overlap; DJI mapping guidance commonly uses 80% front and 70% side. More difficult terrain and 3D reconstruction can require higher overlap.

The trigger interval MUST be calculated rather than entered as an unrelated fixed number. Pix4D provides the relationship between image footprint, overlap and UAV speed for this calculation.

## 9. 3D / oblique acquisition

A nadir grid alone MUST NOT be assumed sufficient for complex 3D objects.

Where the product requires surfaces/facades/steep slopes:

- generate oblique viewpoints;
- control camera pitch/roll/heading;
- maintain appropriate distance to the surface;
- ensure sufficient overlap between viewpoints;
- ensure visibility of required surfaces;
- preserve geometric diversity/parallax.

For building reconstruction, Pix4D describes circular/oblique acquisition as a separate strategy from a standard nadir grid.

## 10. Illumination and sun geometry

The mission planner MUST calculate solar position for the mission time window:

- solar azimuth;
- solar elevation;
- expected shadow direction;
- expected shadow length;
- surface aspect relative to the Sun;
- changing illumination during the mission.

The planner MUST identify illumination conditions likely to produce unusable or inconsistent imagery.

Preferred conditions are stable, diffuse illumination. Direct harsh sunlight, glare and strong shadows can hide features and reduce matching quality.

As a conservative planning parameter, optical survey missions SHOULD flag solar elevation below 30° for review; this is a planning guideline, not a universal physical threshold.

For long or multi-UAV missions, the system MUST evaluate illumination across the entire acquisition window, not only at mission start.

## 11. Weather

Before execution, evaluate:

- wind speed;
- wind direction;
- gusts;
- turbulence;
- precipitation;
- visibility/atmospheric clarity;
- temperature;
- expected lighting stability.

Strong wind can reduce image sharpness through UAV motion; rain can contaminate the lens and change surface appearance.

## 12. Positioning and georeferencing

The mission profile MUST select an accuracy strategy:

- standard GNSS;
- RTK;
- PPK;
- RTK/PPK + GCP;
- GCP + independent checkpoints;
- hybrid control.

RTK/PPK improves direct georeferencing, while GCPs and independent checkpoints provide additional control and validation.

The planner MUST store the coordinate reference system, geoid/vertical reference and all positioning metadata required by the processing chain.

## 13. Multi-UAV acquisition

For a shared 3D mapping task:

GLOBAL OBJECT / AREA → SURFACE SEGMENTATION → SUBTASKS → UAV ASSIGNMENT → LOCAL ACQUISITION PLANS → SYNCHRONIZATION → DATA FUSION → COMPLETENESS / QUALITY CHECK

Allocation MUST consider:

- surface area;
- surface complexity;
- acquisition time;
- UAV endurance;
- payload capability;
- camera configuration;
- accessibility;
- expected image count/data volume;
- wind exposure;
- illumination;
- overlap requirements;
- synchronization;
- communication constraints;
- return/reserve requirements.

The optimization objective SHOULD primarily minimize mission makespan while preserving complete coverage and required product quality.

## 14. Data and storage

The mission plan MUST estimate:

- expected image count;
- image size;
- total payload data volume;
- storage margin;
- transfer time;
- required metadata;
- per-UAV dataset identity.

Each image MUST remain associated with its acquisition timestamp, position/orientation metadata and camera configuration.

## 15. Preflight quality gates

Before launch, verify:

- camera detected;
- correct camera/lens profile selected;
- focus locked;
- exposure strategy valid;
- shutter capability sufficient for planned speed;
- ISO/aperture within configured limits;
- stabilization configuration valid;
- storage available;
- battery/endurance sufficient;
- RTK/PPK status valid if required;
- camera time/GNSS time synchronized;
- terrain model available where terrain-following is required;
- GSD achievable;
- overlap achievable;
- illumination window acceptable;
- weather acceptable;
- safety clearance achievable.

If a required condition cannot be met, the mission MUST be marked NOT READY or require explicit operator resolution.

## 16. In-flight quality monitoring

The system SHOULD monitor:

- actual GSD proxy / surface distance;
- exposure;
- image sharpness;
- blur;
- overlap;
- camera triggering;
- GNSS/RTK/PPK status;
- UAV speed;
- altitude/surface distance;
- gimbal/camera orientation;
- storage;
- battery;
- wind deviation;
- coverage progress.

If quality falls below configured thresholds, the system MUST either adapt the mission when safe or create a re-acquisition task.

## 17. Post-flight validation

Mission completion MUST require more than successful landing.

Validate:

1. image count vs planned count;
2. missing/duplicate images;
3. geotag integrity;
4. camera calibration consistency;
5. coverage completeness;
6. overlap sufficiency;
7. image sharpness/exposure;
8. reconstruction alignment;
9. GSD;
10. RTK/PPK quality;
11. GCP/checkpoint residuals where applicable;
12. holes/uncovered surfaces;
13. final product coordinate reference;
14. multi-UAV dataset registration/fusion;
15. final product quality.

The result is MISSION COMPLETE only when the required product quality gates pass.

## 18. Optimization variables

The Optimization Layer MAY optimize:

- flight-line orientation;
- line spacing;
- surface-following altitude;
- speed;
- trigger interval;
- front/side overlap;
- oblique viewpoint selection;
- UAV-to-surface assignment;
- UAV task allocation;
- acquisition time;
- mission sequencing;
- energy/time trade-offs.

Hard safety and equipment constraints remain authoritative.

## 19. Important distinction

These values are starting recommendations, not universal constants:

- front overlap;
- side overlap;
- shutter speed;
- ISO;
- aperture;
- solar elevation;
- GSD;
- flight speed;
- camera angle.

BlueSky PRO MUST derive or validate them against the selected camera, UAV, terrain, required product accuracy, lighting and mission objective.

## 20. Final solution principle

USER REQUIREMENTS
→ PRODUCT REQUIREMENTS
→ EQUIPMENT CONFIGURATION
→ SURFACE MODEL
→ GSD / CAMERA-SURFACE DISTANCE
→ ILLUMINATION
→ FLIGHT-LINE / VIEWPOINT GENERATION
→ OVERLAP / TRIGGER
→ MULTI-UAV ALLOCATION
→ ENERGY / TIME
→ OPTIMIZATION
→ DATA FUSION
→ QUALITY VALIDATION
→ FINAL PRODUCT
