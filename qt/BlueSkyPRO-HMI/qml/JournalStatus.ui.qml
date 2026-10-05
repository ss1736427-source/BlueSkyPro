import QtQuick

Item {
    id: root
    property color bg: "#0A0A0A"
    property color text: "#FFFFFF"
    property color secondary: "#BFBFBF"
    property color cyan: "#32FFFF"
    property color divider: "#202020"
    property string eventText: ""
    property string eventType: "JOURNAL"

    implicitHeight: 32

    Rectangle {
        anchors.fill: parent
        color: root.bg
        border.color: root.divider
        border.width: 1
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        text: eventText.length > 0 ? eventType + " · " + eventText : eventType + " · READY"
        color: eventText.length > 0 ? root.cyan : root.secondary
        font.family: "B612 Mono"
        font.pixelSize: 10
    }
}
