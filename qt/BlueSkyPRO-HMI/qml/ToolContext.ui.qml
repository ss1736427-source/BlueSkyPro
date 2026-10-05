import QtQuick

Item {
    id: root

    property string contextName: "UAV"
    property string contextSubtitle: ""
    property var sections: []
    property int selectedUavIndex: -1
    property string selectedUavId: "NO UAV SELECTED"

    Rectangle {
        anchors.fill: parent
        color: "transparent"
    }

    Text {
        x: 24
        y: 22
        text: root.contextName === "UAV" ? "UAV  /  " + root.selectedUavId : root.contextName
        color: "#FFFFFF"
        font.family: "B612"
        font.pixelSize: 20
        font.bold: true
    }

    Text {
        x: 24
        y: 52
        text: root.contextSubtitle
        color: "#7F7F7F"
        font.family: "B612 Mono"
        font.pixelSize: 10
    }

    Rectangle {
        x: 24
        y: 82
        width: parent.width - 48
        height: 1
        color: "#202020"
    }

    Rectangle {
        visible: root.contextName === "FPV"
        x: 24
        y: 104
        width: parent.width - 48
        height: 250
        color: "#000000"
        border.color: "#202020"
        border.width: 1

        Text {
            x: 18
            y: 14
            text: root.selectedUavId + "   CONTROL STATE: UNAVAILABLE"
            color: "#FFD339"
            font.family: "B612 Mono"
            font.pixelSize: 13
            font.bold: true
        }

        Rectangle {
            x: 18
            y: 44
            width: parent.width - 36
            height: 118
            color: "#08111D"
            border.color: "#202020"
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "CAMERA VIDEO"
                color: "#7F7F7F"
                font.family: "B612 Mono"
                font.pixelSize: 12
            }
        }

        Text { x: 18; y: 174; text: "TELEMETRY: NOT CONNECTED"; color: "#FFD339"; font.family: "B612 Mono"; font.pixelSize: 10 }
        Text { x: 18; y: 196; text: "C2: UNKNOWN     VIDEO: UNKNOWN     RC: UNKNOWN"; color: "#BFBFBF"; font.family: "B612 Mono"; font.pixelSize: 10 }

        Rectangle {
            x: parent.width - 190
            y: 180
            width: 172
            height: 42
            color: "transparent"
            border.color: "#FFD339"
            border.width: 1

            Text {
                anchors.fill: parent
                text: "CONTROL TRANSFER UNAVAILABLE"
                color: "#FFD339"
                font.family: "B612 Mono"
                font.pixelSize: 10
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

        }

        Text {
            x: 18
            y: 226
            text: "Prototype display only — no control link"
            color: "#7F7F7F"
            font.family: "B612"
            font.pixelSize: 10
        }
    }

    Flow {
        visible: root.contextName !== "FPV"
        x: 24
        y: 104
        width: parent.width - 48
        spacing: 10

        Repeater {
            model: root.sections

            delegate: Rectangle {
                width: 220
                height: 74
                color: "#08111D"
                border.color: "#202020"
                border.width: 1

                Text {
                    x: 12
                    y: 12
                    text: modelData
                    color: "#BFBFBF"
                    font.family: "B612"
                    font.pixelSize: 12
                    font.bold: true
                }

                Text {
                    x: 12
                    y: 38
                    text: "CONFIGURABLE"
                    color: "#7F7F7F"
                    font.family: "B612 Mono"
                    font.pixelSize: 9
                }
            }
        }
    }

    PanelSettingsButton {
        id: settings
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 14
        onClicked: popup.open = !popup.open
    }

    PanelSettingsPopup {
        id: popup
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 48
        width: 280
        height: Math.min(420, 80 + root.sections.length * 28)
        title: root.contextName + " SETTINGS"
        tools: root.sections
        onClosed: open = false
    }
}
