import QtQuick

Item {
    id: root

    property string title: "PANEL SETTINGS"
    property var tools: []
    property var toolGroups: []
    property bool open: false
    property var expandedGroups: []
    property int rowHeight: 30
    property int groupHeaderHeight: 30
    property var enabledTools: root.tools.slice()
    signal toolToggled(string tool, bool enabled)

    function syncEnabledTools(toolsList) {
        enabledTools = toolsList.slice()
    }

    function firstEnabled() {
        return enabledTools.length > 0 ? enabledTools[0] : ""
    }

    function groupExpanded(groupKey) {
        return expandedGroups.indexOf(groupKey) >= 0
    }

    function toggleGroup(groupKey) {
        var next = expandedGroups.slice()
        var i = next.indexOf(groupKey)
        if (i >= 0) next.splice(i, 1)
        else next.push(groupKey)
        expandedGroups = next
    }

    function visibleRowCount() {
        var count = 0
        for (var i = 0; i < toolGroups.length; ++i) {
            count += 1
            if (groupExpanded(toolGroups[i].key)) count += toolGroups[i].tools.length
        }
        return count
    }

    function syncGroups() {
        var next = []
        for (var i = 0; i < toolGroups.length; ++i)
            if (toolGroups[i].expandedByDefault) next.push(toolGroups[i].key)
        expandedGroups = next
    }

    function toggleTool(tool) {
        var next = enabledTools.slice()
        var i = next.indexOf(tool)
        var enabled = i < 0
        if (enabled)
            next.push(tool)
        else
            next.splice(i, 1)
        enabledTools = next
        toolToggled(tool, enabled)
    }

    signal closed()

    function longestToolLabel() {
        var longest = ""
        for (var i = 0; i < tools.length; ++i) {
            if (String(tools[i]).length > longest.length)
                longest = String(tools[i])
        }
        return longest
    }

    TextMetrics {
        id: toolLabelMetrics
        font.family: "B612"
        font.pixelSize: 13
        text: root.longestToolLabel()
    }

    readonly property real contentWidth: Math.max(toolLabelMetrics.width + 64, 260)
    readonly property bool directToolsMode: toolGroups.length === 0 && tools.length > 0
    readonly property real contentHeight: 64 + (directToolsMode ? tools.length * 34 : visibleRowCount() * 34)

    visible: root.open
    clip: true
    Component.onCompleted: root.syncGroups()
    z: 500

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: "#08111D"
        border.color: "#236078"
        border.width: 1
        antialiasing: true
    }

    Text {
        x: 14
        y: 12
        width: parent.width - 28
        elide: Text.ElideRight
        text: root.title
        color: "#FFFFFF"
        font.family: "B612"
        font.pixelSize: 13
        font.bold: true
    }

    Rectangle {
        x: 14
        y: 36
        width: parent.width - 28
        height: 1
        color: "#202020"
    }

    Flickable {
        id: settingsScroller
        x: 10
        y: 46
        width: parent.width - 20
        height: Math.max(0, parent.height - 82)
        contentWidth: width
        contentHeight: settingsColumn.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: settingsColumn
            width: settingsScroller.width
            spacing: 4

            // Header Settings uses the direct tool list when no groups are supplied.
            // This preserves the original ETD/TOT/TRIP/ETA/READY/WARNING settings workflow.
            Repeater {
                model: root.directToolsMode ? root.tools : []
                delegate: Item {
                    width: settingsColumn.width
                    height: root.rowHeight
                    property string toolKey: modelData
                    property bool toolEnabled: root.enabledTools.indexOf(toolKey) >= 0

                    Rectangle {
                        x: 0
                        anchors.verticalCenter: parent.verticalCenter
                        width: 32
                        height: 18
                        radius: 9
                        color: toolEnabled ? "#64FF00" : "#263748"
                        border.color: toolEnabled ? "#64FF00" : "#536273"
                        border.width: 1

                        Rectangle {
                            width: 12
                            height: 12
                            radius: 6
                            anchors.verticalCenter: parent.verticalCenter
                            x: toolEnabled ? parent.width - width - 2 : 2
                            color: toolEnabled ? "#08111D" : "#BFBFBF"
                        }
                    }

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 50
                        anchors.right: parent.right
                        anchors.rightMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        text: toolKey
                        color: "#BFBFBF"
                        font.family: "B612"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.toggleTool(toolKey)
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }

            Repeater {
                model: root.toolGroups
                delegate: Column {
                    width: settingsColumn.width
                    spacing: 2
                    property var groupData: modelData
                    Rectangle {
                        width: parent.width
                        height: root.groupHeaderHeight
                        radius: 4
                        color: "#0C1725"
                        border.color: "#263748"
                        border.width: 1
                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            text: (root.groupExpanded(groupData.key) ? "▾  " : "▸  ") + groupData.title
                            color: "#FFFFFF"
                            font.family: "B612"
                            font.pixelSize: 11
                            font.bold: true
                            elide: Text.ElideRight
                        }
                        MouseArea { anchors.fill: parent; onClicked: root.toggleGroup(groupData.key); cursorShape: Qt.PointingHandCursor }
                    }
                    Repeater {
                        model: root.groupExpanded(groupData.key) ? groupData.tools : []
                        delegate: Item {
                            width: parent.width
                            height: root.rowHeight
                            property string toolKey: modelData
                            property bool toolEnabled: root.enabledTools.indexOf(toolKey) >= 0
                            Rectangle {
                                x: 8; anchors.verticalCenter: parent.verticalCenter
                                width: 32; height: 18; radius: 9
                                color: toolEnabled ? "#64FF00" : "#263748"
                                border.color: toolEnabled ? "#64FF00" : "#536273"; border.width: 1
                                Rectangle { width: 12; height: 12; radius: 6; anchors.verticalCenter: parent.verticalCenter; x: toolEnabled ? parent.width - width - 2 : 2; color: toolEnabled ? "#08111D" : "#BFBFBF" }
                            }
                            Text { anchors.left: parent.left; anchors.leftMargin: 50; anchors.right: parent.right; anchors.rightMargin: 4; anchors.verticalCenter: parent.verticalCenter; text: toolKey; color: "#BFBFBF"; font.family: "B612"; font.pixelSize: 12; elide: Text.ElideRight }
                            MouseArea { anchors.fill: parent; onClicked: root.toggleTool(toolKey); cursorShape: Qt.PointingHandCursor }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        visible: settingsScroller.contentHeight > settingsScroller.height
        width: 2
        height: Math.max(18, settingsScroller.height * settingsScroller.height / settingsScroller.contentHeight)
        x: parent.width - 5
        y: settingsScroller.y + (settingsScroller.contentY / Math.max(1, settingsScroller.contentHeight - settingsScroller.height)) * (settingsScroller.height - height)
        radius: 1
        color: "#32FFFF"
    }
    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        text: "CLOSE"
        color: "#32FFFF"
        font.family: "B612 Mono"
        font.pixelSize: 10
        font.bold: true

        MouseArea {
            anchors.fill: parent
            anchors.margins: -8
            onClicked: root.closed()
        }
    }
}
