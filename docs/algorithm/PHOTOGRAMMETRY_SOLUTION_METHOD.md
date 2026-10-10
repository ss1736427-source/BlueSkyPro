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

Prefer a stable fixed focal length for mapping when compatible with the camera. Variable zoom MUST be treated as a controlled parameter because changing focal length changes camera geometry and acquisition footprint.

### 3.2 Focus

For aerial mapping, use locked/manual focus at infinity when supported and validated for the payload. Do not allow autofocus changes during a mapping block.

### 3.3 Stabilization

Electronic/mechanical image stabilization used by the camera itself SHOULD be disabled when it interferes with photogrammetric camera modelling. The gimbal may still stabilize payload orientation.

### 3.4 Shutter

The planner MUST maintain a shutter speed sufficient to prevent motion blur at the planned ground speed, altitude and focal length.

As an initial photogrammetry rule, 1/300–1/800 s may be used as an indicative range; the final value MUST be derived from the specific camera, speed and lighting rather than hard-coded globally.

### 3.5 ISO

Prefer the lowest ISO that provides adequate exposure. High ISO increases noise and can reduce reconstruction quality.

### 3.6 Aperture

Aperture MUST be selected to maintain sufficient sharpness/depth of field while avoiding under/overexposure. Automatic aperture can be acceptable when the exposure strategy is controlled; fully manual exposure may be preferable where lighting is stable and repeatability is required.

### 3.7 Exposure

The acquisition controller MUST monitor exposure quality and reject/flag overexposed or underexposed images. Exposure should remain consistent throughout a mapping block where possible.

### 3.8 Image format

The payload profile MUST define the supported format and preserve the highest-quality source imagery appropriate to the processing chain. Still imagery is preferred over video for accurate mapping.

## 4. Rolling shutter / camera calibration

The equipment model MUST record whether the sensor uses rolling shutter.

For rolling-shutter cameras, the planning and processing chain MUST preserve the camera timing and motion information needed for rolling-shutter compensation.

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

The planner MUST derive permissible camera-to-surface distance from:

- target GSD;
- sensor dimensions/resolution;
- focal length;
- camera orientation;
- surface geometry.

GSD depends on distance to terrain/object and camera parameters.

## 6. Surface-relative flight

For inclined or irregular surfaces:

- generate or use a terrain/3D surface model;
- calculate local surface elevation;
- calculate surface normal/slope/aspect;
- maintain required camera-to-surface distance;
- maintain safety clearance from terrain;
- adapt speed and flight-line spacing where geometry changes.

The planner MUST distinguish ALTITUDE_ABOVE_REFERENCE from DISTANCE_TO_SURVEY_SURFACE.

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

## 8. Image overlap / заступ

Image overlap is a core planning parameter because common features must appear in multiple images and, for a multi-flight or multi-UAV project, in adjacent datasets as well.

BlueSky PRO MUST distinguish at least four overlap types:

1. Forward overlap (frontlap) — overlap of consecutive images along one flight line.
2. Side overlap (sidelap) — overlap between adjacent parallel flight lines.
3. Block/strip overlap — overlap between neighboring survey blocks, strips, UAV sectors or repeated flights.
4. Surface/3D overlap — overlap between observations of the same physical surface from different viewpoints.

### 8.1 Baseline values

For ordinary nadir mapping, use as a planning baseline:

- front overlap: ≥75%;
- side overlap: ≥60%.

Pix4D explicitly recommends these minimums for general cases. citeturn0search1turn0search4

For more demanding acquisition, use higher starting targets:

- general high-quality mapping: 80–85% front / 70% side;
- complex terrain, structures or 3D reconstruction: 80–85% front / 70–80% side;
- difficult low-texture vegetation/snow/sand: ≥85% front / ≥70% side;
- vertical/tall structures: approximately 90% same-level overlap and 60% overlap between levels where that acquisition geometry is used. citeturn0search0turn0search7

These are planning baselines, not universal constants.

### 8.2 Why the заступ must be calculated

The system MUST NOT store “80% overlap” as an isolated parameter.

Given camera footprint dimensions L and W on the surface:

image_step_along = L × (1 - front_overlap)

flight_line_spacing = W × (1 - side_overlap)

For example, at 80% front overlap the UAV advances approximately 20% of the along-track image footprint between exposures. At 70% side overlap adjacent tracks are separated by approximately 30% of the cross-track footprint.

The actual step MUST be recalculated when GSD, surface distance, focal length, camera orientation or surface geometry changes.

### 8.3 Coverage margin

The planner MUST add a coverage margin / perimeter overrun so that the required survey boundary is fully covered by the usable image footprint.

The margin MUST be calculated from camera footprint and acquisition geometry, not as an arbitrary fixed number.

The system MUST distinguish TASK_BOUNDARY from IMAGE_COVERAGE_BOUNDARY.

The latter must extend beyond the task boundary enough that the required edge area is reconstructed without holes.

### 8.4 Edge and corner coverage

The first and last flight lines and the first/last exposures MUST be checked for:

- complete coverage of the required area;
- sufficient overlap with neighboring line/image;
- sufficient observations for bundle adjustment;
- no unobserved corners;
- no loss of coverage caused by turns or acceleration/deceleration.

The planner SHOULD extend acquisition lines beyond the nominal polygon where necessary, while keeping the UAV itself within all applicable safety and airspace constraints.

### 8.5 Multi-UAV and multi-block stitching

For UAV-1 / UAV-2 / UAV-3 working on one common object, partitions MUST NOT be treated as independent islands.

Adjacent sectors MUST have a deliberate stitching overlap zone containing common surface observations.

The overlap between neighboring sectors MUST be large enough to provide common tie features and geometric connection. The exact width MUST be derived from camera footprint, GSD, scene texture, viewing geometry and the processing method; it MUST NOT be hard-coded as one universal percentage.

Where datasets have different acquisition geometries or are captured in separate sessions, BlueSky PRO SHOULD require additional common observations and, where appropriate, GCPs or manual tie points. Pix4D recommends sufficient overlap within and between datasets and recommends GCPs/manual tie points when combining different capture methods. citeturn0search0turn0search4

### 8.6 3D surface overlap

For steep slopes, cliffs, buildings and complex objects, a point should ideally be observed from multiple camera positions and, where required, multiple directions.

Therefore the planner MUST evaluate:

- number of observations per surface patch;
- viewing-angle diversity;
- parallax;
- visibility;
- distance to surface;
- overlap between oblique and nadir datasets.

A nadir grid alone MUST NOT be considered sufficient for a complex 3D object.

### 8.7 Overlap quality gate

Before mission release, the planner MUST simulate image footprints and verify:

COVERAGE >= REQUIRED_COVERAGE

and

OVERLAP >= REQUIRED_OVERLAP

for all required surface patches.

After acquisition, the system MUST verify actual image coverage and identify:

- holes;
- weakly observed areas;
- missing image sequences;
- insufficient inter-strip overlap;
- insufficient inter-UAV overlap;
- insufficient viewpoint diversity.

If a required area fails the quality gate, the system creates a re-acquisition task instead of declaring the mission complete.

## 9. Image trigger rate

The planner MUST calculate image spacing from:

- desired front overlap;
- side overlap;
- camera footprint;
- GSD;
- UAV speed;
- sensor orientation.

The trigger interval MUST be calculated rather than entered as an unrelated fixed number.

## 10. 3D / oblique acquisition

A nadir grid alone MUST NOT be assumed sufficient for complex 3D objects.

Where the product requires surfaces/facades/steep slopes:

- generate oblique viewpoints;
- control camera pitch/roll/heading;
- maintain appropriate distance to the surface;
- ensure sufficient overlap between viewpoints;
- ensure visibility of required surfaces;
- preserve geometric diversity/parallax.

Pix4D recommends oblique/circular acquisition for building and vertical-object reconstruction, with high overlap for difficult 3D geometry. citeturn0search0turn0search4

## 11. Illumination and sun geometry

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

## 12. Weather

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

## 13. Positioning and georeferencing

The mission profile MUST select an accuracy strategy:

- standard GNSS;
- RTK;
- PPK;
- RTK/PPK + GCP;
- GCP + independent checkpoints;
- hybrid control.

RTK/PPK improves direct georeferencing, while GCPs and independent checkpoints provide additional control and validation.

The planner MUST store the coordinate reference system, geoid/vertical reference and all positioning metadata required by the processing chain.

## 14. Multi-UAV acquisition

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

## 15. Data and storage

The mission plan MUST estimate:

- expected image count;
- image size;
- total payload data volume;
- storage margin;
- transfer time;
- required metadata;
- per-UAV dataset identity.

Each image MUST remain associated with acquisition timestamp, position/orientation metadata and camera configuration.

## 16. Preflight quality gates

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

## 17. In-flight quality monitoring

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

## 18. Post-flight validation

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

## 19. Optimization variables

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

## 20. Important distinction

These values are starting recommendations, not universal constants:

- front overlap;
- side overlap;
- inter-block/inter-UAV stitching overlap;
- shutter speed;
- ISO;
- aperture;
- solar elevation;
- GSD;
- flight speed;
- camera angle.

BlueSky PRO MUST derive or validate them against the selected camera, UAV, terrain, required product accuracy, lighting and mission objective.

## 21. Special-case acquisition classes

The planner MUST NOT treat every imaging mission as the same generic grid.

External photogrammetry guidance identifies materially different acquisition strategies for:

- general terrain;
- dense vegetation/forest;
- homogeneous agricultural fields;
- buildings;
- city/facade reconstruction;
- large vertical objects;
- corridors such as roads, railways and rivers;
- snow and sand;
- water;
- multiple flights;
- mixed aerial/terrestrial or nadir/oblique datasets;
- multispectral surveys. citeturn0search1turn0search4

### 21.1 Dense vegetation / forest

Use higher overlap, typically at least 85% in both directions, and consider higher altitude to reduce perspective variation. Avoid periods when vegetation is moving strongly between exposures. citeturn0search4

### 21.2 Homogeneous agriculture

Increase overlap to at least 80% in both directions, maintain accurate image geolocation and consider higher altitude while preserving required GSD. citeturn0search4

### 21.3 Buildings / urban facades

Use dedicated orbit/double-grid/oblique acquisition rather than a single nadir grid. Multiple heights and camera attitudes may be required. For city reconstruction, facade visibility must be explicitly planned. citeturn0search1

### 21.4 Corridors

Treat corridor mapping as a separate geometry. External guidance recommends at least two flight lines and preferably three; approximately 85% front and 60% side overlap is recommended for a two-line corridor case. citeturn0search1

### 21.5 Snow / sand

These surfaces have weak visual texture. Increase overlap, control exposure for maximum useful contrast, and expect higher reconstruction difficulty. citeturn0search4

### 21.6 Water

Open water is generally unsuitable for ordinary image-based reconstruction because it lacks stable visual features and is reflective/dynamic. Where water must be included, the planner should ensure that identifiable land/shore features occupy a substantial part of the images and treat the water surface as a special data-quality class. citeturn0search4

### 21.7 Multiple flights / datasets

If a project spans multiple flights, datasets MUST have compatible acquisition geometry and sufficient overlap. Differences in sun, weather, scene state, GSD or acquisition method can weaken matching. If datasets differ materially, require additional common observations, GCPs or manual tie points. citeturn0search1turn0search10

For terrain with large elevation differences, when multiple constant-altitude flights are used, adjacent flights should overlap and their GSDs in the overlap should remain within approximately a factor of two. citeturn0search0turn0search12

### 21.8 Multispectral

Multispectral missions require a separate radiometric acquisition workflow, not just a modified RGB grid.

The planner MUST support, where applicable:

- calibrated reflectance panel;
- downwelling light/sun sensor;
- band-specific calibration metadata;
- exposure/irradiance metadata;
- radiometric correction;
- consistent illumination;
- spectral-band registration;
- native resolution/aspect ratio;
- flight overlap suitable for the sensor.

Current MicaSense/Pix4D guidance calls for panel captures immediately before and after each flight, unobstructed light-sensor view, and consistent illumination. Pix4D also recommends at least 75% front and side overlap for multispectral capture. citeturn1search0turn1search4

### 21.9 Low-altitude mapping

Very low flight can create processing problems even when nominal GSD is excellent. For such missions the planner should treat low altitude as a special risk class and, where appropriate, add a second acquisition layer at a higher altitude to strengthen matching. MicaSense currently recommends this approach for low-altitude multispectral acquisition. citeturn1search10

## 22. Field quality assurance

The mission is NOT considered successful solely because all waypoints were flown.

The field workflow SHOULD include:

1. immediate image count/storage check;
2. representative sharpness and exposure check;
3. coverage check;
4. geolocation/RTK status check;
5. rapid/low-resolution reconstruction where available;
6. detection of holes or weakly connected blocks;
7. decision whether to re-fly before leaving the site.

Pix4D explicitly notes that inadequate acquisition can require reacquisition and that rapid/low-resolution processing can be used as a field indicator, although a rapid failure does not necessarily prove that full processing will fail. citeturn0search2

## 23. Control-point design

When GCPs are used, the planner MUST treat their placement as part of mission design.

GCPs SHOULD be distributed across the area rather than clustered, placed near important objects/areas, and kept sufficiently inside the project boundary. Independent checkpoints SHOULD be reserved for validation rather than used as control. citeturn0search1turn0search10

The system should also calculate whether a planned GCP is large enough to be reliably identifiable at the target GSD.

## 24. Environmental and operational state

The mission record MUST retain:

- weather state;
- illumination state;
- wind;
- sensor configuration;
- acquisition time;
- GNSS/RTK/PPK state;
- camera/lens identity;
- calibration identity;
- processing-relevant metadata.

This is required for traceability and for deciding whether two datasets are legitimately combinable.

## 25. Final solution principle

USER REQUIREMENTS
→ PRODUCT REQUIREMENTS
→ EQUIPMENT CONFIGURATION
→ SURFACE MODEL
→ TASK-SPECIFIC ACQUISITION CLASS
→ GSD / CAMERA-SURFACE DISTANCE
→ ILLUMINATION / WEATHER
→ FLIGHT-LINE / VIEWPOINT GENERATION
→ OVERLAP / TRIGGER
→ MULTI-UAV ALLOCATION
→ CONTROL / GEOREFERENCING
→ FIELD QA
→ ENERGY / TIME
→ OPTIMIZATION
→ DATA FUSION
→ COMPLETENESS / QUALITY VALIDATION
→ FINAL PRODUCT
