# BlueSky PRO — Top Header Composition Working Variant 001

**Status:** working intermediate variant — NOT FROZEN  
**Scope:** visual composition only; no production geometry is approved by this document.

## 1. Current composition

The Top Header is constructed as three geometric regions:

`LEFT ANCHOR | CENTRAL INFORMATION COMPOSITION | RIGHT ANCHOR`

### Left anchor
**LOGO**

- Anchor field is adaptive to the visual/logo composition.
- The field must not be assigned an arbitrary fixed width at this stage.
- Logo scale, visual width, height, X/Y placement, and internal padding remain adjustable.
- The controlled master SVG remains the source asset; do not redraw the logo.

### Right anchor
**OPERATOR**

- Operator is a dedicated anchor card, not a normal telemetry sector.
- Visual structure:
  - operator icon centered above;
  - `OPERATOR` label centered below;
  - compact bordered/outlined field.
- The outer anchor geometry is symmetric with the LOGO anchor.
- Icon scale, label scale, vertical spacing, and internal padding remain adjustable.

## 2. Central information composition

The central composition contains six sectors:

`ETD | TOT | TRIP | ETA | READY | WARNING`

The geometric center of the Header is between **TRIP** and **ETA**.

The six sectors are distributed symmetrically from the center toward the two anchors.

Left side:
`ETD | TOT | TRIP`

Right side:
`ETA | READY | WARNING`

The widths and spacing are currently adjustable for visual composition work.

## 3. Sector structure

Each central sector has:

- a vertical separator from adjacent sectors;
- an abbreviation/status heading;
- its value directly below;
- horizontal centering of heading and value on the same sector axis.

Time sectors therefore use:

`ETD`  
`10:30`

`TOT`  
`—`

`TRIP`  
`—`

`ETA`  
`11:48`

Values are centered beneath their abbreviations.

## 4. Symmetry rule

The working geometry is:

`LOGO | ETD | TOT | TRIP | CENTER | ETA | READY | WARNING | OPERATOR`

with:

- equal left/right anchor geometry;
- central axis between TRIP and ETA;
- adjustable central sector widths;
- adjustable inter-sector spacing;
- adaptive logo sizing;
- adaptive operator card sizing within the symmetric anchor geometry.

The objective is to tune the composition visually rather than prematurely freeze pixel dimensions.

## 5. Workbench intent

A temporary Header Composition Workbench may be used to tune:

- Header height;
- left/right anchor width;
- logo scale;
- logo visual width/height;
- operator icon scale;
- operator label scale;
- central sector widths;
- inter-sector spacing;
- separator height/width;
- heading size;
- value size;
- vertical spacing;
- central axis.

These parameters are **working composition controls**, not final application settings.

## 6. Freeze rule

Do not treat this variant as the final Header until the full Header has been visually reviewed and the composition has been explicitly frozen.

After freeze:

1. record approved geometry in the Design System;
2. transfer approved values to production QML;
3. remove temporary Workbench controls;
4. retain this document as the intermediate design record.

## 7. Current exclusions

This working variant does not resolve:

- final logo asset;
- final typography;
- final color treatment;
- final responsive breakpoints;
- tablet-specific BAT placement;
- final interaction/state behavior;
- final accessibility/ergonomic validation.

Those remain governed by the existing controlled HMI documentation and later reconciliation.
