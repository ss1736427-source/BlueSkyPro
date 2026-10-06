import QtQuick
import QtCore

Item {
    id: root
    width: 1920
    height: 1080

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
    // Example current automatic mission composition; supplied by mission/task aggregation in production.
    property var missionTemplateIndices: [0, 1, 7]
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

    // Map receives the canonical route geometry maintained by MissionProfileWindow.
    // The HMI only converts its coordinate strings to [latitude, longitude].
    function mapRouteCoordinates() {
        var routes = missionProfileWindow.routeDataByUav
        if (!routes || typeof routes !== "object")
            return []

        var route = selectedUavId !== "NO UAV SELECTED" ? routes[selectedUavId] : null
        if (!Array.isArray(route)) {
            var keys = Object.keys(routes)
            route = keys.length > 0 ? routes[keys[0]] : null
        }
        if (!Array.isArray(route))
            return []

        var result = []
        for (var i = 0; i < route.length; ++i) {
            var pair = String(route[i].coordinates || "").split(",")
            if (pair.length !== 2)
                continue
            var lat = Number(pair[0].trim())
            var lon = Number(pair[1].trim())
            if (isFinite(lat) && isFinite(lon))
                result.push([lat, lon])
        }
        return result
    }

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
        anchors.top: topHeader.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomToolbar.top

        LeftPanel {
            id: leftPanel
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // TopHeader owns the shared horizontal separator.
            showTopBorder: false
            // BottomToolbar owns the shared seam; avoid drawing a second line here.
            showBottomBorder: false
            visible: root.leftPanelOpen
            width: visible ? root.leftWidth : 0
            missionVisible: root.missionVisible
            missionCreationMode: root.missionCreationMode
            missionId: root.missionId
            missionSummary: root.missionSummary
            missionTemplateIndices: root.missionTemplateIndices
            onHideMissionRequested: root.missionState = "HIDDEN"
            onRestoreMissionRequested: root.missionState = "AUTO"
            onCreateMissionRequested: root.missionState = "MANUAL"
        }

        FlightChart {
            id: flightChart
            anchors.left: leftPanel.right
            anchors.right: rightPanel.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            missionVisible: root.missionVisible
            manualCreationMode: root.missionCreationMode
            manualCompositionComplete: root.manualCompositionComplete
            useExternalMapState: true
            mapPanX: root.mapPanX
            mapPanY: root.mapPanY
            mapZoom: root.mapZoom
            routeCoordinates: root.mapRouteCoordinates()
            // Keep the map as the persistent workspace in every mode.
            visible: true
            onMapViewChangeRequested: function(panX, panY, zoom) {
                root.mapPanX = panX
                root.mapPanY = panY
                root.mapZoom = zoom
            }
            onManualCompositionCompleted: root.manualCompositionComplete = true
            onMapDoubleClicked: root.leftPanelOpen = false
        }

        // Expanded mission profile: route table and flight profile use the available workspace.
        MissionProfileWindow {
            id: missionProfileWindow
            visible: root.missionProfileOpen
            missionId: root.missionId
            missionSummary: root.missionSummary
            missionTemplateIndices: root.missionTemplateIndices
            uavModel: uavStatus.uavModel
            missionAssignments: root.missionAssignments
            selectedUavIndex: root.selectedUavIndex
            onUavSelectionRequested: function(index) { root.selectedUavIndex = index }
            onCloseRequested: {
                root.missionProfileOpen = false
                root.taskCreationVisible = true
                root.missionSplashVisible = false
                root.headerAlertsOpen = false
            }
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

        RightPanel {
            id: rightPanel
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // A zero-width panel does not clip its children: hide the whole
            // component when collapsed so buttons/text/popups cannot leak
            // into the Flight Chart.
            visible: root.rightPanelOpen
            width: visible ? root.rightWidth : 0
            // Match the left panel's shared-edge ownership: header and toolbar draw the horizontal seams.
            showTopBorder: false
            showBottomBorder: false
            showLeftBorder: true
            showRightBorder: true
            missionReady: root.missionReady
            warningActive: root.warningActive
            manualCreationMode: root.missionCreationMode
            manualCompositionComplete: root.manualCompositionComplete
            manualValidationStarted: root.manualValidationStarted
            onValidateManualMissionRequested: root.missionState = "VALIDATING"
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
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: root.toolbarHeight
        leftOpen: root.leftPanelOpen
        rightOpen: root.rightPanelOpen
        onLeftPanelToggleRequested: root.leftPanelOpen = !root.leftPanelOpen
        onRightPanelToggleRequested: root.rightPanelOpen = !root.rightPanelOpen
        onToolActivated: root.contextOverlayOpen = false
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
        onMissionSetRequested: function(templateIndices, taskText) {
            root.missionTemplateIndices = templateIndices
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
