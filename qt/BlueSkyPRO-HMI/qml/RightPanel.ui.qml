import QtQuick
import QtCore

Item {
    id: root

    // Prevent child controls from painting outside the panel when its
    // width is collapsed to zero by MainContent.
    clip: true
    implicitWidth: 270

    property color bg: "#08111D"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color muted: "#7F7F7F"
    property color green: "#64FF00"
    property color amber: "#FFD339"
    property color red: "#FF1E14"
    property color cyan: "#32FFFF"
    property color divider: "#7F7F7F"

    // Match LeftPanel outline; shared top/bottom seams are owned by header/toolbar.
    property bool showTopBorder: true
    property bool showRightBorder: true
    property bool showBottomBorder: true
    property bool showLeftBorder: true

    // Contextual validation is not shown until automatic revalidation succeeds.
    property bool validationConfirmationRequired: false
    property bool manualCreationMode: false
    property bool manualCompositionComplete: false
    property bool manualValidationStarted: false
    property bool missionReady: false
    property bool warningActive: true

    Settings {
        id: panelOrderSettings
        category: "BlueSkyPRO/RightPanel"
        property string orderCsv: "Checklist,Flight Conditions,Alerting,ATC"
    }

    property var panelOrder: panelOrderSettings.orderCsv.split(",")
    property string draggingPanel: ""
    property real dragOffsetY: 0
    property real dragPressRootY: 0
    property real dragVisualY: 0
    property real dragGrabOffsetY: 0

    function beginPanelDrag(key, pressRootY) {
        if (!panelVisible(key))
            return
        draggingPanel = key
        dragOffsetY = 0
        dragPressRootY = pressRootY
        dragVisualY = panelBaseY(key)
        dragGrabOffsetY = pressRootY - dragVisualY
    }

    function updatePanelDrag(key, currentRootY) {
        if (draggingPanel !== key)
            return

        dragVisualY = currentRootY - dragGrabOffsetY
        dragOffsetY = dragVisualY - panelBaseY(key)
        reorderPanelAtPosition(key)
    }


    Component.onCompleted: {
        var migratedOrder = panelOrder.filter(function(key) {
            return key !== "Information"
        })
        var requiredOrder = ["Checklist", "Flight Conditions", "Alerting", "ATC"]
        for (var i = 0; i < requiredOrder.length; ++i) {
            if (migratedOrder.indexOf(requiredOrder[i]) < 0)
                migratedOrder.push(requiredOrder[i])
        }
        panelOrder = migratedOrder
        panelOrderSettings.orderCsv = migratedOrder.join(",")
    }

    function panelVisible(key) {
        if (key === "Information")
            return true
        if (key === "Checklist")
            return panelSettingsPopup.enabledTools.indexOf("Checklist") >= 0
        if (key === "Flight Conditions")
            return ((panelSettingsPopup.enabledTools.indexOf("Weather") >= 0
                     || root.weatherPilotAttentionRequired)
                    || (panelSettingsPopup.enabledTools.indexOf("NOTAM") >= 0
                        || root.notamPilotAttentionRequired))
        if (key === "Alerting")
            return panelSettingsPopup.enabledTools.indexOf("Information") >= 0
                   || root.hasUnacknowledgedCriticalMessage()
        if (key === "ATC")
            return root.atcHeaderVisible || root.atcVisibleButtonCount > 0
        return false
    }

    function panelHeight(key) {
        if (key === "Information")
            return informationTitleCard.height
        if (key === "Checklist")
            return checklistCard.height
        if (key === "Flight Conditions")
            return operationalCard.height
        if (key === "Alerting")
            return informationCard.height
        if (key === "ATC")
            return atcWorkArea.height
        return 0
    }

    function panelBaseY(key) {
        var y = 54
        for (var i = 0; i < panelOrder.length; ++i) {
            var current = panelOrder[i]
            if (current === key)
                return y
            if (panelVisible(current))
                y += panelHeight(current) + 10
        }
        return y
    }

    function reorderPanelAtPosition(key) {
        var order = panelOrder.slice()
        var from = order.indexOf(key)
        if (from < 0)
            return

        var center = dragVisualY + panelHeight(key) / 2
        var target = from
        var y = 54

        for (var i = 0; i < order.length; ++i) {
            var other = order[i]
            if (other === key || !panelVisible(other))
                continue

            var h = panelHeight(other)
            if (center < y + h / 2) {
                target = i
                break
            }
            y += h + 10
            target = i + 1
        }

        if (target > from)
            target--

        target = Math.max(0, Math.min(order.length - 1, target))
        if (target === from)
            return

        order.splice(from, 1)
        order.splice(target, 0, key)

        panelOrder = order
        panelOrderSettings.orderCsv = order.join(",")
        panelOrderSettings.sync()
    }

    function finishPanelDrag(key) {
        if (draggingPanel !== key)
            return

        // The order has already been updated continuously while dragging.
        // Release only commits the final visual position and clears drag state.
        reorderPanelAtPosition(key)
        dragOffsetY = 0
        dragVisualY = 0
        dragGrabOffsetY = 0
        draggingPanel = ""
    }

    function finishPanelDrag(key) {
        if (draggingPanel !== key)
            return
        reorderPanel(key)
        draggingPanel = ""
    }
    // Preview state only. Live weather/NOTAM providers must supply authoritative data.
    property string selectedOperationalTool: ""
    // Set only by the authoritative route-planning/revalidation result.
    // HMI must not infer pilot attention from weather/NOTAM presence alone.
    property bool weatherPilotAttentionRequired: false
    property bool notamPilotAttentionRequired: false
    property var checklistItems: [
        { label: "Mission definition", status: "PENDING" },
        { label: "UAV allocation", status: "PENDING" },
        { label: "Route validation", status: "PENDING" },
        { label: "NOTAM / Airspace", status: "PENDING" },
        { label: "Weather", status: "PENDING" },
        { label: "Terrain / Obstacles", status: "PENDING" },
        { label: "Battery / Payload", status: "PENDING" },
        { label: "C2 / GNSS", status: "PENDING" },
        { label: "Permissions", status: "PENDING" },
        { label: "Final validation", status: "PENDING" }
    ]
    readonly property int checklistPassedCount: checklistItems.filter(function(item) {
        return item.status === "PASS"
    }).length
    readonly property bool mapAlertsEnabled:
        panelSettingsPopup.enabledTools.indexOf("Map Alerts") >= 0
    property var systemMessages: [
        { id: "SYS-C2-001", kind: "FAILURE", title: "Потеря связи C2", detail: "Связь с БПЛА требует проверки. Проверьте состояние канала и доступность аппарата.", action: "Проверить связь с БПЛА", requiresIntervention: true, severity: "critical" },
        { id: "SYS-WIND-001", kind: "WARNING", title: "Коррекция ветра требует подтверждения", detail: "Изменение ветровых условий повлияло на расчёт маршрута. Проверьте обновлённую коррекцию.", action: "Проверить коррекцию маршрута", requiresIntervention: true, severity: "warning", sourceTool: "WEATHER" },
        { id: "SYS-BAT-001", kind: "CHANGE", title: "Применена модель деградации батареи", detail: "Расчёт производительности учитывает деградацию аккумулятора.", action: "", requiresIntervention: false, severity: "info" }
    ]
    property var acknowledgedMessageIds: []
    property var selectedInformationMessage: null
    property bool interventionMode: false
    signal pilotInterventionRequested(string messageId)
    property bool validationVisible: manualCreationMode ? !manualValidationStarted : validationConfirmationRequired
    property real validationPulse: 1.0

    // ATC work area sizes itself to the currently enabled action buttons.
    property bool atcHeaderVisible: panelSettingsPopup.enabledTools.indexOf("Readiness") >= 0
    property bool atcValidationVisible: root.validationVisible && panelSettingsPopup.enabledTools.indexOf("Validation") >= 0
    property bool atcSendVisible: panelSettingsPopup.enabledTools.indexOf("Send Flight Plan") >= 0
    property bool atcStartVisible: panelSettingsPopup.enabledTools.indexOf("Start Mission") >= 0
    property int atcVisibleButtonCount: (atcValidationVisible ? 1 : 0)
                                        + (atcSendVisible ? 1 : 0)
                                        + (atcStartVisible ? 1 : 0)
    property int atcButtonHeight: 38
    property int atcButtonGap: 10
    property int atcButtonStackTop: atcHeaderVisible ? 43 : 12
    property int atcWorkAreaHeight: (atcHeaderVisible ? 33 : 12)
                                    + (atcHeaderVisible && atcVisibleButtonCount > 0 ? atcButtonGap : 0)
                                    + atcVisibleButtonCount * atcButtonHeight
                                    + Math.max(0, atcVisibleButtonCount - 1) * atcButtonGap
                                    + 12
    property int atcValidationY: atcWorkArea.y + atcButtonStackTop
    property int atcSendY: atcValidationY + (atcValidationVisible ? atcButtonHeight + atcButtonGap : 0)
    property int atcStartY: atcValidationY
                            + (atcValidationVisible ? atcButtonHeight + atcButtonGap : 0)
                            + (atcSendVisible ? atcButtonHeight + atcButtonGap : 0)

    signal startMissionRequested()
    signal validateManualMissionRequested()

    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    // Edge ownership mirrors LeftPanel. MainContent disables the top and bottom
    // edges so TopHeader and BottomToolbar each provide one continuous shared line.
    Rectangle {
        visible: root.showTopBorder
        x: 0
        y: 0
        width: parent.width
        height: 1
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        visible: root.showRightBorder
        x: parent.width - 1
        y: 0
        width: 1
        height: parent.height
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        visible: root.showBottomBorder
        x: 0
        y: parent.height - 1
        width: parent.width
        height: 1
        color: root.cyan
        antialiasing: false
    }
    Rectangle {
        visible: root.showLeftBorder
        x: 0
        y: 0
        width: 1
        height: parent.height
        color: root.cyan
        antialiasing: false
    }

    // INFORMATION is the fixed right-panel title, not a reorderable card.
    Rectangle {
        id: informationTitleCard
        x: 16
        y: 8
        width: parent.width - 32
        height: 38
        color: "transparent"

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            text: "INFORMATION"
            color: root.text
            font.family: "B612"
            font.pixelSize: 15
            font.bold: true
            elide: Text.ElideRight
        }
    }

    // Checklist card — same rounded, outlined visual language as ATC.
    Rectangle {
        id: checklistCard
        visible: root.panelVisible("Checklist")
        x: 16
        y: root.draggingPanel === "Checklist" ? root.dragVisualY : root.panelBaseY("Checklist")
        width: parent.width - 32
        height: Math.min(42 + root.visibleChecklistItems().length * 18, Math.max(90, parent.height * 0.34))
        radius: 8
        color: "transparent"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
        z: root.draggingPanel === "Checklist" ? 200 : 1
    }

    Rectangle {
        visible: checklistCard.visible
        x: checklistCard.x + 1
        y: checklistCard.y + 1
        width: checklistCard.width - 2
        height: 32
        radius: 7
        color: "#0B1B2B"
        antialiasing: true

        Rectangle {
            x: 0
            y: height / 2
            width: parent.width
            height: parent.height / 2
            color: parent.color
        }

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: checklistCounts.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            text: "CHECKLIST"
            color: root.text
            font.family: "B612"
            font.pixelSize: 13
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        MouseArea {
            // The panel header is the drag handle. No visual grip is shown.
            anchors.fill: parent
            preventStealing: true
            onPressed: root.beginPanelDrag("Checklist", mapToItem(root, mouse.x, mouse.y).y)
            onPositionChanged: root.updatePanelDrag("Checklist", mapToItem(root, mouse.x, mouse.y).y)
            onReleased: {
                root.updatePanelDrag("Checklist", mapToItem(root, mouse.x, mouse.y).y)
                root.finishPanelDrag("Checklist")
            }
            onCanceled: root.finishPanelDrag("Checklist")
        }

        Row {
            id: checklistCounts
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 3

            Text {
                text: String(root.checklistPassedCount)
                color: root.checklistPassedCount === root.checklistItems.length ? root.green : root.amber
                font.family: "B612"
                font.pixelSize: 13
                font.bold: true
            }
            Text {
                text: "/"
                color: root.secondary
                font.family: "B612"
                font.pixelSize: 13
                font.bold: true
            }
            Text {
                text: String(root.checklistItems.length)
                color: "#FF00D4"
                font.family: "B612"
                font.pixelSize: 13
                font.bold: true
            }
        }
    }

    Flickable {
        id: checklistScroller
        x: checklistCard.x + 12
        y: checklistCard.y + 39
        width: checklistCard.width - 24
        height: Math.max(0, checklistCard.height - 48)
        contentWidth: width
        contentHeight: checklistList.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        visible: checklistCard.visible

        Column {
            id: checklistList
            width: checklistScroller.width
            spacing: 1
            Repeater {
                model: root.visibleChecklistItems()
                delegate: Text {
                    width: checklistList.width
                    height: 17
                    text: (modelData.status === "PASS" ? "✓" :
                           modelData.status === "FAIL" ? "✕" :
                           modelData.status === "WARNING" ? "⚠" : "○")
                          + "  " + modelData.label
                    color: modelData.status === "PASS" ? root.green :
                           modelData.status === "FAIL" ? root.red :
                           modelData.status === "WARNING" ? root.amber : root.secondary
                    font.family: "B612"
                    font.pixelSize: 11
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    // Weather and NOTAM are dedicated tools. Until connected, they explicitly
    // report missing data rather than implying a successful operational check.
    Rectangle {
        id: operationalCard
        visible: root.panelVisible("Flight Conditions")
        x: 16
        y: root.draggingPanel === "Flight Conditions" ? root.dragVisualY : root.panelBaseY("Flight Conditions")
        width: parent.width - 32
        height: root.selectedOperationalTool === "" ? 94 : Math.min(parent.height * 0.25, Math.max(94, operationalDetailColumn.implicitHeight + 50))
        radius: 8
        color: "transparent"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
        z: root.draggingPanel === "Flight Conditions" ? 200 : 1

        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 32
            radius: 7; color: "#0B1B2B"
            Rectangle { x: 0; y: height / 2; width: parent.width; height: parent.height / 2; color: parent.color }
            Text {
                anchors.left: parent.left; anchors.leftMargin: 12
                anchors.right: parent.right; anchors.rightMargin: 32
                anchors.verticalCenter: parent.verticalCenter
                text: "FLIGHT CONDITIONS"
                color: root.text; font.family: "B612"; font.pixelSize: 12; font.bold: true
            }
        MouseArea {
            // The panel header is the drag handle. No visual grip is shown.
            anchors.fill: parent
            preventStealing: true
            onPressed: root.beginPanelDrag("Flight Conditions", mapToItem(root, mouse.x, mouse.y).y)
            onPositionChanged: root.updatePanelDrag("Flight Conditions", mapToItem(root, mouse.x, mouse.y).y)
            onReleased: {
                root.updatePanelDrag("Flight Conditions", mapToItem(root, mouse.x, mouse.y).y)
                root.finishPanelDrag("Flight Conditions")
            }
            onCanceled: root.finishPanelDrag("Flight Conditions")
        }
        }

        Column {
            x: 8; y: 38; width: parent.width - 16; spacing: 3
            visible: root.selectedOperationalTool === ""

            Row {
                visible: panelSettingsPopup.enabledTools.indexOf("Weather") >= 0
                         || root.weatherPilotAttentionRequired
                width: parent.width; height: 23
                spacing: 8
                Text { id: weatherStatusLabel; width: parent.width * 0.42; text: "WEATHER"; color: root.text; font.family: "B612"; font.pixelSize: 11; verticalAlignment: Text.AlignVCenter }
                Text { width: parent.width - weatherStatusLabel.width - parent.spacing; text: "NO DATA"; color: root.amber; font.family: "B612"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignRight; verticalAlignment: Text.AlignVCenter }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.selectedOperationalTool = "WEATHER" }
            }
            Row {
                visible: panelSettingsPopup.enabledTools.indexOf("NOTAM") >= 0
                         || root.notamPilotAttentionRequired
                width: parent.width; height: 23
                spacing: 8
                Text { id: notamStatusLabel; width: parent.width * 0.42; text: "NOTAM"; color: root.text; font.family: "B612"; font.pixelSize: 11; verticalAlignment: Text.AlignVCenter }
                Text { width: parent.width - notamStatusLabel.width - parent.spacing; text: "NOT CHECKED"; color: root.amber; font.family: "B612"; font.pixelSize: 10; font.bold: true; horizontalAlignment: Text.AlignRight; verticalAlignment: Text.AlignVCenter }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: root.selectedOperationalTool = "NOTAM" }
            }
        }

        Flickable {
            id: operationalDetails
            x: 10; y: 39; width: parent.width - 20; height: Math.max(0, parent.height - 45)
            contentWidth: width; contentHeight: operationalDetailColumn.implicitHeight
            clip: true; boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            visible: (root.selectedOperationalTool === "WEATHER"
                       && (panelSettingsPopup.enabledTools.indexOf("Weather") >= 0 || root.weatherPilotAttentionRequired))
                     || (root.selectedOperationalTool === "NOTAM"
                         && (panelSettingsPopup.enabledTools.indexOf("NOTAM") >= 0 || root.notamPilotAttentionRequired))
            Column {
                id: operationalDetailColumn
                width: operationalDetails.width
                spacing: 4
                Text {
                    width: parent.width
                    text: root.selectedOperationalTool === "WEATHER"
                          ? "WEATHER · Данные источника не подключены. Проверка условий не выполнена."
                          : "NOTAM / AIRSPACE · Источник ограничений не подключён. Маршрут не проверен."
                    color: root.text; font.family: "B612"; font.pixelSize: 11; wrapMode: Text.WordWrap
                }
                Text {
                    text: "‹ НАЗАД"
                    color: root.cyan; font.family: "B612"; font.pixelSize: 10
                    MouseArea { anchors.fill: parent; anchors.margins: -4; cursorShape: Qt.PointingHandCursor; onClicked: root.selectedOperationalTool = "" }
                }
            }
        }
    }

    function openSystemMessage(message) {
        selectedInformationMessage = message
        interventionMode = false
    }

    // INFORMATION: acknowledgement hides an item from this overview only.
    // Source events remain in the system journal/audit trail.
    function visibleSystemMessages() {
        return systemMessages.filter(function(message) {
            var sourceRequiresAttention =
                    message.sourceTool === "WEATHER" ? root.weatherPilotAttentionRequired :
                    message.sourceTool === "NOTAM" ? root.notamPilotAttentionRequired : true
            return acknowledgedMessageIds.indexOf(message.id) < 0 &&
                   sourceRequiresAttention &&
                   (message.requiresIntervention || message.severity === "critical" || message.severity === "warning")
        })
    }

    function visibleChecklistItems() {
        return checklistItems.filter(function(item) { return item.status !== "PASS" })
    }

    function hasUnacknowledgedCriticalMessage() {
        return systemMessages.some(function(message) {
            return message.severity === "critical" &&
                   acknowledgedMessageIds.indexOf(message.id) < 0
        })
    }

    function acknowledgeInformationMessage() {
        if (!selectedInformationMessage)
            return
        var next = acknowledgedMessageIds.slice()
        if (next.indexOf(selectedInformationMessage.id) < 0)
            next.push(selectedInformationMessage.id)
        acknowledgedMessageIds = next
        selectedInformationMessage = null
        interventionMode = false
    }

    // The message card grows with its content and remains bounded by the right panel.
    // ATC is an independently reorderable panel and is no longer a fixed bottom anchor.
    readonly property real informationAvailableHeight:
        Math.max(72, parent.height - informationCard.y - 20)

    Rectangle {
        id: informationCard
        visible: root.panelVisible("Alerting")
        x: 16
        y: root.draggingPanel === "Alerting" ? root.dragVisualY : root.panelBaseY("Alerting")
        width: parent.width - 32
        height: Math.max(72, Math.min(root.informationAvailableHeight, (root.selectedInformationMessage ? informationDetailsColumn.implicitHeight : informationList.implicitHeight) + 48))
        radius: 8
        color: "transparent"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
        z: root.draggingPanel === "Alerting" ? 200 : 1
    }

    Rectangle {
        visible: informationCard.visible
        x: informationCard.x + 1
        y: informationCard.y + 1
        width: informationCard.width - 2
        height: 32
        radius: 7
        color: "#0B1B2B"
        antialiasing: true

        Rectangle {
            x: 0
            y: height / 2
            width: parent.width
            height: parent.height / 2
            color: parent.color
        }

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 32
            anchors.verticalCenter: parent.verticalCenter
            text: "ALERTING"
            color: root.text
            font.family: "B612"
            font.pixelSize: 13
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        MouseArea {
            // The panel header is the drag handle. No visual grip is shown.
            anchors.fill: parent
            preventStealing: true
            onPressed: root.beginPanelDrag("Alerting", mapToItem(root, mouse.x, mouse.y).y)
            onPositionChanged: root.updatePanelDrag("Alerting", mapToItem(root, mouse.x, mouse.y).y)
            onReleased: {
                root.updatePanelDrag("Alerting", mapToItem(root, mouse.x, mouse.y).y)
                root.finishPanelDrag("Alerting")
            }
            onCanceled: root.finishPanelDrag("Alerting")
        }
    }

    // Empty body is intentional: no unacknowledged messages means no alert text.
    Flickable {
        id: informationOverview
        visible: informationCard.visible && !root.selectedInformationMessage
        x: informationCard.x + 10
        y: informationCard.y + 39
        width: informationCard.width - 20
        height: Math.max(0, informationCard.height - 48)
        contentWidth: width
        contentHeight: informationList.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: informationList
            width: informationOverview.width
            spacing: 2

        Repeater {
            model: root.visibleSystemMessages()

            delegate: Item {
                width: informationList.width
                // Card height follows the wrapped message text, not a fixed row size.
                height: Math.max(39, messageBody.implicitHeight + 22)

                Rectangle {
                    anchors.fill: parent
                    radius: 4
                    color: messageMouse.containsMouse ? "#102337" : "#000000"
                    border.width: modelData.severity === "critical" ? 1 : 0
                    border.color: root.red
                }

                Text {
                    x: 4
                    y: 2
                    width: parent.width - 8
                    height: 15
                    text: (modelData.kind === "FAILURE" ? "✕  " :
                           modelData.kind === "WARNING" ? "⚠  " : "•  ") + modelData.kind
                    color: modelData.severity === "critical" ? root.red :
                           modelData.severity === "warning" ? root.amber : root.cyan
                    font.family: "B612"
                    font.pixelSize: 9
                    font.bold: true
                }

                Text {
                    id: messageBody
                    x: 4
                    y: 17
                    width: parent.width - 8
                    text: modelData.title
                    color: root.text
                    font.family: "B612"
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft
                }

                MouseArea {
                    id: messageMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.selectedInformationMessage = modelData
                        root.interventionMode = false
                    }
                }
            }
            }
        }
    }

    Flickable {
        id: informationDetails
        visible: informationCard.visible && !!root.selectedInformationMessage
        x: informationCard.x + 12
        y: informationCard.y + 40
        width: informationCard.width - 24
        height: Math.max(0, informationCard.height - 48)
        contentWidth: width
        contentHeight: informationDetailsColumn.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: informationDetailsColumn
            width: informationDetails.width
            spacing: 5

        Text {
            width: parent.width
            text: root.selectedInformationMessage ? root.selectedInformationMessage.title : ""
            color: root.text
            font.family: "B612"
            font.pixelSize: 13
            font.bold: true
            wrapMode: Text.Wrap
        }

        Text {
            width: parent.width
            text: root.interventionMode && root.selectedInformationMessage
                  ? "ТРЕБУЕТСЯ ДЕЙСТВИЕ ПИЛОТА"
                  : (root.selectedInformationMessage ? root.selectedInformationMessage.detail : "")
            color: root.text
            font.family: "B612"
            font.pixelSize: 13
            wrapMode: Text.Wrap
        }

        Text {
            visible: !!root.selectedInformationMessage && root.selectedInformationMessage.requiresIntervention
            width: parent.width
            text: root.interventionMode && root.selectedInformationMessage
                  ? root.selectedInformationMessage.action
                  : "Нажмите, чтобы открыть область действий пилота."
            color: root.cyan
            font.family: "B612"
            font.pixelSize: 13
            wrapMode: Text.Wrap
        }

        Row {
            width: parent.width
            spacing: 6

            Rectangle {
                visible: !!root.selectedInformationMessage &&
                         root.selectedInformationMessage.requiresIntervention &&
                         !root.interventionMode
                width: (parent.width - parent.spacing) * 0.58
                height: 26
                radius: 3
                color: "#0B1B2B"
                border.color: root.cyan

                Text {
                    anchors.fill: parent
                    text: "К ДЕЙСТВИЮ"
                    color: root.cyan
                    font.family: "B612"
                    font.pixelSize: 8
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        root.interventionMode = true
                        root.pilotInterventionRequested(root.selectedInformationMessage.id)
                    }
                }
            }

            Rectangle {
                width: root.selectedInformationMessage &&
                       root.selectedInformationMessage.requiresIntervention &&
                       !root.interventionMode
                       ? (parent.width - parent.spacing) * 0.42 : parent.width
                height: 26
                radius: 3
                color: "transparent"
                border.color: root.divider

                Text {
                    anchors.fill: parent
                    text: root.selectedInformationMessage &&
                          root.selectedInformationMessage.requiresIntervention &&
                          !root.interventionMode ? "НАЗАД" : "ПОДТВЕРДИТЬ"
                    color: root.secondary
                    font.family: "B612"
                    font.pixelSize: 8
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (root.selectedInformationMessage &&
                            root.selectedInformationMessage.requiresIntervention &&
                            !root.interventionMode) {
                            root.selectedInformationMessage = null
                        } else {
                            root.acknowledgeInformationMessage()
                        }
                    }
                }
            }
            }
        }
    }

    // Unified ATC work area. The rounded frame encloses the heading and
    // all action controls, with a clear inset around every button.
    Rectangle {
        id: atcWorkArea
        visible: root.atcHeaderVisible || root.atcVisibleButtonCount > 0
        x: 16
        y: root.draggingPanel === "ATC" ? root.dragVisualY : root.panelBaseY("ATC")
        width: parent.width - 32
        height: root.atcWorkAreaHeight
        radius: 8
        color: "transparent"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
        z: root.draggingPanel === "ATC" ? 200 : 1
    }

    // Header is a filled band, not a separate bordered card.
    Rectangle {
        visible: root.atcHeaderVisible
        x: atcWorkArea.x + 1
        y: atcWorkArea.y + 1
        width: atcWorkArea.width - 2
        height: 32
        radius: 7
        color: "#0B1B2B"
        antialiasing: true

        // Square the lower corners of the header band so it joins the work area.
        Rectangle {
            x: 0
            y: height / 2
            width: parent.width
            height: parent.height / 2
            color: parent.color
        }

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.rightMargin: 32
            anchors.verticalCenter: parent.verticalCenter
            text: "ATC"
            color: root.text
            font.family: "B612"
            font.pixelSize: 13
            font.bold: true
            verticalAlignment: Text.AlignVCenter
        }
        MouseArea {
            // The panel header is the drag handle. No visual grip is shown.
            anchors.fill: parent
            preventStealing: true
            onPressed: root.beginPanelDrag("ATC", mapToItem(root, mouse.x, mouse.y).y)
            onPositionChanged: root.updatePanelDrag("ATC", mapToItem(root, mouse.x, mouse.y).y)
            onReleased: {
                root.updatePanelDrag("ATC", mapToItem(root, mouse.x, mouse.y).y)
                root.finishPanelDrag("ATC")
            }
            onCanceled: root.finishPanelDrag("ATC")
        }
    }

    Rectangle {
        visible: root.atcValidationVisible
        x: 26
        y: root.atcValidationY
        width: parent.width - 52
        height: root.atcButtonHeight
        color: "transparent"
        border.color: Qt.rgba(root.green.r, root.green.g, root.green.b, root.validationPulse)
        opacity: root.manualCreationMode && !root.manualCompositionComplete ? 0.55 : 1.0
        border.width: 1
    }

    Text {
        visible: root.atcValidationVisible
        x: 26
        y: root.atcValidationY
        width: parent.width - 52
        height: root.atcButtonHeight
        text: root.manualCreationMode ? "ВАЛИДАЦИЯ МИССИИ" : "VALIDATE MISSION"
        opacity: root.manualCreationMode && !root.manualCompositionComplete ? 0.65 : 1.0
        color: root.green
        font.family: "B612"
        font.pixelSize: 12
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
        visible: root.manualCreationMode && !root.manualValidationStarted
                 && panelSettingsPopup.enabledTools.indexOf("Validation") >= 0
        x: 26
        y: root.atcValidationY
        width: parent.width - 52
        height: root.atcButtonHeight
        enabled: root.manualCompositionComplete
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.validateManualMissionRequested()
    }

    SequentialAnimation on validationPulse {
        running: root.manualCreationMode ? !root.manualValidationStarted : root.validationConfirmationRequired
        loops: Animation.Infinite
        NumberAnimation { from: 0.35; to: 1.0; duration: 650; easing.type: Easing.InOutSine }
        NumberAnimation { from: 1.0; to: 0.35; duration: 650; easing.type: Easing.InOutSine }
    }

    Rectangle {
        visible: root.atcSendVisible
        x: 26
        y: root.atcSendY
        width: parent.width - 52
        height: root.atcButtonHeight
        color: "transparent"
        border.color: root.divider
        border.width: 1
    }

    Text {
        visible: root.atcSendVisible
        x: 26
        y: root.atcSendY
        width: parent.width - 52
        height: root.atcButtonHeight
        text: "SEND FLIGHT PLAN"
        color: root.secondary
        font.family: "B612"
        font.pixelSize: 12
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Rectangle {
        visible: root.atcStartVisible
        x: 26
        y: root.atcStartY
        width: parent.width - 52
        height: root.atcButtonHeight
        color: root.missionReady ? "transparent" : "#050505"
        border.color: root.missionReady ? root.green : root.divider
        border.width: 1
    }

    Text {
        visible: root.atcStartVisible
        x: 26
        y: root.atcStartY
        width: parent.width - 52
        height: root.atcButtonHeight
        text: "START MISSION"
        color: root.missionReady ? root.green : root.muted
        font.family: "B612"
        font.pixelSize: 12
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
        visible: root.atcStartVisible
        x: 26
        y: root.atcStartY
        width: parent.width - 52
        height: root.atcButtonHeight
        enabled: root.missionReady
        onClicked: root.startMissionRequested()
    }

    PanelSettingsButton {
        id: panelSettings
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        z: 400
        onClicked: panelSettingsPopup.open = !panelSettingsPopup.open
    }

    PanelSettingsPopup {
        id: panelSettingsPopup
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 44
        width: Math.min(parent.width - 16, Math.max(260, panelSettingsPopup.contentWidth))
        height: Math.min(parent.height - 52, panelSettingsPopup.contentHeight + 30)
        title: "PANEL CONTROL"
        tools: ["Checklist", "Weather", "NOTAM", "Information", "Readiness", "Validation", "Send Flight Plan", "Start Mission", "Map Alerts"]
        toolGroups: [
            { key: "monitoring", title: "MONITORING", expandedByDefault: true, tools: ["Checklist", "Weather", "NOTAM", "Information"] },
            { key: "mission", title: "MISSION CONTROL", expandedByDefault: true, tools: ["Readiness", "Validation", "Send Flight Plan", "Start Mission"] },
            { key: "map", title: "MAP", expandedByDefault: false, tools: ["Map Alerts"] }
        ]
        onClosed: open = false
    }
}
