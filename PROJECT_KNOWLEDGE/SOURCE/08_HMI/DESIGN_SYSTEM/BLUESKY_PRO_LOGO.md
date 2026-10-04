---
id: HMI-LOGO-001
type: brand_asset_control
status: pending_asset
system: BlueSky PRO
---

# BlueSky PRO — Logo Asset

## Current state

В проектных материалах зафиксирована фирменная композиция BlueSky PRO и отдельные storyboard-варианты анимации логотипа. В качестве основного asset должен использоваться утверждённый пользователем исходный векторный логотип, а не перерисованная версия.

## Required asset package

```text
08_HMI/ASSETS/logo/
├── BlueSky_PRO.svg
├── BlueSky_PRO_mark.svg
└── README.md
```

## Usage rules

- SVG является master asset.
- Не менять геометрию логотипа при трассировке HMI.
- Не заменять фирменную типографику логотипа UI-шрифтом.
- Размерирование выполнять без искажения пропорций.
- Для Figma использовать векторный master.
- Для Qt 6 + QML подготовить SVG/ресурсную версию из того же master.

## Animation reference

Storyboard предусматривает последовательность: тёмно-синий экран → горизонтальная световая линия → проявление BlueSky → появление PRO → объединение → световой проход → плавное проявление интерфейса.

## Status

PENDING_ASSET. Утверждённый исходный SVG логотипа требуется добавить в controlled repository. До этого любые визуальные реконструкции считаются временными.
