import QtQuick

Item {
    id: root
    property string title: ""
    property string value: ""
    property color valueColor: "#BFBFBF"
    property color headingColor: "#BFBFBF"
    property int headingSize: 11
    property int valueSize: 18
    property int headingValueGap: 3

    Text {
        id: heading
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -(root.headingValueGap + root.valueSize) / 2
        text: root.title
        color: root.headingColor
        font.family: "B612"
        font.pixelSize: root.headingSize
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: heading.bottom
        anchors.topMargin: root.headingValueGap
        text: root.value
        color: root.valueColor
        // Use the platform's system UI font for the value row.
        // Scale with the header height while respecting the configured value size.
        font.pixelSize: Math.max(14, Math.min(root.valueSize, parent.height * 0.34))
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
    }

}