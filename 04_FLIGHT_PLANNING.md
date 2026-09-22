# Flight Planning

The planning concept is based on an energy-aware and constraint-aware mission model.

The planner can account for:

- aircraft capabilities and installed equipment;
- mission objective;
- route geometry;
- altitude constraints;
- restricted areas and operational limitations;
- weather and wind;
- mandatory waypoints;
- lateral and vertical bypass conditions;
- energy reserve and aircraft degradation history;
- multi-UAV task decomposition.

The interface presents one active operational route while alternatives can be evaluated in the planning process.

The design goal is not simply the shortest route. The route is evaluated against the mission objective, operational constraints, environmental conditions and energy preservation.
