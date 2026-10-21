import QtQuick
import QtCore

Item {
    id: root
    // BlueSky PRO — Qt Design Studio working screen.
    // Visual composition only. Core / Safety remain authoritative.
    property int toolbarHeight: 54
    property int headerHeight: toolbarHeight
    property int leftWidth: leftPanel.implicitWidth
    property int rightWidth: leftWidth
    property bool leftPanelOpen: true
    property bool rightPanelOpen: true
    // Mission workflow states: AUTO, HIDDEN, MANUAL, VALIDATING
    property string missionState: "AUTO"
    readonly property bool missionVisible: missionState === "AUTO"
    // Validation is part of the manual-mission workflow; keep its UI active.
    readonly property bool missionCreationMode:
        missionState === "MANUAL" || missionState === "VALIDATING"
    property bool missionReady: false
    property bool manualCompositionComplete: false
    // Shared map view state: retained while switching mission templates and tools.
    property real mapPanX: 0
    property real mapPanY: 0
    property real mapZoom: 1.0
    readonly property bool manualValidationStarted: missionState === "VALIDATING"
    property bool manualSetRequested: false
    property bool manualSetBlocked: false
    property string manualModelValidationStatus: "NOT_CHECKED"
    property string manualMissionModelJson: ""
    property string manualSetBaselineResultId: ""
    // Manual input version advances whenever templates or operator geometry changes.
    property int manualMissionVersion: 1
    property var manualSelectedTemplateIds: []
    property bool warningActive: true
    // Explicit header-triggered map review; independent of the right-panel Map Alerts toggle.
    property bool headerAlertsOpen: false
    readonly property int systemMessageCount: rightPanel.visibleSystemMessages().length
    property int selectedUavIndex: -1
    property bool contextOverlayOpen: false
    readonly property var selectedUav: selectedUavIndex >= 0 && selectedUavIndex < uavStatus.uavModel.length ? uavStatus.uavModel[selectedUavIndex] : null
    readonly property string selectedUavId: selectedUav ? selectedUav.id : "NO UAV SELECTED"
    property string uavDecision: ""
    property string lastJournalEvent: ""
    property string missionId: "BS-260920-A-001"
    // Mission review status is independent of flight readiness.
    property string missionReviewState: "REWORK"
    // Populated by the mission/task aggregation layer; current value is a design-preview example.
    property string missionSummary: "Картография · 3D-реконструкция · Экомониторинг"
    // Design Studio fixture only. Production assignments must come from the planning allocator.
    property var missionAssignments: [
        { uavId: "BS-001", templateIndex: 1, task: "3D-картография", sector: "Северный склон" },
        { uavId: "BS-002", templateIndex: 1, task: "3D-картография", sector: "Южный склон" },
        { uavId: "BS-003", templateIndex: 1, task: "3D-картография", sector: "Западный склон" },
        { uavId: "BS-004", templateIndex: 7, task: "Экомониторинг", sector: "Периметр" }
    ]
    property bool missionProfileOpen: false
    // Planning-core result injected by the application/planning bridge; QML never calculates it.
    // Bind only the validated result published by PlanningBridge.
    // Empty bridge output remains unavailable; QML never invents route/readiness data.
    // Do not expose a late planning result to a different active mission.
    readonly property var planningResult:
        typeof planningBridge !== "undefined"
        && planningBridge !== null
        && planningBridge.result !== undefined
        && planningBridge.result !== null
        && String(planningBridge.result.missionId) === String(root.missionId)
        ? planningBridge.result : null
    // A green SET requires a fresh result for this mission and an authoritative PASS.
    readonly property bool manualSetSucceeded:
        root.manualSetRequested && root.planningResult
        && String(root.planningResult.resultId || "") !== root.manualSetBaselineResultId
        && root.planningResult.verification
        && root.planningResult.verification.finalGateStatus === "PASS"
        && root.planningResult.verification.releaseStatus === "RELEASE_ELIGIBLE"
        && root.planningResult.verification.verified === true
    function syncPlanningRouteCandidate() {
        var result = root.planningResult
        var candidate = result && result.result ? result.result.routeCandidate : null
        if (candidate !== null && candidate !== undefined
                && String(candidate.missionId) === String(root.missionId)) {
            if (typeof routeController !== "undefined" && routeController !== null)
                routeController.loadCandidate(candidate)
        } else {
            // Never leave a route from the previous mission visible when the
            // active mission changes or the bridge no longer has its candidate.
            if (typeof routeController !== "undefined" && routeController !== null)
                routeController.clear()
        }
    }
    readonly property bool is3DMappingProfile: root.missionTemplateIds.indexOf("3D_MAPPING") >= 0
    // Example current automatic mission composition; supplied by mission/task aggregation in production.
    property var missionTemplateIndices: [0, 1, 7]
    property var missionTemplateIds: ["MAPPING", "3D_MAPPING", "ENVIRONMENTAL_MONITORING"]
    property string journalStatus: "READY"
    property string activeTool: bottomToolbar.activeTool
    // Entry flow: role authorization -> pilot task setup -> brief transition -> workspace.
    property bool roleSelectionVisible: true
    property bool taskCreationVisible: false
    property bool missionSplashVisible: false
    property string currentRole: ""
    readonly property bool uavPanelOpen: activeTool === "UAV"
    signal journalEvent(string eventType, int uavIndex, string decision)
    signal journalAppendRequested(string eventType, string missionId, int uavIndex, string decision)
    signal uavDecisionRequested(string decision, int uavIndex)
    signal workspaceContextRequested(string context)
    property string workspaceContext: bottomToolbar.activeTool
    // Shared non-operational integration fixture switch for ADMIN and VIRTUAL FLT.
    // Store only this test control here; it must never imply live-source availability.
    Settings {
        id: integrationSettings
        category: "BlueSkyPRO/ExternalIntegrations"
        property bool testStubsEnabled: true
    }
    property bool integrationTestStubsEnabled: integrationSettings.testStubsEnabled

    // Persist the operator-positioned alert overlay across application launches.
    Settings {
        id: alertOverlaySettings
        category: "BlueSkyPRO/AlertOverlay"
        property real x: 330
        property real y: 24
    }

    Rectangle {
        anchors.fill: parent
        color: "#050A12"
    }

    TopHeader {
        id: topHeader
        z: 100
        visible: root.activeTool !== "ADMIN"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.headerHeight
        leftAnchorWidth: root.leftWidth
        rightAnchorWidth: root.rightWidth
        tabletVariant: false
        ready: root.missionReady
        warningActive: root.systemMessageCount > 0
        warningCount: root.systemMessageCount
        onWarningClicked: {
            if (root.systemMessageCount > 0)
                root.headerAlertsOpen = !root.headerAlertsOpen
        }
    }

    Item {
        id: workspace
        anchors.top: root.activeTool === "ADMIN" ? parent.top : topHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: root.activeTool === "ADMIN" ? parent.bottom : bottomToolbar.top

        LeftPanel {
            id: leftPanel
            z: 20
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // TopHeader owns the shared horizontal separator.
            showTopBorder: false
            // BottomToolbar owns the shared seam; avoid drawing a second line here.
            showBottomBorder: false
            visible: root.leftPanelOpen && root.activeTool !== "ADMIN"
            width: visible ? root.leftWidth : 0
            missionVisible: root.missionVisible
            missionCreationMode: root.missionCreationMode
            missionId: root.missionId
            missionReviewState: root.missionReviewState
            missionSummary: root.missionSummary
            missionTemplateIndices: root.missionTemplateIndices
            onMissionProfileRequested: root.missionProfileOpen = !root.missionProfileOpen
            onHideMissionRequested: root.missionState = "HIDDEN"
            onRestoreMissionRequested: root.missionState = "AUTO"
            onCreateMissionRequested: {
                root.missionState = "MANUAL"
                root.manualCompositionComplete = false
                root.manualSetRequested = false
                root.manualSetBlocked = false
                root.manualSetBaselineResultId = ""
                root.manualModelValidationStatus = "NOT_CHECKED"
                root.manualMissionModelJson = ""
                root.manualSelectedTemplateIds = []
                root.manualMissionVersion += 1
            }
            onManualTemplateSelectionCommitted: function(indices) {
                var nextIds = indices.map(function(index) {
                    return "MT-" + (index + 1 < 10 ? "0" : "") + String(index + 1)
                })
                root.manualSelectedTemplateIds = nextIds
                root.manualMissionVersion += 1
                // Any template change invalidates a prior check for the previous model.
                root.manualSetRequested = false
                root.manualSetBlocked = false
                root.manualSetBaselineResultId = ""
                root.manualCompositionComplete = false
                root.manualMissionModelJson = ""
                root.manualModelValidationStatus = "NOT_CHECKED"
                if (root.missionState === "VALIDATING")
                    root.missionState = "MANUAL"
            }
            onReturnToAutoRequested: root.missionState = "AUTO"
            onTaskCreationRequested: {
                root.taskCreationVisible = true
                root.missionSplashVisible = false
                root.missionProfileOpen = false
                root.headerAlertsOpen = false
            }
        }

        FlightChart {
            id: flightChart
            // The map viewport is independent of side-panel visibility.
            // Panels overlay the persistent map instead of resizing it.
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            missionVisible: root.missionVisible
            manualCreationMode: root.missionCreationMode
            manualCompositionComplete: root.manualCompositionComplete
            leftPanelWidth: root.leftPanelOpen ? root.leftWidth : 0
            rightPanelWidth: root.rightPanelOpen ? root.rightWidth : 0
            useExternalMapState: true
            mapPanX: root.mapPanX
            mapPanY: root.mapPanY
            mapZoom: root.mapZoom
            // Preview route is visible only while its non-authoritative editor is open.
            routeCoordinates: typeof routeController !== "undefined" && routeController !== null
                && routeController.waypoints !== undefined && routeController.waypoints !== null
                ? routeController.waypoints.map(function(p) { return [Number(p.longitudeDeg), Number(p.latitudeDeg)] })
                : []
            manualTemplateIds: root.manualSelectedTemplateIds
            selectedNotamId: rightPanel.selectedNotamId
            // ADMIN is a dedicated workspace. Keep map state in memory but suspend map rendering.
            visible: root.activeTool !== "ADMIN"
            onMapViewChangeRequested: function(panX, panY, zoom) {
                root.mapPanX = panX
                root.mapPanY = panY
                root.mapZoom = zoom
            }
            onManualCompositionCompleted: {
                root.manualCompositionComplete = true
                root.manualMissionVersion += 1
                root.manualSetRequested = false
                root.manualSetBlocked = false
                root.manualSetBaselineResultId = ""
                if (root.missionState === "VALIDATING")
                    root.missionState = "MANUAL"
            }
            onMapDoubleClicked: root.leftPanelOpen = false
        }

        // Expanded mission profile: route table and flight profile use the available workspace.
        MissionProfileWindow {
            id: missionProfileWindow
            visible: root.missionProfileOpen && root.activeTool !== "ADMIN"
            routeController: canonicalRouteController
            missionId: root.missionId
            missionSummary: root.missionSummary
            missionReviewState: root.missionReviewState
            missionTemplateIndices: root.missionTemplateIndices
            uavModel: uavStatus.uavModel
            missionAssignments: root.missionAssignments
            selectedUavIndex: root.selectedUavIndex
            onUavSelectionRequested: function(index) { root.selectedUavIndex = index }
            onCloseRequested: {
                // Close the profile only. Do not navigate to Task Creation.
                root.missionProfileOpen = false
                root.missionSplashVisible = false
                root.headerAlertsOpen = false
            }
            onApplyRequested: root.missionProfileOpen = false
        }

        Connections {
            target: manualGeometryBridge
            function onCompositionCompleteChanged() {
                root.manualCompositionComplete = manualGeometryBridge.compositionComplete
                root.manualMissionModelJson = ""
                root.manualModelValidationStatus = "NOT_CHECKED"
                root.manualMissionVersion += 1
                root.manualSetRequested = false
                root.manualSetBaselineResultId = ""
                if (root.missionState === "VALIDATING")
                    root.missionState = "MANUAL"
            }
            function onGeometryChanged() {
                root.manualMissionModelJson = ""
                root.manualModelValidationStatus = "NOT_CHECKED"
                root.manualMissionVersion += 1
                root.manualSetRequested = false
                root.manualSetBlocked = false
                root.manualSetBaselineResultId = ""
                if (root.missionState === "VALIDATING")
                    root.missionState = "MANUAL"
            }
        }

        Connections {
            target: planningBridge
            function onResultChanged() {
                root.syncPlanningRouteCandidate()
                if (root.missionState === "VALIDATING" && root.planningResult
                        && String(root.planningResult.resultId || "") !== root.manualSetBaselineResultId) {
                    if (!root.manualSetSucceeded) {
                        // Failed or blocked checks return to editable manual setup.
                        root.missionState = "MANUAL"
                        root.manualSetRequested = false
                    }
                }
            }
        }

        Connections {
            target: root
            function onMissionIdChanged() {
                root.syncPlanningRouteCandidate()
            }
        }

        ThreeDMappingProfile {
            id: threeDMappingProfile
            // 3D-specific overlay is disabled here; the canonical mission profile owns the route table.
            visible: false
            missionId: root.missionId
            missionSummary: root.missionSummary
            uavModel: uavStatus.uavModel
            selectedUavIndex: root.selectedUavIndex
            planningResult: root.planningResult
            onCloseRequested: root.missionProfileOpen = false
            onApplyRequested: root.missionProfileOpen = false
        }

        ToolContext {
            id: toolContext
            anchors.left: leftPanel.right
            anchors.right: rightPanel.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // Do not replace the map workspace while creating a mission.
            visible: !root.missionCreationMode && root.activeTool !== "MAP" && root.activeTool !== "UAV"
                     && root.activeTool !== "ADMIN" && root.activeTool !== "VIRTUAL FLT"
            contextName: root.activeTool
            selectedUavIndex: root.selectedUavIndex
            selectedUavId: root.selectedUavId
            contextSubtitle: root.activeTool === "UAV" ? "SELECT UAV / CONTROL / C2 / CONFIGURATION" : root.activeTool === "ADMIN" ? "SYSTEM ADMINISTRATION / ENGINEER / TECHNICIAN" : root.activeTool === "FPV" ? "VIDEO + FLIGHT DATA + CONTROL TRANSFER" : "SIMULATION / VIRTUAL UAV"
            sections: root.activeTool === "UAV"
                      ? ["UAV SELECTION", "CONTROL / C2", "UAV CONFIGURATION", "NAVIGATION", "ENERGY", "PAYLOAD / EQUIPMENT", "MAINTENANCE", "DIAGNOSTICS"]
                      : root.activeTool === "ADMIN"
                      ? ["USERS", "ROLES & ACCESS", "SYSTEM SETTINGS", "INTEGRATIONS", "DATA & SYNC", "DOCUMENTS", "AUDIT LOG"]
                      : root.activeTool === "FPV"
                      ? ["UAV SELECTION", "CONTROL STATION", "CONTROL MAPPING", "C2 / VIDEO STATE", "MANUAL CONTROL", "RETURN TO AUTO"]
                      : ["VIRTUAL UAV", "SIMULATION", "ENVIRONMENT", "SCENARIOS", "PLANNED / SIMULATED / ACTUAL", "RESULTS"]
        }

        AdminWorkspace {
            anchors.fill: parent
            z: 40
            // Entering ADMIN must work from manual mission editing too.
            // This is a full workspace switch, not a floating layer over the map.
            visible: root.activeTool === "ADMIN"
            onExitRequested: {
                root.taskCreationVisible = false
                root.missionSplashVisible = false
                root.roleSelectionVisible = true
                bottomToolbar.activateTool("MAP")
            }
        }

        // Virtual Flight uses the same map and HMI as normal operations.
        // The only mode-specific visual indicator is a subtle, non-interactive watermark.
        Item {
            id: virtualFlightOverlay
            anchors.left: leftPanel.right
            anchors.right: rightPanel.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            visible: root.activeTool === "VIRTUAL FLT"
            z: 10
            clip: true

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: "VIRTUAL FLIGHT"
                color: "#B8E8F7"
                opacity: 0.30
                font.family: "B612"
                font.pixelSize: Math.max(22, Math.min(34, parent.width * 0.035))
                font.bold: true
                font.letterSpacing: 3
                horizontalAlignment: Text.AlignHCenter
                style: Text.Outline
                styleColor: "#182C3B"
            }
        }

        RightPanel {
            id: rightPanel
            z: 20
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // A zero-width panel does not clip its children: hide the whole
            // component when collapsed so buttons/text/popups cannot leak
            // into the Flight Chart.
            visible: root.rightPanelOpen && root.activeTool !== "ADMIN"
            width: visible ? root.rightWidth : 0
            // RightPanel is already anchored above the BottomToolbar; do not subtract the toolbar twice.
            bottomInset: 0
            // Match the left panel's shared-edge ownership: header and toolbar draw the horizontal seams.
            showTopBorder: false
            showBottomBorder: false
            showLeftBorder: true
            showRightBorder: true
            missionReady: root.missionReady
            notamRecords: (typeof faaNmsClient !== "undefined" && faaNmsClient !== null) ? faaNmsClient.notamRecords : []
            notamSnapshotId: (typeof faaNmsClient !== "undefined" && faaNmsClient !== null) ? faaNmsClient.notamSnapshotId : ""
            warningActive: root.warningActive
            manualCreationMode: root.missionCreationMode
            manualCompositionComplete: root.manualCompositionComplete
            manualValidationStarted: root.manualValidationStarted
            manualValidationSucceeded: root.manualSetSucceeded
            manualSetBlocked: root.manualSetBlocked
            manualModelValidationStatus: root.manualModelValidationStatus
            onSetManualMissionRequested: {
                // Build the typed Common Mission Model from the exact geometry
                // currently published by the map. This validates structure only;
                // it does not calculate a route or authorize flight.
                var modelResult = manualMissionModelAdapter.buildMission(
                            manualGeometryBridge.geometryJson,
                            root.manualSelectedTemplateIds,
                            root.missionId,
                            root.manualMissionVersion,
                            root.manualCompositionComplete)
                root.manualModelValidationStatus = modelResult.valid
                        ? "MODEL_STRUCTURALLY_VALID_ROUTE_GENERATION_NOT_CONNECTED"
                        : modelResult.errors.join(", ")
                root.manualMissionModelJson = modelResult.valid
                        ? String(modelResult.missionModelJson || "") : ""
                if (!modelResult.valid) {
                    root.manualSetRequested = false
                    root.manualSetBlocked = true
                    root.missionState = "MANUAL"
                    return
                }

                // Keep release blocked until a versioned Planning Core request
                // consumes this canonical model and returns fresh authoritative
                // route + safety verification results.
                root.manualSetRequested = false
                root.manualSetBlocked = true
                root.missionState = "MANUAL"
            }
            onStartMissionRequested: root.leftPanelOpen = false
        }

        // Floating alert review stays entirely on the map; opening a message never opens the right panel.
        // Drag the header to reposition; the last position is persisted in Settings.
        Item {
            id: alertOverlay
            visible: root.headerAlertsOpen
                     && !root.roleSelectionVisible
                     && !root.taskCreationVisible
                     && !root.missionSplashVisible
                     && rightPanel.visibleSystemMessages().length > 0
            x: alertOverlaySettings.x
            y: alertOverlaySettings.y
            width: Math.max(180, Math.min(380, workspace.width - 24))
            height: alertOverlayHeader.height + 8
                    + (rightPanel.selectedInformationMessage
                       ? alertDetails.implicitHeight
                       : alertCards.implicitHeight)
                    + 12
            z: 90

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: "#08111D"
                border.color: "#236078"
                border.width: 1
                antialiasing: true
            }

            Rectangle {
                id: alertOverlayHeader
                x: 1
                y: 1
                width: parent.width - 2
                height: 30
                radius: 7
                color: "#0B1B2B"

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: rightPanel.selectedInformationMessage
                          ? rightPanel.selectedInformationMessage.kind
                          : "ALERTING"
                    color: "#FFFFFF"
                    font.family: "B612"
                    font.pixelSize: 14
                    font.bold: true
                }
                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: rightPanel.selectedInformationMessage ? "‹" : "⠿"
                    color: "#32FFFF"
                    font.pixelSize: 18
                    visible: !rightPanel.selectedInformationMessage
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.SizeAllCursor
                    drag.target: alertOverlay
                    drag.minimumX: 0
                    drag.maximumX: Math.max(0, workspace.width - alertOverlay.width)
                    drag.minimumY: 0
                    drag.maximumY: Math.max(0, workspace.height - alertOverlay.height)
                    onReleased: {
                        alertOverlaySettings.x = alertOverlay.x
                        alertOverlaySettings.y = alertOverlay.y
                    }
                }
            }

            Column {
                id: alertCards
                visible: !rightPanel.selectedInformationMessage
                x: 8
                y: 38
                width: parent.width - 16
                spacing: 6

                Repeater {
                    model: rightPanel.visibleSystemMessages()

                    delegate: Item {
                        width: alertCards.width
                        height: alertText.implicitHeight + 18

                        Rectangle {
                            anchors.fill: parent
                            radius: 4
                            color: "#000000"
                            border.width: 1
                            border.color: modelData.severity === "critical" ? "#FF1E14"
                                          : modelData.severity === "warning" ? "#FFD339"
                                          : "#236078"
                        }

                        Text {
                            id: alertText
                            x: 8
                            y: 5
                            width: parent.width - 16
                            text: (modelData.kind === "FAILURE" ? "✕  " :
                                   modelData.kind === "WARNING" ? "⚠  " : "•  ")
                                  + modelData.kind + "\n" + modelData.title
                            color: "#FFFFFF"
                            font.family: "B612"
                            font.pixelSize: 13
                            lineHeight: 1.15
                            wrapMode: Text.Wrap
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: rightPanel.openSystemMessage(modelData)
                        }
                    }
                }
            }

            Column {
                id: alertDetails
                visible: !!rightPanel.selectedInformationMessage
                x: 12
                y: 38
                width: parent.width - 24
                spacing: 8

                Text {
                    width: parent.width
                    text: rightPanel.selectedInformationMessage
                          ? rightPanel.selectedInformationMessage.title : ""
                    color: "#FFFFFF"
                    font.family: "B612"
                    font.pixelSize: 14
                    font.bold: true
                    wrapMode: Text.Wrap
                }

                Text {
                    width: parent.width
                    text: rightPanel.interventionMode
                          ? "ТРЕБУЕТСЯ ДЕЙСТВИЕ ПИЛОТА"
                          : (rightPanel.selectedInformationMessage
                             ? rightPanel.selectedInformationMessage.detail : "")
                    color: "#D8E4EF"
                    font.family: "B612"
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                }

                Text {
                    visible: !!rightPanel.selectedInformationMessage
                             && rightPanel.selectedInformationMessage.requiresIntervention
                    width: parent.width
                    text: rightPanel.selectedInformationMessage
                          ? rightPanel.selectedInformationMessage.action : ""
                    color: "#32FFFF"
                    font.family: "B612"
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                }

                Row {
                    width: parent.width
                    spacing: 8

                    Rectangle {
                        visible: !!rightPanel.selectedInformationMessage
                                 && rightPanel.selectedInformationMessage.requiresIntervention
                                 && !rightPanel.interventionMode
                        width: (parent.width - parent.spacing) * 0.58
                        height: 30
                        radius: 3
                        color: "#0B1B2B"
                        border.color: "#32FFFF"

                        Text {
                            anchors.fill: parent
                            text: "К ДЕЙСТВИЮ"
                            color: "#32FFFF"
                            font.family: "B612"
                            font.pixelSize: 10
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                rightPanel.interventionMode = true
                                rightPanel.pilotInterventionRequested(
                                    rightPanel.selectedInformationMessage.id)
                            }
                        }
                    }

                    Rectangle {
                        width: (rightPanel.selectedInformationMessage
                                && rightPanel.selectedInformationMessage.requiresIntervention
                                && !rightPanel.interventionMode)
                               ? (parent.width - parent.spacing) * 0.42
                               : parent.width
                        height: 30
                        radius: 3
                        color: "transparent"
                        border.color: "#7F7F7F"

                        Text {
                            anchors.fill: parent
                            text: rightPanel.selectedInformationMessage
                                  && rightPanel.selectedInformationMessage.requiresIntervention
                                  && !rightPanel.interventionMode
                                  ? "НАЗАД" : "ПОДТВЕРДИТЬ"
                            color: "#D8E4EF"
                            font.family: "B612"
                            font.pixelSize: 10
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (rightPanel.selectedInformationMessage
                                    && rightPanel.selectedInformationMessage.requiresIntervention
                                    && !rightPanel.interventionMode) {
                                    rightPanel.selectedInformationMessage = null
                                } else {
                                    rightPanel.acknowledgeInformationMessage()
                                    if (rightPanel.visibleSystemMessages().length === 0)
                                        root.headerAlertsOpen = false
                                }
                            }
                        }
                    }
                }
            }

            Component.onCompleted: {
                x = Math.max(0, Math.min(workspace.width - width, alertOverlaySettings.x))
                y = Math.max(0, Math.min(workspace.height - height, alertOverlaySettings.y))
            }
            onWidthChanged: x = Math.max(0, Math.min(workspace.width - width, x))
            onHeightChanged: y = Math.max(0, Math.min(workspace.height - height, y))
        }

        // Keep fleet and side panels in the same coordinate space so anchors
        // resolve correctly. Cards are centered within the available workspace.
        UAVFleetPanel {
            id: uavStatus
            visible: root.uavPanelOpen
            z: 20
            selectedIndex: root.selectedUavIndex
            onUavSelected: {
                root.selectedUavIndex = index
                root.contextOverlayOpen = false
            }
            onUavDoubleClicked: {
                root.selectedUavIndex = index
                root.contextOverlayOpen = true
            }
            anchors.left: leftPanel.right
            anchors.right: rightPanel.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
        }

        // Keep the overlay in the same coordinate space as the UAV fleet.
        ContextOverlay {
            id: contextOverlay
            visible: root.contextOverlayOpen && root.selectedUavIndex >= 0
            uavIndex: root.selectedUavIndex
            uavId: root.selectedUavId
            // Match the selected card's width, then clamp to the workspace.
            width: Math.max(minWidth, Math.min(maxWidth,
                uavStatus.cardRect(root.selectedUavIndex).width, parent.width - 16))
            height: implicitHeight
            x: {
                var card = uavStatus.cardRect(root.selectedUavIndex)
                var cardCenterX = uavStatus.x + card.x + card.width / 2
                return Math.max(8, Math.min(parent.width - width - 8, cardCenterX - width / 2))
            }
            y: {
                var card = uavStatus.cardRect(root.selectedUavIndex)
                var cardTop = uavStatus.y + card.y
                var above = cardTop - height - 8
                // Prefer directly above the affected card; if space is limited,
                // place it over the card while keeping the whole panel in view.
                return above >= 8 ? above
                                  : Math.max(8, Math.min(parent.height - height - 8,
                                        cardTop + (card.height - height) / 2))
            }
            z: 30
            onDecisionRequested: root.uavDecisionRequested(decision, uavIndex)
            onContextClosed: root.contextOverlayOpen = false
        }
    }

    BottomToolbar {
        id: bottomToolbar
        visible: root.activeTool !== "ADMIN"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: root.toolbarHeight
        leftOpen: root.leftPanelOpen
        rightOpen: root.rightPanelOpen
        onLeftPanelToggleRequested: root.leftPanelOpen = !root.leftPanelOpen
        onRightPanelToggleRequested: root.rightPanelOpen = !root.rightPanelOpen
        onToolActivated: {
            root.contextOverlayOpen = false
            if (tool === "ADMIN") {
                // Hide flight-only overlays but preserve the mission and geometry
                // so returning to MAP does not discard operator work.
                root.headerAlertsOpen = false
                root.taskCreationVisible = false
                root.missionSplashVisible = false
                root.missionProfileOpen = false
            }
        }
    }

    // Authorization is the first screen. Pilot proceeds to task composition.
    RoleSelection {
        id: roleSelection
        anchors.fill: parent
        z: 1000
        visible: root.roleSelectionVisible
        onContinueRequested: function(role) {
            root.currentRole = role
            root.roleSelectionVisible = false
            if (role === "PILOT") {
                root.taskCreationVisible = true
            } else if (role === "ADMIN") {
                bottomToolbar.activateTool("ADMIN")
            } else {
                bottomToolbar.activateTool("UAV")
            }
        }
    }

    // Tablet-first task setup: all approved mission templates and the on-screen keyboard.
    TaskCreation {
        id: taskCreation
        anchors.fill: parent
        z: 1001
        visible: root.taskCreationVisible
        onMissionSetRequested: function(templateIndices, taskText, templateIds) {
            root.missionTemplateIndices = templateIndices
            root.missionTemplateIds = templateIds
            root.missionSummary = taskText.trim().length > 0
                                  ? taskText.trim()
                                  : "Выбрано шаблонов: " + templateIndices.length
            root.missionState = "AUTO"
            root.taskCreationVisible = false
            root.missionSplashVisible = true
            missionTransition.restart()
        }
    }

    // Short visual hand-off before the main map workspace appears.
    Timer {
        id: missionTransition
        interval: 1000
        repeat: false
        onTriggered: root.missionSplashVisible = false
    }

    Rectangle {
        anchors.fill: parent
        z: 1002
        visible: root.missionSplashVisible
        color: "#050A12"

        Column {
            anchors.centerIn: parent
            spacing: 18
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "BlueSky PRO"
                color: "#E5F0FA"
                font.family: "B612"
                font.pixelSize: 38
                font.bold: true
            }
            Rectangle {
                width: 220
                height: 2
                color: "#20C8F4"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "ФОРМИРОВАНИЕ МИССИИ"
                color: "#9FB4C9"
                font.family: "B612"
                font.pixelSize: 15
                font.letterSpacing: 2
            }
        }
    }

}
