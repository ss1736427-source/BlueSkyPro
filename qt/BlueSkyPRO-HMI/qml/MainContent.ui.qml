import QtQuick

Item {
    id: root
    width: 1920
    height: 1080

    // BlueSky PRO — Qt Design Studio working screen.
    // Visual composition only. Core / Safety remain authoritative.
    property int headerHeight: 86
    property int leftWidth: leftPanel.implicitWidth
    property int rightWidth: 340
    property int toolbarHeight: 54
    property int uavPanelHeight: 270
    property bool leftPanelOpen: true
    property bool rightPanelOpen: true
    property string missionState: "AUTO" // AUTO, HIDDEN, MANUAL
    readonly property bool missionVisible: missionState === "AUTO"
    readonly property bool missionCreationMode: missionState === "MANUAL"
    property bool missionReady: false
    property bool manualCompositionComplete: false
    readonly property bool manualValidationStarted: missionState === "VALIDATING"
    property bool warningActive: true
    property int selectedUavIndex: -1
    property bool contextOverlayOpen: false
    readonly property var selectedUav: selectedUavIndex >= 0 && selectedUavIndex < uavStatus.uavModel.length ? uavStatus.uavModel[selectedUavIndex] : null
    readonly property string selectedUavId: selectedUav ? selectedUav.id : "NO UAV SELECTED"
    property string uavDecision: ""
    property string lastJournalEvent: ""
    property string missionId: "BS-260920-A-001"
    // Populated by the mission/task aggregation layer; current value is a design-preview example.
    property string missionSummary: "3D картография территории"
    // Example current automatic mission composition; supplied by mission/task aggregation in production.
    property var missionTemplateIndices: [2]
    property string journalStatus: "READY"
    property string activeTool: bottomToolbar.activeTool
    readonly property bool uavPanelOpen: activeTool === "UAV"
    signal journalEvent(string eventType, int uavIndex, string decision)
    signal journalAppendRequested(string eventType, string missionId, int uavIndex, string decision)
    signal uavDecisionRequested(string decision, int uavIndex)
    signal workspaceContextRequested(string context)
    property string workspaceContext: bottomToolbar.activeTool

    Rectangle {
        anchors.fill: parent
        color: "#050A12"
    }

    TopHeader {
        id: topHeader
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.headerHeight
        tabletVariant: false
        ready: root.missionReady
        warningActive: root.warningActive
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
            visible: root.activeTool !== "UAV" && root.leftPanelOpen
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
            visible: root.activeTool === "MAP" || root.activeTool === "UAV"
            onManualCompositionCompleted: root.manualCompositionComplete = true
            onMapDoubleClicked: root.leftPanelOpen = false
        }

        ToolContext {
            id: toolContext
            anchors.left: leftPanel.right
            anchors.right: rightPanel.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            visible: root.activeTool !== "MAP" && root.activeTool !== "UAV"
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
            visible: root.activeTool !== "UAV" && root.rightPanelOpen
            width: visible ? root.rightWidth : 0
            missionReady: root.missionReady
            warningActive: root.warningActive
            manualCreationMode: root.missionCreationMode
            manualCompositionComplete: root.manualCompositionComplete
            manualValidationStarted: root.manualValidationStarted
            onValidateManualMissionRequested: root.missionState = "VALIDATING"
            onStartMissionRequested: root.leftPanelOpen = false
        }
    }

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
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomToolbar.top
        height: Math.min(root.uavPanelHeight, root.height - root.headerHeight - root.toolbarHeight)
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
        onToolActivated: {
            if (tool !== "UAV")
                root.contextOverlayOpen = false
            root.workspaceContextRequested(tool)
        }
    }

    ContextOverlay {
        id: contextOverlay
        visible: root.contextOverlayOpen && root.selectedUavIndex >= 0
        uavIndex: root.selectedUavIndex
        uavId: root.selectedUavId
        x: Math.max(0, Math.min(root.width - width,
                                root.width * (root.selectedUavIndex + 0.5) / uavStatus.uavModel.length - width / 2))
        anchors.bottom: uavStatus.top
        width: 360
        height: 122
        onDecisionRequested: root.uavDecisionRequested(decision, uavIndex)
        onContextClosed: root.contextOverlayOpen = false
    }
}
