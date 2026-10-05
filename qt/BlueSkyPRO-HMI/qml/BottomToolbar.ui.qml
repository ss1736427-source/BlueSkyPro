import QtQuick

Item {
    id: root

    implicitHeight: 54

    property color bg: "#050A12"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color cyan: "#32FFFF"
    property color divider: "#111F30"
    property bool leftOpen: true
    property bool rightOpen: true
    property string activeTool: "MAP"

    signal leftPanelToggleRequested()
    signal rightPanelToggleRequested()
    signal toolActivated(string tool)
    signal toolConfigurationRequested()

    ListModel {
        id: toolModel
        ListElement { tool: "MAP" }
        ListElement { tool: "UAV" }
        ListElement { tool: "FPV" }
        ListElement { tool: "ADMIN" }
        ListElement { tool: "VIRTUAL FLT" }
    }

    function activateTool(tool) {
        // UAV is a toggle: second activation returns the workspace to MAP.
        activeTool = (tool === "UAV" && activeTool === "UAV") ? "MAP" : tool
        toolActivated(activeTool)
    }

    function moveTool(fromIndex, toIndex) {
        if (fromIndex === toIndex || toIndex < 0 || toIndex >= toolModel.count)
            return
        toolModel.move(fromIndex, toIndex, 1)
    }

    function enabledToolNames() {
        var names = []
        for (var i = 0; i < toolModel.count; ++i)
            names.push(toolModel.get(i).tool)
        return names
    }

    function setToolEnabled(tool, enabled) {
        var current = []
        for (var i = 0; i < toolModel.count; ++i)
            current.push(toolModel.get(i).tool)

        var exists = current.indexOf(tool) >= 0
        if (enabled && !exists) {
            var insertIndex = current.length
            toolModel.append({ tool: tool })
            return
        }

        if (!enabled && exists) {
            toolModel.remove(current.indexOf(tool), 1)
        }
    }

    Rectangle {
        anchors.fill: parent
        color: root.bg
    }

    // Single shared seam between the workspace and the bottom toolbar.
    Rectangle {
        x: 0
        y: 0
        width: parent.width
        height: 1
        color: root.cyan
        antialiasing: false
    }

    // Fixed left anchor.
    Rectangle {
        id: leftButton
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 10
        width: 88
        height: 38
        color: "#0C1725"
        border.color: root.divider
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: "LEFT ◀"
            color: root.leftOpen ? root.cyan : root.secondary
            font.family: "B612 Mono"
            font.pixelSize: 10
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.leftPanelToggleRequested()
        }
    }

    // Fixed right anchor.
    Rectangle {
        id: rightButton
        anchors.right: timeButton.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        width: 88
        height: 38
        color: "#0C1725"
        border.color: root.divider
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: "RIGHT ▶"
            color: root.rightOpen ? root.cyan : root.secondary
            font.family: "B612 Mono"
            font.pixelSize: 10
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.rightPanelToggleRequested()
        }
    }

    // Reorderable working tools.
    Row {
        id: toolRow
        anchors.left: leftButton.right
        anchors.right: rightButton.left
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        height: 38
        spacing: 6

        Repeater {
            model: toolModel

            delegate: Rectangle {
                property bool dragging: false
                property int sourceIndex: index
                property int dragStartIndex: index

                width: Math.max(96, Math.min(132, (toolRow.width - 24) / Math.max(1, toolModel.count)))
                height: 38
                color: root.activeTool === model.tool ? "#111F30" : "#0A0A0A"
                border.color: root.activeTool === model.tool ? root.cyan : root.divider
                border.width: 1
                z: dragging ? 20 : 1

                Text {
                    anchors.centerIn: parent
                    text: model.tool
                    color: root.activeTool === model.tool ? root.cyan : root.secondary
                    font.family: "B612 Mono"
                    font.pixelSize: 10
                    horizontalAlignment: Text.AlignHCenter
                }

                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis: Drag.XAxis
                    onPressed: {
                        parent.dragging = true
                        parent.dragStartIndex = index
                        parent.z = 20
                    }
                    onReleased: {
                        parent.dragging = false
                        parent.z = 1

                        var center = parent.x + parent.width / 2
                        var target = parent.dragStartIndex
                        for (var i = 0; i < toolRow.children.length; ++i) {
                            var candidate = toolRow.children[i]
                            if (candidate === parent || candidate.width === undefined)
                                continue
                            if (center > candidate.x + candidate.width / 2)
                                target = i
                        }
                        target = Math.max(0, Math.min(toolModel.count - 1, target))
                        root.moveTool(parent.dragStartIndex, target)
                    }
                    onClicked: root.activateTool(model.tool)
                }
            }
        }
    }

    Rectangle {
        id: timeButton
        anchors.right: settingsButton.left
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        width: 72
        height: 38
        color: "#050A12"
        border.color: root.divider
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: Qt.formatTime(new Date(), "HH:mm")
            color: root.secondary
            font.family: "B612 Mono"
            font.pixelSize: 11
        }
    }

    Rectangle {
        id: settingsButton
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 10
        width: 44
        height: 38
        color: "#0C1725"
        border.color: root.cyan
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: "☰"
            color: root.cyan
            font.pixelSize: 16
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.toolConfigurationRequested()
        }
    }
}
