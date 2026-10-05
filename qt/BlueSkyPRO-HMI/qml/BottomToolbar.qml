import QtQuick
import QtCore

Item {
    id: root

    implicitHeight: 54

    property color bg: "#050A12"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color cyan: "#32FFFF"
    property color divider: "#7F7F7F"
    property bool leftOpen: true
    property bool rightOpen: true

    // Bottom-toolbar configuration is presentation state only.
    // It never changes mission data, flight logic, or safety state.
    property string activeTool: "MAP"
    readonly property var toolDefinitions: [
        { key: "UAV", label: "UAV" },
        { key: "MAP", label: "MAP" },
        { key: "ADMIN", label: "ADMIN" },
        { key: "FPV", label: "FPV" },
        { key: "VIRTUAL FLT", label: "VIRTUAL FLT" }
    ]

    signal leftPanelToggleRequested()
    signal rightPanelToggleRequested()
    signal toolActivated(string tool)
    signal toolConfigurationChanged(string orderJson, string enabledJson)
signal workspaceContextRequested(string tool)

    Settings {
        id: settings
        category: "BlueSkyPRO/BottomToolbar"
        property string toolOrderJson: ""
        property string enabledToolsJson: ""
        property string activeTool: "MAP"
    }

    ListModel {
        id: toolModel
    }

    // Visible toolbar projection. Hidden tools stay in toolModel so their
    // relative order is preserved, while the toolbar itself has no gaps.
    ListModel {
        id: visibleToolModel
    }

    property string dragToolKey: ""
    property int dragSourceIndex: -1
    property bool dragActive: false

    function defaultOrder() {
        return ["MAP", "ADMIN", "VIRTUAL FLT", "FPV", "UAV"]
    }

    function defaultEnabled() {
        return {
            "UAV": true,
            "MAP": true,
            "ADMIN": true,
            "FPV": true,
            "VIRTUAL FLT": true
        }
    }

    function contains(list, value) {
        for (var i = 0; i < list.length; ++i)
            if (list[i] === value)
                return true
        return false
    }

    function normalizeOrder(raw) {
        var defaults = defaultOrder()
        var result = []

        if (Array.isArray(raw)) {
            for (var i = 0; i < raw.length; ++i) {
                if (contains(defaults, raw[i]) && !contains(result, raw[i]))
                    result.push(raw[i])
            }
        }

        for (var j = 0; j < defaults.length; ++j) {
            if (!contains(result, defaults[j]))
                result.push(defaults[j])
        }

        // Migrate the previous default/current toolbar order so VIRTUAL FLT
        // occupies the middle slot in the five-tool layout.
        var previousDefault = ["UAV", "MAP", "ADMIN", "FPV", "VIRTUAL FLT"]
        var previousDisplayed = ["MAP", "ADMIN", "FPV", "VIRTUAL FLT", "UAV"]
        var matchesPreviousDefault = result.length === previousDefault.length
        var matchesPreviousDisplayed = result.length === previousDisplayed.length
        for (var k = 0; k < result.length; ++k) {
            if (result[k] !== previousDefault[k])
                matchesPreviousDefault = false
            if (result[k] !== previousDisplayed[k])
                matchesPreviousDisplayed = false
        }
        if (matchesPreviousDefault || matchesPreviousDisplayed)
            result = defaults.slice(0)

        return result
    }

    function normalizeEnabled(raw) {
        var defaults = defaultEnabled()
        var result = {}
        var enabledCount = 0

        for (var i = 0; i < toolDefinitions.length; ++i) {
            var key = toolDefinitions[i].key
            result[key] = raw && raw[key] === false ? false : defaults[key]
            if (result[key])
                ++enabledCount
        }

        // At least one work context must remain available.
        if (enabledCount === 0)
            result["MAP"] = true

        return result
    }

    function loadConfiguration() {
        var order
        var enabled

        try {
            order = settings.toolOrderJson ? JSON.parse(settings.toolOrderJson) : defaultOrder()
        } catch (e) {
            order = defaultOrder()
        }

        try {
            enabled = settings.enabledToolsJson ? JSON.parse(settings.enabledToolsJson) : defaultEnabled()
        } catch (e2) {
            enabled = defaultEnabled()
        }

        order = normalizeOrder(order)
        enabled = normalizeEnabled(enabled)

        toolModel.clear()
        for (var i = 0; i < order.length; ++i) {
            var key = order[i]
            toolModel.append({
                key: key,
                label: key,
                enabled: enabled[key]
            })
        }

        rebuildVisibleToolModel()

        var requestedActive = settings.activeTool
        if (!contains(order, requestedActive) || !enabled[requestedActive])
            requestedActive = firstEnabled(order, enabled)

        root.activeTool = requestedActive
        saveConfiguration()
    }

    function firstEnabled(order, enabled) {
        for (var i = 0; i < order.length; ++i) {
            if (enabled[order[i]])
                return order[i]
        }
        return "MAP"
    }

    function rebuildVisibleToolModel() {
        visibleToolModel.clear()
        for (var i = 0; i < toolModel.count; ++i) {
            var item = toolModel.get(i)
            if (item.enabled)
                visibleToolModel.append({
                    key: item.key,
                    label: item.label
                })
        }
    }

    function findVisibleIndex(key) {
        for (var i = 0; i < visibleToolModel.count; ++i)
            if (visibleToolModel.get(i).key === key)
                return i
        return -1
    }

    function syncVisibleOrder() {
        var desired = []
        for (var i = 0; i < toolModel.count; ++i) {
            if (toolModel.get(i).enabled)
                desired.push(toolModel.get(i).key)
        }

        for (var j = 0; j < desired.length; ++j) {
            var current = findVisibleIndex(desired[j])
            if (current >= 0 && current !== j)
                visibleToolModel.move(current, j, 1)
        }
    }

    // STRICT INVARIANT:
    // enabled=true means the tool MUST exist on the bottom toolbar.
    // Reordering is never allowed to remove an enabled tool.
    function ensureEnabledToolsVisible() {
        for (var i = 0; i < toolModel.count; ++i) {
            var item = toolModel.get(i)
            if (item.enabled && findVisibleIndex(item.key) < 0)
                visibleToolModel.append({ key: item.key, label: item.label })
        }
        syncVisibleOrder()
    }

    function currentOrder() {
        var result = []
        for (var i = 0; i < toolModel.count; ++i)
            result.push(toolModel.get(i).key)
        return result
    }

    function currentEnabled() {
        var result = {}
        for (var i = 0; i < toolModel.count; ++i)
            result[toolModel.get(i).key] = toolModel.get(i).enabled
        return result
    }

    function saveConfiguration() {
        var order = currentOrder()
        var enabled = currentEnabled()

        settings.toolOrderJson = JSON.stringify(order)
        settings.enabledToolsJson = JSON.stringify(enabled)
        settings.activeTool = root.activeTool
        root.toolConfigurationChanged(settings.toolOrderJson, settings.enabledToolsJson)
    }

    function activateTool(key) {
        for (var i = 0; i < toolModel.count; ++i) {
            var item = toolModel.get(i)
            if (item.key === key && item.enabled) {
                // UAV is a true panel toggle: second press closes it to MAP.
                var nextTool = key === "UAV" && root.activeTool === "UAV" ? "MAP" : key
                root.activeTool = nextTool
                settings.activeTool = nextTool
                root.toolActivated(nextTool)
                root.workspaceContextRequested(nextTool)
                return
            }
        }
    }

    function setToolEnabled(key, enabled) {
        var enabledCount = 0
        for (var i = 0; i < toolModel.count; ++i)
            if (toolModel.get(i).enabled)
                ++enabledCount

        if (!enabled && enabledCount <= 1)
            return

        for (var j = 0; j < toolModel.count; ++j) {
            if (toolModel.get(j).key === key) {
                toolModel.setProperty(j, "enabled", enabled)
                break
            }
        }

        if (enabled) {
            var visibleIndex = findVisibleIndex(key)
            if (visibleIndex < 0) {
                visibleToolModel.append({ key: key, label: key })
                syncVisibleOrder()
            }
        } else {
            var hiddenIndex = findVisibleIndex(key)
            if (hiddenIndex >= 0)
                visibleToolModel.remove(hiddenIndex, 1)
        }

        if (!enabled && root.activeTool === key)
            root.activeTool = firstEnabled(currentOrder(), currentEnabled())

        saveConfiguration()
    }

    function findToolIndex(key) {
        for (var i = 0; i < toolModel.count; ++i)
            if (toolModel.get(i).key === key)
                return i
        return -1
    }

    function moveTool(from, to) {
        if (from < 0 || to < 0 || from >= toolModel.count || to >= toolModel.count || from === to)
            return

        toolModel.move(from, to, 1)
        ensureEnabledToolsVisible()
        saveConfiguration()
    }

    function moveVisibleTool(key, targetVisibleIndex) {
        var fromVisible = findVisibleIndex(key)
        if (fromVisible < 0 || targetVisibleIndex < 0 || targetVisibleIndex >= visibleToolModel.count || fromVisible === targetVisibleIndex)
            return

        var targetKey = visibleToolModel.get(targetVisibleIndex).key
        var fromFull = findToolIndex(key)
        var targetFull = findToolIndex(targetKey)
        if (fromFull < 0 || targetFull < 0)
            return

        // Keep the full model and visible projection in lockstep without
        // clearing/recreating the visible delegates during a drop.
        toolModel.move(fromFull, targetFull, 1)
        visibleToolModel.move(fromVisible, targetVisibleIndex, 1)

        // STRICT: a drag/reorder can never hide an enabled tool.
        ensureEnabledToolsVisible()
        saveConfiguration()
    }

    Component.onCompleted: loadConfiguration()

    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    Rectangle {
        x: 0
        y: 0
        width: parent.width
        height: 1
        color: root.cyan
    }

    Rectangle {
        x: 0
        y: parent.height - 1
        width: parent.width
        height: 1
        color: root.cyan
    }

    Rectangle {
        x: 0
        y: 0
        width: 1
        height: parent.height
        color: root.cyan
    }

    Rectangle {
        x: parent.width - 1
        y: 0
        width: 1
        height: parent.height
        color: root.cyan
    }

    Timer {
        interval: 30000
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: clockText.text = Qt.formatTime(new Date(), "hh:mm")
    }

    Row {
        id: toolbarRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 6

        Rectangle {
            width: 76
            height: 38
            color: "#0C1725"
            border.color: root.divider
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "LEFT"
                color: root.leftOpen ? root.cyan : root.secondary
                font.family: "B612 Mono"
                font.pixelSize: 10
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.leftPanelToggleRequested()
            }
        }

        Item {
            id: toolArea
            width: Math.max(1, toolbarRow.width - 76 - 76 - 58 - 1 - 30 - toolsButton.width)
            height: 40

            Item {
                id: visibleToolRow
                anchors.fill: parent

                // For an odd number of tools, anchor the middle button itself
                // to the exact horizontal center, then pack the other buttons
                // outward on both sides.
                function widthAt(i) {
                    if (i < 0 || i >= visibleToolModel.count)
                        return 0
                    // Uniform button width guarantees stable, equal gaps
                    // while delegates are dragged and after the order changes.
                    return 100
                }

                function xForTool(i) {
                    var count = visibleToolModel.count
                    if (count === 0)
                        return 0
                    var middle = Math.floor(count / 2)
                    // Center against the outer panel width, not the space
                    // between LEFT/RIGHT anchors. Convert panel center to
                    // this tool area's local coordinate system.
                    var centerX = root.width / 2 - toolArea.x
                    if (count % 2 === 1) {
                        var middleWidth = widthAt(middle)
                        if (i === middle)
                            return centerX - middleWidth / 2
                        if (i < middle) {
                            var leftOffset = middleWidth / 2 + 6
                            for (var left = middle - 1; left >= i; --left)
                                leftOffset += widthAt(left) + (left === i ? 0 : 6)
                            return centerX - leftOffset
                        }
                        var rightOffset = middleWidth / 2 + 6
                        for (var right = middle + 1; right < i; ++right)
                            rightOffset += widthAt(right) + 6
                        return centerX + rightOffset
                    }
                    var total = 0
                    for (var j = 0; j < count; ++j)
                        total += widthAt(j)
                    total += Math.max(0, count - 1) * 6
                    var start = centerX - total / 2
                    for (var k = 0; k < i; ++k)
                        start += widthAt(k) + 6
                    return start
                }

                Repeater {
                    id: toolRepeater
                    model: visibleToolModel

                    delegate: Rectangle {
                        id: toolDelegate
                        width: 100
                        x: visibleToolRow.xForTool(index)
                        y: (visibleToolRow.height - height) / 2
                        height: 38
                        color: root.activeTool === model.key ? "#111F30" : "#0C1725"
                        border.color: root.activeTool === model.key ? root.cyan : root.divider
                        border.width: 1

                        property bool dragging: false
                        property real pressX: 0
                        property real dragOffsetX: 0
                        property int dragTargetIndex: index

                        transform: Translate {
                            x: toolDelegate.dragOffsetX
                        }

                        z: dragging ? 100 : 0

                        Text {
                            id: toolLabel
                            anchors.centerIn: parent
                            text: model.label
                            color: root.activeTool === model.key ? root.cyan : root.secondary
                            font.family: "B612 Mono"
                            font.pixelSize: 10
                        }

                        MouseArea {
                            anchors.fill: parent
                            preventStealing: true
                            hoverEnabled: true

                            property string pressedToolKey: ""

                            onPressed: {
                                var p = mapToItem(toolArea, mouse.x, mouse.y)
                                pressedToolKey = model.key
                                toolDelegate.pressX = p.x
                                toolDelegate.dragOffsetX = 0
                                toolDelegate.dragTargetIndex = index
                                toolDelegate.dragging = false
                            }

                            onPositionChanged: {
                                if (!pressed)
                                    return

                                var p = mapToItem(toolArea, mouse.x, mouse.y)
                                var delta = p.x - toolDelegate.pressX

                                if (!toolDelegate.dragging && Math.abs(delta) > 8)
                                    toolDelegate.dragging = true

                                if (!toolDelegate.dragging)
                                    return

                                // The button follows the pointer through a transform.
                                // Row remains the sole owner of the delegate's x position.
                                toolDelegate.dragOffsetX = delta

                                // Calculate insertion position from the pointer,
                                // allowing movement across any number of buttons.
                                var centerX = p.x
                                var target = visibleToolModel.count - 1

                                for (var i = 0; i < visibleToolModel.count; ++i) {
                                    if (i === index)
                                        continue

                                    var item = toolRepeater.itemAt(i)
                                    if (!item)
                                        continue

                                    var itemCenter = item.x + item.width / 2
                                    if (centerX < itemCenter) {
                                        target = i
                                        break
                                    }
                                }

                                toolDelegate.dragTargetIndex = target
                            }

                            onReleased: {
                                var wasDragged = toolDelegate.dragging
                                toolDelegate.dragging = false

                                // Return control of positioning to the Row.
                                toolDelegate.dragOffsetX = 0

                                if (!wasDragged) {
                                    root.activateTool(pressedToolKey)
                                } else {
                                    root.moveVisibleTool(pressedToolKey, toolDelegate.dragTargetIndex)
                                }
                                pressedToolKey = ""
                            }

                            onCanceled: {
                                toolDelegate.dragging = false
                                toolDelegate.dragOffsetX = 0
                                pressedToolKey = ""
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            width: 76
            height: 38
            color: "#0C1725"
            border.color: root.divider
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "RIGHT"
                color: root.rightOpen ? root.cyan : root.secondary
                font.family: "B612 Mono"
                font.pixelSize: 10
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.rightPanelToggleRequested()
            }
        }

        Rectangle {
            width: 1
            height: 38
            color: root.cyan
        }

        Rectangle {
            width: 58
            height: 38
            color: "transparent"

            Text {
                anchors.centerIn: parent
                id: clockText
                text: Qt.formatTime(new Date(), "hh:mm")
                color: root.secondary
                font.family: "B612 Mono"
                font.pixelSize: 14
            }
        }

        Rectangle {
            id: toolsButton
            width: 36
            height: 38
            color: "transparent"

            Text {
                anchors.centerIn: parent
                text: "☰"
                color: toolsPopup.visible ? root.text : root.cyan
                font.family: "B612 Mono"
                font.pixelSize: 14
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    // Opening configuration must never expose a stale/incomplete
                    // visible projection. ENABLED tools are restored first.
                    root.ensureEnabledToolsVisible()
                    toolsPopup.visible = !toolsPopup.visible
                }
            }
        }
    }

    Rectangle {
        id: toolsPopup
        visible: false
        z: 20

        onVisibleChanged: {
            if (visible)
                root.ensureEnabledToolsVisible()
        }
        // Fit the panel to the longest tool label and the number of rows.
        // Keep a small minimum for touch targets, but avoid a fixed oversized box.
        width: Math.min(root.width - 16, Math.max(220, configLabelMetrics.width + 112))
        height: 70 + toolModel.count * 45
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.height + 6
        color: "#08111D"
        border.color: root.divider
        border.width: 1

        TextMetrics {
            id: configLabelMetrics
            font.family: "B612 Mono"
            font.pixelSize: 10
            text: "VIRTUAL FLT"
        }

        Item {
            id: configHeader
            x: 0
            y: 0
            width: parent.width
            height: 54

            Text {
                x: 16
                y: 14
                text: "TOOLS CONFIGURATION"
                color: root.text
                font.family: "B612 Mono"
                font.pixelSize: 11
            }

            Text {
                x: 16
                y: 34
                text: "ENABLE / DISABLE | ORDER"
                color: root.secondary
                font.family: "B612 Mono"
                font.pixelSize: 8
            }

            MouseArea {
                anchors.fill: parent
                onClicked: toolsPopup.visible = false
            }
        }

        ListView {
            id: configList
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 58
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 5
            model: toolModel
            clip: true

            delegate: Rectangle {
                id: configRow
                width: configList.width
                height: 40
                color: "#0C1725"
                border.color: root.divider
                border.width: 1


                // Compact touch-friendly switch. Green = enabled, dark = disabled.
                Rectangle {
                    id: enabledSwitch
                    x: 12
                    anchors.verticalCenter: parent.verticalCenter
                    width: 38
                    height: 20
                    radius: 10
                    color: model.enabled ? "#64FF00" : "#0C1725"
                    border.color: model.enabled ? "#64FF00" : "#7F7F7F"
                    border.width: 1

                    Rectangle {
                        width: 16
                        height: 16
                        radius: 8
                        anchors.verticalCenter: parent.verticalCenter
                        x: model.enabled ? parent.width - width - 2 : 2
                        color: model.enabled ? "#050A12" : "#BFBFBF"
                    }
                }

                Text {
                    x: 62
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.label
                    color: root.text
                    font.family: "B612 Mono"
                    font.pixelSize: 10
                }

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: "⋮"
                    color: root.secondary
                    font.pixelSize: 14
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (mouse.x < 58)
                            root.setToolEnabled(model.key, !model.enabled)
                    }

                    onPositionChanged: {
                        if (!pressed)
                            return

                        var point = mapToItem(configList, mouse.x, mouse.y)
                        var target = configList.indexAt(point.x, point.y)
                        var current = root.findToolIndex(model.key)
                        if (target < 0 || current < 0 || target === current)
                            return

                        root.moveTool(current, target)
                    }
                }
            }
        }
    }
}
