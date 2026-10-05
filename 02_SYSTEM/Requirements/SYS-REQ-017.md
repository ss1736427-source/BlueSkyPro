---
id: SYS-REQ-017
type: system_requirement
title: Edge AI и Cloud AI
status: draft
criticality: high
source:
  - "[[BLUE_SKY_PRO_онцепция(2)]]"
verification_method: analysis
---

# SYS-REQ-017 — Edge AI и Cloud AI

## Requirement

Система должна разделять AI-функции между Edge и Cloud/Hub.

а Edge должны выполняться, в частности:

- detection;
- tracking;
- classification;
- obstacle avoidance;
- sensor fusion;
- критические решения.

а Cloud/Hub должны выполняться, в частности:

- тяжёлая аналитика;
- архив;
- GIS;
- обучение;
- cross-mission analysis;
- отчётность;
- knowledge base.

Система не должна передавать все исходные данные без необходимости и должна передавать результат обработки.

## Source
[[BLUE_SKY_PRO_концепция(2)]]]



