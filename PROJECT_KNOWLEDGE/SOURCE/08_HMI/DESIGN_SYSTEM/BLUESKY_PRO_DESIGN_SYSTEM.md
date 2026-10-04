# BlueSky PRO — HMI Design System

Status: WORKING BASELINE

## Purpose

This document is the visual source of truth for the BlueSky PRO HMI. The current interface work is performed from the approved BlueSky PRO visual baseline, screen references and logo assets.

## Design principles

- Preserve the BlueSky PRO visual identity.
- Use the project's approved colors and typography rather than substituting generic UI styling.
- Treat panels as independent components so their position, size, visibility and grouping can be changed without redrawing the whole screen.
- Optimize for operator ergonomics and information hierarchy.
- Keep primary flight information visible; move secondary information into expandable or contextual panels.
- The Figma prototype is a component/layout reference for later Qt 6 + QML implementation.
- HMI semantic colors are controlled and must not be replaced by arbitrary UI colors.

## Approved HMI visual baseline

### Background and neutrals

- Main background: `#050A12`
- Panel surface: `#08111D`
- Card surface: `#0C1725`
- Selected / hover surface: `#111F30`
- Primary text: `#FFFFFF`
- Secondary text: `#BFBFBF`
- Muted/service text: `#7F7F7F`

### Semantic colors

- Normal / green zone: `#64FF00`
- Caution / amber zone: `#FFD339`
- Warning / critical: `#FF1E14`
- Navigation / information: `#32FFFF`
- Selected / target / commanded / director: `#FF32FF`

### PFD-only colors

- PFD sky: `#006EAA`
- PFD ground: `#5F2A05`

PFD sky and PFD ground are specialized display colors and are not general-purpose UI colors.

### Color semantics

- WHITE = current / primary neutral data
- GREEN = normal / green operating zone
- AMBER = caution / amber operating zone
- RED = warning / critical / red operating zone
- CYAN = navigation / information
- MAGENTA = selected / target / commanded / director

Green, amber and red may represent numeric operating zones as well as state semantics.

Avoid introducing additional semantic colors when brightness, opacity, outline, thickness or other non-color interaction treatment is sufficient.

### Prohibited / undesirable combinations

The following combinations are not used for normal HMI information because of afterimage, visual fatigue, insufficient contrast or poor separation:

- blue + magenta
- blue + red
- brown + white
- gray + white
- blue + yellow
- brown + red

### Night / NVIS

Night/NVIS colors are a separate theme and are not conflated with the daytime semantic palette:

- NVIS Green A: `#00FF00`
- NVIS Green B: `#00C000`
- Night Red: `#FF0000`

## Typography baseline

### Primary fonts

- Primary UI font: **B612**
- Primary technical/numeric font: **B612 Mono**
- Fallback UI font: **IBM Plex Sans Condensed**

### Approved hierarchy

| Level | Font | Size | Color | Use |
|---|---|---:|---|---|
| 1 | B612 Mono Bold | 36 px | #FFFFFF | Primary flight data |
| 2 | B612 Bold | 22 px | #FFFFFF | Main headers |
| 3 | B612 Mono Bold | 20 px | Green / Amber / Red | Status and numeric operating zones |
| 4 | B612 Mono Bold | 20 px | #32FFFF | Navigation data |
| 5 | B612 Mono Bold | 20 px | #FF32FF | Target / selected / commanded |
| 6 | B612 Bold | 16 px | #FFFFFF | Section headers |
| 7 | B612 | 13 px | #BFBFBF | Labels |
| 8 | B612 Mono | 13 px | #BFBFBF | Secondary numeric data |
| 9 | IBM Plex Sans Condensed | 12 px | #7F7F7F | Service information |
| 10 | IBM Plex Sans Condensed | 11 px | #7F7F7F | Service labels |

### Numeric display rule

Digital flight/navigation values use **B612 Mono** to maintain stable character widths and rapid visual comparison.

Headers and textual UI elements use **B612** unless a service/information context explicitly uses the fallback font.

## Navigation HMI terminology baseline

The following terms are fixed:

- `GS` — Ground Speed
- `AS` — Air Speed
- `CLIMB` — climb
- `DESCENT` — descent
- `HGT` — true height above ground
- `ALT` — barometric altitude

Do not use `CLIMB RATE` or `DESCENT RATE` in HMI.

Do not use `ALTITUDE` as the heading for the primary altitude indication.

Primary altitude indication:

- below 100 m HGT: `HGT XX m`
- at/above 100 m HGT: `ALT XXX m`

No leading `+` or `-` signs are used for CLIMB, DESCENT, HGT or ALT in the primary indication.

## Units

Internal navigation calculations use SI units:

- speed: m/s
- vertical speed: m/s
- altitude/height: m

User-facing horizontal speed may be switched:

`m/s ↔ km/h`

The display-unit preference must not change the internal calculation unit.

## Component rules

Panels, cards, headers, controls, telemetry blocks and navigation elements are separate components. Layout must support controlled repositioning and resizing.

## Logo rule

Use the supplied BlueSky PRO master asset. Do not redraw, simplify, recolor or replace the logo without an explicit project decision.

## Change control

Any new color, font, spacing, line width or component geometry becomes part of this baseline only after explicit approval.
