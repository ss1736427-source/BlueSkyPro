import QtQuick

Item {
    id: root

    property color bg: "#0A0A0A"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color amber: "#FFD339"
    property color cyan: "#32FFFF"
    property int uavIndex: -1
    property string uavId: "NO UAV SELECTED"

    // Parent sizes the overlay to the selected card and available workspace.
    property real contentPadding: 12
    property real preferredWidth: 360
    property real minWidth: 220
    property real maxWidth: 420
    readonly property real contentWidth: Math.max(0, width - contentPadding * 2)

    implicitWidth: preferredWidth
    implicitHeight: contentColumn.implicitHeight + contentPadding * 2
    visible: uavIndex >= 0

    signal decisionRequested(string decision, int uavIndex)
    signal contextClosed()

    Rectangle {
        anchors.fill: parent
        color: root.bg
        border.color: root.amber
        border.width: 1
    }

    Column {
        id: contentColumn
        x: root.contentPadding
        y: root.contentPadding
        width: root.contentWidth
        spacing: 7

        Text {
            width: parent.width
            text: "UAV CONTEXT"
            color: root.secondary
            font.family: "B612"
            font.pixelSize: 12
            font.bold: true
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.uavId + " · Preliminary assessment"
            color: root.text
            font.family: "B612"
            font.pixelSize: 13
            wrapMode: Text.Wrap
        }

        Text {
            width: parent.width
            text: "Telemetry · deviation · resource · weather"
            color: root.secondary
            font.family: "B612"
            font.pixelSize: 10
            wrapMode: Text.Wrap
        }

        Flow {
            width: parent.width
            spacing: 6

            Rectangle {
                width: Math.min(78, contentColumn.width)
                height: 28
                color: "transparent"
                border.color: root.amber
                border.width: 1

                Text {
                    anchors.fill: parent
                    text: "RETURN"
                    color: root.amber
                    font.family: "B612"
                    font.pixelSize: 11
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.decisionRequested("RETURN", root.uavIndex)
                }
            }

            Rectangle {
                width: Math.max(0, Math.min(190, contentColumn.width - 84))
                height: 28
                color: "transparent"
                border.color: root.cyan
                border.width: 1

                Text {
                    anchors.fill: parent
                    anchors.margins: 3
                    text: "ПРОДОЛЖИТЬ ПОЛЁТ"
                    color: root.cyan
                    font.family: "B612"
                    font.pixelSize: 11
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                    minimumPixelSize: 8
                    fontSizeMode: Text.Fit
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.decisionRequested("CONTINUE", root.uavIndex)
                }
            }

            Text {
                width: 24
                height: 28
                text: "×"
                color: root.secondary
                font.family: "B612"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.contextClosed()
                }
            }
        }
    }
}
