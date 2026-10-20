import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    visible: true
    width: Math.min(Screen.width * 0.96, 1920)
    height: Math.min(Screen.height * 0.90, 1080)
    minimumWidth: 1280
    minimumHeight: 720
    title: "BlueSky PRO"
    color: "#050A12"

    MainContent {
        anchors.fill: parent
    }
}
