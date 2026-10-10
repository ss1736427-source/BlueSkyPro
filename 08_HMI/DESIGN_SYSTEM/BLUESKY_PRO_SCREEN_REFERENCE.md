# BlueSky PRO — Current Screen Reference

## Left Panel — «Миссии»

The left panel is titled **Миссии**. It provides access to mission templates, the active mission context, and mission creation. The title is Russian-language UI text; the component name `LeftPanel` remains unchanged in QML.

Status: WORKING REFERENCE

## Right Panel — operational control, checklist, INFORMATION and validation — controlled intermediate state

The Right Panel is an **operational panel**, not a duplicate telemetry dashboard. Detailed current telemetry belongs to the UAV Panel; spatial operational information belongs to the Map; non-immediate technical information, history, archives and journals belong to Administration / Technical State / Logs.

### Panel tool configuration

Each panel provides a configuration control for selecting which supported tools/information blocks are displayed on that panel.

Rules:
- the operator can configure the visible tool set for each panel independently;
- configuration changes presentation only and do not change data acquisition, calculation, mission state or control logic;
- tools not selected for display remain available through the panel configuration and are not deleted or disabled;
- newly enabled tools are appended to the panel according to the panel's established layout rules;
- the panel's common visual style is applied uniformly to its displayed tools;
- critical information and safety-relevant warnings cannot be made inaccessible by display filtering;
- the system may temporarily surface a required warning or critical state even when its normal block is not currently displayed;
- panel configuration is persistent for the operator/device context unless explicitly changed.

The panel configuration control belongs to the panel itself and is distinct from the Bottom Toolbar menu, which configures the central Bottom Toolbar tool set.

### Default Right Panel state — Checklist

The default content during flight preparation is **CHECKLIST**.

Header:
`CHECKLIST 5/8 ✓`

- completed stages — green;
- incomplete/pending stages — amber/yellow;
- red — critical/error only.

Completed items may be hidden without changing their underlying state or audit record:
- tablet — swipe right;
- PC — double-click.

The counter remains based on actual checklist state.

### INFORMATION — system messages and pilot intervention

The Right Panel section is titled **INFORMATION** and presents system failures, changes and warnings.

- A new warning synchronizes the Header WARNING state and appears in the INFORMATION overview.
- Selecting a message opens its details.
- Messages requiring pilot intervention provide an entry to the contextual pilot-action area.
- After reading and explicitly confirming a message, it disappears from the overview only. The underlying event remains in the Journal/Audit trail.
- Confirmation acknowledges that the message was read; it does not itself perform or authorize the required operational action.
- When no unacknowledged messages remain, the INFORMATION body is empty. Do not display a positive status message.
- Critical information must not become inaccessible through panel filtering. Serious/critical warnings may also be surfaced over the Map.

The current QML contains illustrative preview messages. Production event feed, durable acknowledgement, Journal/Audit recording and routing into the actual pilot-action workflow require integration with the corresponding system services.

### Weather forecast change warning

For a material weather update affecting the mission, show new TAF/METAR and concise previous → new comparison for:
- temperature;
- wind direction;
- wind speed;
- gusts;
- visibility;
- cloud-base/cloud-height limits.

### Automatic validation

Validation is not permanently visible.

After a relevant change — route/WP, UAV/configuration, battery/resource, equipment, weather/forecast, restrictions/NOTAM or other readiness-affecting input — the system automatically revalidates.

If the automatic validation completes successfully and operator confirmation is required, show `VALIDATE MISSION` with a **dynamic breathing green outline**. After confirmation the outline stops and the control disappears.

If validation finds a problem, show the corresponding warning/error state. If there is no relevant change, no validation button is shown.

### Mission Readiness and Start Mission

`START MISSION` is not green by default. It becomes green and active only when current readiness and all required checklist/validation conditions are satisfied. A readiness-invalidating change returns it to the non-ready/disabled state until rechecked and confirmed where required.

### RETURN — local UAV decision context

**RETURN is removed from the normal Right Panel.**

When a specific UAV has a problem or another reason requiring an operational decision:

1. a warning outline appears around that UAV's area/card in the UAV Panel/interface;
2. the operator clicks the affected UAV area;
3. a local UAV context window opens;
4. the window shows the UAV telemetry and the reason for the problem;
5. the system performs a preliminary situation assessment;
6. the system presents a concise recommendation;
7. the operator makes the decision;
8. the context window collapses after the decision.

The local context contains:

`RETURN` and `ПРОДОЛЖИТЬ ПОЛЁТ`

The system **recommends; the operator decides**. The recommendation is not itself a flight command.

The preliminary assessment may use available operational data such as:
- current telemetry;
- detected problem/deviation;
- battery/resource state;
- remaining distance/time;
- current route and mission state;
- weather;
- communications;
- return/continuation feasibility;
- impact on mission completion;
- possibility of redistributing the affected mission segment among other UAVs.

The recommendation must be accompanied by the relevant reasoning factors rather than being presented as an unexplained command.

If RETURN is selected and mission redistribution is required, redistribution is handled as a subsequent controlled mission operation; it is not silently performed by the recommendation layer.

The event, assessment factors, system recommendation, operator decision and resulting action are recorded in the Journal/Audit trail.

### ABORT

`ABORT` is not part of the normal Right Panel. It appears only in the appropriate emergency/critical context according to the applicable emergency procedure.

### Normal Right Panel action hierarchy

1. CHECKLIST
2. INFORMATION — system messages and required operator actions
3. Mission Readiness
4. contextual validation confirmation after automatic re-check
5. `SEND FLIGHT PLAN`
6. `START MISSION` — active only at full readiness

RETURN is **not** in this global list. It is exposed through the affected UAV's local context.

This is the current controlled intermediate state for further HMI/ergonomic validation; it is not yet the final visual baseline.
