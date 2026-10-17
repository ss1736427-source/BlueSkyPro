import QtQuick

Item {
    id: root

    property var altitudeTelemetry: null
    property color textColor: "#FFFFFF"
    property color mutedColor: "#7F7F7F"
    property color normalColor: "#64FF00"
    property color degradedColor: "#FFD339"
    property int labelSize: 10
    property int valueSize: 17

    readonly property bool available: altitudeTelemetry !== null && altitudeTelemetry.available
    readonly property bool degraded: altitudeTelemetry !== null &&
                                      (altitudeTelemetry.degraded ||
                                       altitudeTelemetry.timestampQualityDegraded)
    readonly property string sourceLabel: altitudeTelemetry !== null
                                              ? altitudeTelemetry.sourceLabel
                                              : "UNAVAILABLE"
    readonly property string freshnessLabel: altitudeTelemetry !== null
                                                 ? altitudeTelemetry.freshness
                                                 : "MISSING"

    implicitWidth: 188
    implicitHeight: 44

    Column {
        anchors.fill: parent
        spacing: 2

        Text {
            width: parent.width
            text: root.sourceLabel
            color: root.available
                    ? (root.degraded ? root.degradedColor : root.normalColor)
                    : root.mutedColor
            font.pixelSize: root.labelSize
            font.bold: true
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            width: parent.width
            text: root.available ? Number(altitudeTelemetry.valueM).toFixed(1) + " m"
                                 : "—"
            color: root.available
                    ? (root.degraded ? root.degradedColor : root.textColor)
                    : root.mutedColor
            font.pixelSize: root.valueSize
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            width: parent.width
            visible: !root.available || root.degraded
            text: root.freshnessLabel
            color: root.degraded ? root.degradedColor : root.mutedColor
            font.pixelSize: root.labelSize - 1
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }
}
