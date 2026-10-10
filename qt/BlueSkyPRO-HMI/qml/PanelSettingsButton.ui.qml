import QtQuick

Item {
    id: root
    implicitWidth: 28
    implicitHeight: 28

    signal clicked()

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: "transparent"
        border.width: 0
        radius: 3
    }

    // Draw the system-tools glyph as geometry so it renders identically
    // regardless of the installed font or Qt Design Studio glyph fallback.
    Column {
        anchors.centerIn: parent
        spacing: 3
        Repeater {
            model: 3
            delegate: Rectangle {
                width: 12
                height: 1.5
                color: "#32FFFF"
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
