# BlueSky PRO — Controlled Logo Assets

## Source assets supplied for the current HMI work

The following source files were supplied in the working session and are to be preserved as project assets:

- `BlueSky_Icon_MASTER_610x530.png` — standalone mark, RGBA, 610×530.
- `BlueSky_Logo_MAIN_Transparent_2031x774(1).png` — main transparent logo, RGBA, 2031×774.
- `BlueSky_Logo_MASTER_2048x768.png` — supplied master-named logo asset; file metadata currently reports 2031×774 RGBA.
- `bluesky-logo(1).svg` — supplied SVG container, viewBox 2031×774. The supplied file contains an embedded raster image and therefore must be treated as an asset container, not assumed to be a pure path-only SVG.
- `PC_BlueSky_Logo_1024x384_2x.png` — PC UI export, RGBA, 1024×384.
- `Tablet_BlueSky_Logo_768x288_2x.png` — tablet UI export, RGBA, 768×288.
- `Tablet_BlueSky_Logo_1536x576.png` — tablet high-resolution export, RGBA, 1536×576.
- `UI_BlueSky_Logo_512x192.png` — UI export, RGBA, 512×192.
- `Логотип прозрачный фон(1).png` — transparent logo export, RGBA, 2048×768.

## Control rule

These supplied assets are the visual reference. Do not redraw, simplify, recolor, alter proportions, or substitute another logo during HMI reconstruction without an explicit project decision.

## Figma

Use the master/reference asset for visual placement and scaling. Keep the logo as a separate component/asset so it can be replaced by the final controlled vector master without changing the surrounding layout.

## Qt 6 + QML

The final application resource should originate from the same controlled master asset. Do not create a second independently redrawn logo.

## Status

ASSET_REFERENCE_SET — supplied and catalogued in the working project. Binary files must be committed to the repository's controlled asset store as the next repository asset operation.
