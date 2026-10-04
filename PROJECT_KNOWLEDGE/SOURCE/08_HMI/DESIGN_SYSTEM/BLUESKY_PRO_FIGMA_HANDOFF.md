# BlueSky PRO — Figma Handoff

## Purpose

Use Figma as the ergonomic/component design environment for the supplied BlueSky PRO HMI. Figma is not the source of the visual identity; the supplied sketches, assets, colors and typography are the baseline.

## Componentization

Build independently reusable components for:

- Header telemetry strip
- Mission Templates
- Flight Chart
- Mission Analysis
- Warnings / Corrections
- ATC / communications
- UAV Telemetry
- Mission Actions
- Map controls
- Progress indicators
- Buttons
- Inputs
- Select/Search
- Checkbox/Radio/Toggle
- UAV table
- Bottom navigation

## Layout behavior

Panels must be designed as independent components so their position and size can be changed without rebuilding the whole screen.

Required states include normal, selected/hover, disabled, warning, critical and success where applicable.

## HMI direction

The objective is a full working operational view: reduce clutter, hide secondary information when it is not needed, add missing operational panels, and establish a clear information hierarchy.

The map remains the primary workspace. Route, waypoint and aircraft information must remain visually dominant over the subdued map background.

## Engineering handoff

The Figma component model must remain transferable to Qt 6 + QML. Avoid design decisions that depend on Figma-only effects or behavior.

## Typography

Use the controlled typography specification in `BLUESKY_PRO_TYPOGRAPHY.md`.

## Colors

Use the controlled palette in `BLUESKY_PRO_COLORS.md`.

## Logo

Use the supplied BlueSky PRO logo assets. Do not redraw or substitute the logo.
