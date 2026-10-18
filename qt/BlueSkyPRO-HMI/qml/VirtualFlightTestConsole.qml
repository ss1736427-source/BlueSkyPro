import QtQuick
import QtQuick.Controls

Item {
    id: root
    property int scenarioIndex: 0
    property bool runRequested: false
    readonly property color panel: "#0B1724"
    readonly property color line: "#26394B"
    readonly property color text: "#E8EEF5"
    readonly property color muted: "#8EA2B8"
    readonly property color cyan: "#00B9F0"
    readonly property color amber: "#FFD166"

    readonly property var scenarios: [
        { name: "HISTORICAL AVERAGE / CALM", wind: "3.0 m/s", gust: "5.0 m/s", temp: "15 °C", visibility: "10 km", cloud: "FEW / 1 500 m", notam: "2 synthetic test records", profile: "Illustrative averaged profile — not tied to a real station or date" },
        { name: "HISTORICAL AVERAGE / WINDY", wind: "8.0 m/s", gust: "12.0 m/s", temp: "9 °C", visibility: "8 km", cloud: "BKN / 900 m", notam: "4 synthetic test records", profile: "Illustrative averaged profile — not tied to a real station or date" },
        { name: "HISTORICAL AVERAGE / LOW VIS", wind: "4.0 m/s", gust: "7.0 m/s", temp: "6 °C", visibility: "3 km", cloud: "OVC / 450 m", notam: "3 synthetic test records", profile: "Illustrative averaged profile — not tied to a real station or date" }
    ]
    readonly property var scenario: scenarios[scenarioIndex]

    Rectangle { anchors.fill: parent; color: "transparent" }

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 14

        Column {
            spacing: 5
            Text { text: "VIRTUAL FLIGHT / TEST ENVIRONMENT"; color: root.text; font.family: "B612"; font.pixelSize: 22; font.bold: true }
            Text { text: "SIMULATION INPUTS / MOCK CONNECTORS / REPLAY"; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 10 }
        }

        Rectangle {
            width: parent.width; height: 64; radius: 3
            color: "#2B2115"; border.color: "#8B682F"
            Column {
                anchors.fill: parent; anchors.margins: 10; spacing: 4
                Text { text: "SYNTHETIC TEST DATA — NOT FOR FLIGHT OPERATIONS"; color: root.amber; font.family: "B612 Mono"; font.pixelSize: 11; font.bold: true }
                Text { text: "Weather values are illustrative averages for exercising UI and planning interfaces; NOTAM records and zones are stubs, not current aeronautical information."; color: root.text; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap; width: parent.width }
            }
        }

        Row {
            width: parent.width; height: 42; spacing: 10
            Repeater {
                model: root.scenarios
                delegate: Rectangle {
                    width: (root.width - 48 - 20) / 3
                    height: 42
                    radius: 3
                    color: root.scenarioIndex === index ? "#10283A" : root.panel
                    border.color: root.scenarioIndex === index ? root.cyan : root.line
                    Text { anchors.centerIn: parent; text: modelData.name; color: root.scenarioIndex === index ? root.cyan : root.text; font.family: "B612 Mono"; font.pixelSize: 9; font.bold: true }
                    MouseArea { anchors.fill: parent; onClicked: { root.scenarioIndex = index; root.runRequested = false } }
                }
            }
        }

        Grid {
            width: parent.width
            columns: 3
            spacing: 10
            Repeater {
                model: [
                    { label: "WIND / MEAN", value: root.scenario.wind },
                    { label: "WIND / GUST", value: root.scenario.gust },
                    { label: "AIR TEMPERATURE", value: root.scenario.temp },
                    { label: "VISIBILITY", value: root.scenario.visibility },
                    { label: "CLOUD BASE", value: root.scenario.cloud },
                    { label: "NOTAM STUBS", value: root.scenario.notam }
                ]
                delegate: Rectangle {
                    width: (root.width - 48 - 20) / 3
                    height: 76
                    radius: 3
                    color: root.panel
                    border.color: root.line
                    Column {
                        anchors.fill: parent; anchors.margins: 12; spacing: 8
                        Text { text: modelData.label; color: root.muted; font.family: "B612 Mono"; font.pixelSize: 9 }
                        Text { text: modelData.value; color: root.text; font.family: "B612"; font.pixelSize: 16; font.bold: true }
                    }
                }
            }
        }

        Rectangle {
            width: parent.width; height: 62; radius: 3
            color: root.panel; border.color: root.line
            Column {
                anchors.fill: parent; anchors.margins: 10; spacing: 5
                Text { text: "DATA PROVENANCE"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 10; font.bold: true }
                Text { width: parent.width; text: root.scenario.profile; color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap }
            }
        }

        Row {
            spacing: 12
            Rectangle {
                width: 190; height: 42; radius: 3
                color: "#10283A"; border.color: root.cyan
                Text { anchors.centerIn: parent; text: root.runRequested ? "TEST PROFILE SELECTED" : "SELECT TEST PROFILE"; color: root.cyan; font.family: "B612 Mono"; font.pixelSize: 10; font.bold: true }
                MouseArea { anchors.fill: parent; onClicked: root.runRequested = true }
            }
            Text {
                width: root.width - 260
                text: root.runRequested
                      ? "Profile selected in the HMI fixture. No planning kernel was executed and no route or safety result was produced."
                      : "Select a scenario to exercise the Virtual Flight test interface. This control does not launch or authorize a flight."
                color: root.muted; font.family: "B612"; font.pixelSize: 10; wrapMode: Text.Wrap; verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
