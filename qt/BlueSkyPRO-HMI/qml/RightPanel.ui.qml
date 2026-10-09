import QtQuick
import QtCore

Item {
    id: root

    // Prevent child controls from painting outside the panel when its
    // width is collapsed to zero by MainContent.
    clip: true
    implicitWidth: 270

    // MainContent overlays BottomToolbar on top of the full workspace.
    // Keep reorderable panels inside the visible right-panel work area.
    property int bottomInset: 54

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
        property string positionCsv: "Checklist=54,Flight Conditions=293,Alerting=400,ATC=499"
        property int freePositionLayoutVersion: 0
        property bool positionsLocked: false
    }

    property var panelOrder: panelOrderSettings.orderCsv.split(",")
    property bool panelsLocked: panelOrderSettings.positionsLocked

    function setPanelsLocked(locked) {
        panelOrderSettings.positionsLocked = locked
        panelOrderSettings.sync()
        panelsLocked = locked
        if (!locked)
            Qt.callLater(root.reflowPanelPositions)
    }
    property string draggingPanel: ""
    property real dragVisualY: 0
    property real dragGrabOffsetY: 0
    property var panelPositions: ({})
    property int lastAtcPanelHeight: -1

    // When the ATC content grows or shrinks while positions are locked,
    // preserve its bottom edge so the card returns to its prior bottom-aligned
    // location after a temporary action button disappears.
    function handleAtcPanelHeightChanged(newHeight) {
        var previousHeight = lastAtcPanelHeight
        lastAtcPanelHeight = newHeight
        if (previousHeight < 0 || !panelsLocked || draggingPanel !== "")
            return

        var positions = Object.assign({}, panelPositions)
        var currentY = positions["ATC"] !== undefined
                ? positions["ATC"] : panelPositionY("ATC")
        var bottomLimit = Math.max(54, root.height - bottomInset - 8)
        var maxY = Math.max(54, bottomLimit - newHeight)
        positions["ATC"] = Math.max(54, Math.min(maxY, currentY + previousHeight - newHeight))
        panelPositions = positions
        savePanelPositions()
        Qt.callLater(root.reflowPanelPositions)
    }

    function loadPanelPositions() {
        var result = {}
        var entries = panelOrderSettings.positionCsv.split(",")
        for (var i = 0; i < entries.length; ++i) {
            var parts = entries[i].split("=")
            if (parts.length === 2 && parts[0] !== "")
                result[parts[0]] = Number(parts[1])
        }

        // Backfill missing positions from the current visual order.
        var y = 54
        var required = ["Checklist", "Flight Conditions", "Alerting", "ATC"]
        for (var j = 0; j < required.length; ++j) {
            var key = required[j]
            if (result[key] === undefined) {
                result[key] = y
                y += panelHeight(key) + 10
            }
        }
        // One-time migration: place ATC at the true lower edge instead of
        // the old packed-stack position. Future operator positions are preserved.
        if (panelOrderSettings.freePositionLayoutVersion < 2) {
            if (panelVisible("ATC"))
                result["ATC"] = Math.max(54, root.height - bottomInset - 8 - panelHeight("ATC"))
            panelOrderSettings.freePositionLayoutVersion = 2
        }

        panelPositions = result
        savePanelPositions()
    }

    function savePanelPositions() {
        var required = ["Checklist", "Flight Conditions", "Alerting", "ATC"]
        var entries = []
        for (var i = 0; i < required.length; ++i) {
            var key = required[i]
            var value = panelPositions[key]
            if (value !== undefined)
                entries.push(key + "=" + Math.round(value))
        }
        panelOrderSettings.positionCsv = entries.join(",")
        panelOrderSettings.sync()
    }

    function panelPositionY(key) {
        var value = panelPositions[key]
        if (value === undefined)
            return panelBaseY(key)
        return value
    }

    function beginPanelDrag(key, pressRootY) {
        if (panelsLocked || !panelVisible(key))
            return

        draggingPanel = key
        var storedY = panelPositionY(key)
        dragGrabOffsetY = pressRootY - storedY
        dragVisualY = storedY
    }

    function updatePanelDrag(key, currentRootY) {
        if (draggingPanel !== key)
            return

        var maxY = Math.max(54, root.height - bottomInset - 8 - panelHeight(key))
        dragVisualY = Math.max(54, Math.min(maxY, currentRootY - dragGrabOffsetY))
    }

    function saveNormalizedPositions(items) {
        var next = {}
        var keys = ["Checklist", "Flight Conditions", "Alerting", "ATC"]

        for (var i = 0; i < keys.length; ++i)
            next[keys[i]] = panelPositions[keys[i]]

        for (var j = 0; j < items.length; ++j)
            next[items[j].key] = Math.round(items[j].y)

        panelPositions = next
        savePanelPositions()
    }

    // Keep all visible panels separated by a fixed gap while preserving
    // their current vertical order. The available area ends above the
    // overlaid BottomToolbar.
    // Preserve operator-selected positions. Only clamp to the visible work area
    // and repair genuine collisions; never repack the whole stack.
    function reflowPanelPositions() {
        if (draggingPanel !== "")
            return

        var keys = ["Checklist", "Flight Conditions", "Alerting", "ATC"]
        var top = 54
        var bottomLimit = Math.max(top, root.height - bottomInset - 8)
        var items = []

        for (var i = 0; i < keys.length; ++i) {
            var key = keys[i]
            if (panelVisible(key))
                items.push({ key: key, y: panelPositionY(key), h: panelHeight(key) })
        }

        // Locking preserves the operator's arrangement, but a panel whose
        // content grows must still remain above the fixed bottom toolbar.
        // Clamp only out-of-bounds cards; do not reorder or resolve collisions
        // automatically while the layout is locked.
        if (panelsLocked) {
            for (var lockedIndex = 0; lockedIndex < items.length; ++lockedIndex) {
                var lockedMaxY = Math.max(top, bottomLimit - items[lockedIndex].h)
                items[lockedIndex].y = Math.max(top, Math.min(lockedMaxY, items[lockedIndex].y))
            }
            saveNormalizedPositions(items)
            return
        }

        // Clamp saved positions first.
        for (var j = 0; j < items.length; ++j)
            items[j].y = Math.max(top, Math.min(bottomLimit - items[j].h, items[j].y))

        // Repair only real overlaps, keeping the existing order and free gaps.
        items.sort(function(a, b) { return a.y - b.y })
        var gap = 10
        for (var k = 1; k < items.length; ++k) {
            var requiredY = items[k - 1].y + items[k - 1].h + gap
            if (items[k].y < requiredY)
                items[k].y = requiredY
        }

        // If the repaired stack exceeds the bottom, move only the affected
        // lower cards upward as far as possible; do not repack the entire stack.
        if (items.length > 0) {
            var lastBottom = items[items.length - 1].y + items[items.length - 1].h
            if (lastBottom > bottomLimit) {
                for (var m = items.length - 1; m >= 0 && lastBottom > bottomLimit; --m) {
                    var maxY = bottomLimit - items[m].h
                    if (items[m].y > maxY)
                        items[m].y = maxY
                    if (m > 0) {
                        var maxPrev = items[m].y - gap - items[m - 1].h
                        if (items[m - 1].y > maxPrev)
                            items[m - 1].y = Math.max(top, maxPrev)
                    }
                    lastBottom = items[items.length - 1].y + items[items.length - 1].h
                }
            }
        }

        saveNormalizedPositions(items)
    }

    // Drop at the actual mouse position. The dragged card keeps that position
    // whenever it is free; if it intersects another card, move only the
    // dragged card to the nearest free gap.
    function dropPanelAtPosition(key, currentRootY) {
        if (draggingPanel !== key)
            return

        updatePanelDrag(key, currentRootY)

        var top = 54
        var bottomLimit = Math.max(top, root.height - bottomInset - 8)
        var h = panelHeight(key)
        var desired = Math.max(top, Math.min(bottomLimit - h, dragVisualY))
        var gap = 10
        var keys = ["Checklist", "Flight Conditions", "Alerting", "ATC"]
        var obstacles = []

        for (var i = 0; i < keys.length; ++i) {
            var other = keys[i]
            if (other !== key && panelVisible(other))
                obstacles.push({ key: other, y: panelPositionY(other), h: panelHeight(other) })
        }

        function overlaps(y, obstacle) {
            return y < obstacle.y + obstacle.h + gap &&
                   y + h + gap > obstacle.y
        }

        var candidates = [desired, top, bottomLimit - h]
        for (var j = 0; j < obstacles.length; ++j) {
            candidates.push(obstacles[j].y - gap - h)
            candidates.push(obstacles[j].y + obstacles[j].h + gap)
        }

        var best = desired
        var bestDistance = Number.POSITIVE_INFINITY
        for (var k = 0; k < candidates.length; ++k) {
            var candidate = Math.max(top, Math.min(bottomLimit - h, candidates[k]))
            var free = true
            for (var m = 0; m < obstacles.length; ++m) {
                if (overlaps(candidate, obstacles[m])) {
                    free = false
                    break
                }
            }
            if (free) {
                var distance = Math.abs(candidate - desired)
                if (distance < bestDistance) {
                    best = candidate
                    bestDistance = distance
                }
            }
        }

        panelPositions[key] = Math.round(best)
        savePanelPositions()
        draggingPanel = ""
        dragVisualY = 0
        dragGrabOffsetY = 0
    }

    function cancelPanelDrag(key) {
        if (draggingPanel !== key)
            return
        draggingPanel = ""
        dragVisualY = 0
        dragGrabOffsetY = 0
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
        loadPanelPositions()
        Qt.callLater(root.reflowPanelPositions)
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
        y: root.draggingPanel === "Checklist" ? root.dragVisualY : root.panelPositionY("Checklist")
        width: parent.width - 32
        height: Math.min(42 + root.visibleChecklistItems().length * 18, Math.max(90, parent.height * 0.34))
        radius: 8
        color: "transparent"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
        z: root.draggingPanel === "Checklist" ? 200 : 1
        onHeightChanged: root.handleAtcPanelHeightChanged(height)
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
                root.dropPanelAtPosition("Checklist", mapToItem(root, mouse.x, mouse.y).y)
            }
            onCanceled: root.cancelPanelDrag("Checklist")
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
        y: root.draggingPanel === "Flight Conditions" ? root.dragVisualY : root.panelPositionY("Flight Conditions")
        width: parent.width - 32
        height: root.selectedOperationalTool === "" ? 94 : Math.min(parent.height * 0.25, Math.max(94, operationalDetailColumn.implicitHeight + 50))
        radius: 8
        color: "transparent"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
        z: root.draggingPanel === "Flight Conditions" ? 200 : 1
        onHeightChanged: Qt.callLater(root.reflowPanelPositions)

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
                root.dropPanelAtPosition("Flight Conditions", mapToItem(root, mouse.x, mouse.y).y)
            }
            onCanceled: root.cancelPanelDrag("Flight Conditions")
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
        y: root.draggingPanel === "Alerting" ? root.dragVisualY : root.panelPositionY("Alerting")
        width: parent.width - 32
        height: Math.max(72, Math.min(root.informationAvailableHeight, (root.selectedInformationMessage ? informationDetailsColumn.implicitHeight : informationList.implicitHeight) + 48))
        radius: 8
        color: "transparent"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
        z: root.draggingPanel === "Alerting" ? 200 : 1
        onHeightChanged: Qt.callLater(root.reflowPanelPositions)
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
                root.dropPanelAtPosition("Alerting", mapToItem(root, mouse.x, mouse.y).y)
            }
            onCanceled: root.cancelPanelDrag("Alerting")
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
        y: root.draggingPanel === "ATC" ? root.dragVisualY : root.panelPositionY("ATC")
        width: parent.width - 32
        height: root.atcWorkAreaHeight
        radius: 8
        color: "transparent"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
        z: root.draggingPanel === "ATC" ? 200 : 1
        onHeightChanged: Qt.callLater(root.reflowPanelPositions)
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
                root.dropPanelAtPosition("ATC", mapToItem(root, mouse.x, mouse.y).y)
            }
            onCanceled: root.cancelPanelDrag("ATC")
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
        positionsLocked: root.panelsLocked
        onPositionsLockToggled: root.setPanelsLocked(locked)
        tools: ["Checklist", "Weather", "NOTAM", "Information", "Readiness", "Validation", "Send Flight Plan", "Start Mission", "Map Alerts"]
        toolGroups: [
            { key: "monitoring", title: "MONITORING", expandedByDefault: true, tools: ["Checklist", "Weather", "NOTAM", "Information"] },
            { key: "mission", title: "MISSION CONTROL", expandedByDefault: true, tools: ["Readiness", "Validation", "Send Flight Plan", "Start Mission"] },
            { key: "map", title: "MAP", expandedByDefault: false, tools: ["Map Alerts"] }
        ]
        onClosed: open = false
    }
}
